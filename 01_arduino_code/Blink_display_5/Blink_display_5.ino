#include <LiquidCrystal.h>

// ========== НАСТРОЙКИ ПИНОВ LCD ==========
constexpr uint8_t PIN_RS  = 2;
constexpr uint8_t PIN_EN  = A1;
constexpr uint8_t PIN_DB4 = A2;
constexpr uint8_t PIN_DB5 = A3;
constexpr uint8_t PIN_DB6 = A4;
constexpr uint8_t PIN_DB7 = A5;

// ========== ПИН ДАТЧИКА ==========
constexpr uint8_t ADC_PIN = A0;

// ========== ПАРАМЕТРЫ ACS712 (30A) ==========
const float V_REF      = 5.0;      // опорное напряжение AREF (5.000 В от REF195)
const float SENSITIVITY = 0.066;   // 66 мВ/А

// ========== ИНТЕРВАЛ ИЗМЕРЕНИЙ (мс) ==========
constexpr unsigned long INTERVAL_MS = 100;

// ========== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ==========
LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_DB4, PIN_DB5, PIN_DB6, PIN_DB7);
unsigned long n = 0;
unsigned long lastTime = 0;
int zeroADC = 512;                 // калибровочное значение АЦП при нулевом токе

// ========== SETUP ==========
void setup() {
  lcd.begin(16, 2);
  analogReference(EXTERNAL);

  // Калибровка нуля (убедитесь, что ток отсутствует)
  lcd.print("Calibrating...");
  delay(1000);
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(ADC_PIN);
    delay(2);
  }
  zeroADC = sum / 100;
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("n=");
  lcd.setCursor(0, 1);
  lcd.print("ADC=");
}

// ========== LOOP ==========
void loop() {
  unsigned long now = millis();

  if (now - lastTime >= INTERVAL_MS) {
    lastTime = now;
    n++;

    // Измерение
    int adcValue = analogRead(ADC_PIN);

    // Расчёт тока
    float voltage = (adcValue - zeroADC) * (V_REF / 1023.0);
    float current = voltage / SENSITIVITY;   // (Vout - V0) = I * sens

    // ----- Верхняя строка: номер цикла -----
    lcd.setCursor(2, 0);
    lcd.print("          ");       // очистка 10 символов
    lcd.setCursor(2, 0);
    lcd.print(n);

    // ----- Нижняя строка: ADC значение -----
    lcd.setCursor(4, 1);
    lcd.print("     ");            // очистка
    lcd.setCursor(4, 1);
    lcd.print(adcValue);

    // ----- Вывод тока справа (позиция 10, символы с 0) -----
    lcd.setCursor(11, 1);          // колонка 11 (0..15), вторая строка
    lcd.print("     ");            // очистка 5 символов
    lcd.setCursor(11, 1);
    lcd.print(current, 2);
    lcd.print("A");
  }
}