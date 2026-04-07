#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>

constexpr uint8_t pin8 = 8;                 // управление реле (HIGH = вкл)

// ========== НАСТРОЙКИ ПИНОВ LCD ==========
constexpr uint8_t PIN_RS = 2;
constexpr uint8_t PIN_EN = 3;
constexpr uint8_t PIN_DB4 = A2;
constexpr uint8_t PIN_DB5 = A3;
constexpr uint8_t PIN_DB6 = A4;
constexpr uint8_t PIN_DB7 = A5;

// ========== ПИН ДАТЧИКОВ ==========
constexpr uint8_t ADC_PIN  = A0;   // ACS712 (ток)
constexpr uint8_t ADC_PIN1 = A1;   // напряжение аккумулятора (прямое подключение + резистор 10к на GND)

// ========== ПИН SD (CS) ==========
constexpr uint8_t SD_CS = 7;        // PD7, физическая нога 13 ATmega328P

// ========== ПАРАМЕТРЫ ACS712 (30A) ==========
const float V_REF       = 5.0;      // опорное напряжение AREF (должно быть 5.000 В)
const float SENSITIVITY = 0.066;    // 66 мВ/А

// ========== ИНТЕРВАЛ ИЗМЕРЕНИЙ (мс) ==========
constexpr unsigned long INTERVAL_MS = 1000;   // 1 секунда

// ========== ПОРОГИ ВКЛЮЧЕНИЯ/ВЫКЛЮЧЕНИЯ РЕЛЕ ==========
const float VOLTAGE_ON  = 4.0;      // выше этого – включить нагрузку (один раз)
const float VOLTAGE_OFF = 3.0;      // ниже этого – выключить и больше не включать

// ========== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ==========
LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_DB4, PIN_DB5, PIN_DB6, PIN_DB7);
unsigned long n = 0;
unsigned long lastTime = 0;
int zeroADC = 512;
bool sdOk = false;

// ========== SETUP ==========
void setup() {
  lcd.begin(16, 2);
  analogReference(EXTERNAL);

  // Калибровка нуля ACS712
  lcd.print("Calibrating...");
  delay(1000);
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(ADC_PIN);
    delay(2);
  }
  zeroADC = sum / 100;
  lcd.clear();

  // ----- Инициализация SD -----
  lcd.setCursor(0, 0);
  lcd.print("SD init...");
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  if (!SD.begin(SD_CS)) {
    lcd.setCursor(0, 1);
    lcd.print("SD card error!");
    sdOk = false;
  } else {
    lcd.setCursor(0, 1);
    lcd.print("SD OK          ");
    sdOk = true;
    // Создаём заголовок CSV с добавленной колонкой RelayState
    File dataFile = SD.open("data.csv", FILE_WRITE);
    if (dataFile) {
      if (dataFile.size() == 0) {
        dataFile.println("n,Current_A,Voltage_V,ADC_current,ADC_voltage,RelayState");
      }
      dataFile.close();
    }
  }
  delay(1500);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("n=");
  lcd.setCursor(0, 1);
  lcd.print("ADC=");

  pinMode(pin8, OUTPUT);
  digitalWrite(pin8, LOW);
}

// ========== LOOP ==========
void loop() {
  unsigned long now = millis();

  if (now - lastTime >= INTERVAL_MS) {
    lastTime = now;
    n++;

    // Измерения
    int adcValue  = analogRead(ADC_PIN);
    int adcValue1 = analogRead(ADC_PIN1);

    float voltageDiff = (adcValue - zeroADC) * (V_REF / 1023.0);
    float current = voltageDiff / SENSITIVITY;
    float voltage = adcValue1 * (V_REF / 1023.0);

    // ----- Обновление LCD -----
    lcd.setCursor(2, 0);
    lcd.print("          ");
    lcd.setCursor(2, 0);
    lcd.print(n);

    lcd.setCursor(4, 1);
    lcd.print("     ");
    lcd.setCursor(4, 1);
    lcd.print(adcValue);

    lcd.setCursor(11, 1);
    lcd.print("     ");
    lcd.setCursor(11, 1);
    lcd.print(current, 2);
    lcd.print("A");

    lcd.setCursor(10, 0);
    lcd.print("     ");
    lcd.setCursor(10, 0);
    lcd.print(voltage, 4);

    // ----- Управление реле (однократное включение) -----
    static uint8_t nnn = 0;   // 0-ожидание, 1-включено, 2-выключено навсегда

    // Защита: если аккумулятор отключён (напряжение <0.5В), сбрасываем в 0
    if (voltage < 0.5) {
      nnn = 0;
      digitalWrite(pin8, LOW);
    } else {
      if (voltage > VOLTAGE_ON && nnn == 0)
        nnn = 1;
      if (voltage < VOLTAGE_OFF && nnn == 1)
        nnn = 2;
      digitalWrite(pin8, (nnn == 1) ? HIGH : LOW);
    }

    // ----- Запись на SD (всегда, если SD инициализирована) -----
    if (sdOk) {
      File dataFile = SD.open("data.csv", FILE_WRITE);
      if (dataFile) {
        dataFile.print(n);
        dataFile.print(",");
        dataFile.print(current, 3);
        dataFile.print(",");
        dataFile.print(voltage, 4);
        dataFile.print(",");
        dataFile.print(adcValue);
        dataFile.print(",");
        dataFile.print(adcValue1);
        dataFile.print(",");
        dataFile.println(nnn);   // состояние реле (0,1,2)
        dataFile.close();
      } else {
        lcd.setCursor(0, 1);
        lcd.print("SD write error ");
      }
    }
  }
}