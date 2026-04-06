#include <EEPROM.h>
#include <LiquidCrystal.h>

// ======================== НАСТРОЙКИ ПИНОВ ========================
constexpr uint8_t PIN_RS = 2;       // LCD RS
constexpr uint8_t PIN_EN = A1;      // LCD EN
constexpr uint8_t PIN_DB4 = A2;     // LCD DB4
constexpr uint8_t PIN_DB5 = A3;     // LCD DB5
constexpr uint8_t PIN_DB6 = A4;     // LCD DB6
constexpr uint8_t PIN_DB7 = A5;     // LCD DB7

constexpr uint8_t ADC_PIN = A0;     // пин для измерения напряжения
constexpr uint8_t PWM_PIN = 9;      // PWM вывод для формирования напряжения
constexpr uint8_t LED_PIN = 8;      // светодиод
constexpr uint8_t BUTTON_PIN = 0;   // кнопка (физическая нога 2 ATmega328P)

// ======================== ПАРАМЕТРЫ PWM ========================
const int PWM_MAX = 168;            // значение для ~3.3 В (168/255*5В)
const int PWM_STEP = 10;            // шаг изменения каждые 5 секунд
int pwmValue = 0;

// ======================== ПАРАМЕТРЫ ЗАПИСИ В EEPROM ========================
const int EEPROM_SIZE = 1024;
const int BYTES_PER_MEASURE = 2;
int eepromAddr = 0;
bool eepromFull = false;

// ======================== ФЛАГИ УПРАВЛЕНИЯ ========================
bool recordingEnabled = false;
bool wasLedOn = false;

// ======================== LCD ========================
LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_DB4, PIN_DB5, PIN_DB6, PIN_DB7);

unsigned long n = 0;

// ======================== SETUP ========================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);
  analogReference(EXTERNAL);

  lcd.setCursor(0, 0);
  lcd.print("n=");
  lcd.setCursor(0, 1);
  lcd.print("ADC=     V");

  // Временно показываем "REC OFF"
  lcd.setCursor(0, 1);
  lcd.print("REC OFF  V");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("ADC=     V");

  analogWrite(PWM_PIN, 0);
  delay(5000);                     // разряд конденсатора при старте
}

// ======================== LOOP ========================
void loop() {
  static unsigned long lastNTime = 0;
  static unsigned long lastDisplayTime = 0;
  static unsigned long pwmIntervalTimer = 0;
  unsigned long now = millis();

  // ---------- 1. ОБРАБОТКА КНОПКИ (долгое нажатие) ----------
  static unsigned long buttonPressStart = 0;
  static bool buttonLongPressedProcessed = false;
  static bool ledToggleState = false;

  bool currentButtonState = digitalRead(BUTTON_PIN);   // LOW при нажатии

  if (currentButtonState == LOW) {
    if (buttonPressStart == 0) buttonPressStart = now;
    if (!buttonLongPressedProcessed && (now - buttonPressStart) >= 200) {
      buttonLongPressedProcessed = true;
      ledToggleState = !ledToggleState;
      digitalWrite(LED_PIN, ledToggleState ? HIGH : LOW);
    }
  } else {
    buttonPressStart = 0;
    buttonLongPressedProcessed = false;
  }

  // ---------- 2. ФРОНТ ВКЛЮЧЕНИЯ СВЕТОДИОДА (СТАРТ ЗАПИСИ) ----------
  if (ledToggleState && !wasLedOn) {
    // Сброс PWM
    pwmValue = 0;
    analogWrite(PWM_PIN, pwmValue);
    
    // Сброс EEPROM
    eepromAddr = 0;
    eepromFull = false;
    recordingEnabled = true;
    
    // Синхронизация таймеров: момент старта
    pwmIntervalTimer = now;
    lastDisplayTime = now - 100;   // чтобы следующее измерение произошло сразу
    
    // НЕМЕДЛЕННОЕ ПЕРВОЕ ИЗМЕРЕНИЕ И ЗАПИСЬ (устраняет сдвиг)
    int adcValue = analogRead(ADC_PIN);
    uint16_t valueToStore = adcValue;
    EEPROM.put(eepromAddr, valueToStore);
    eepromAddr += BYTES_PER_MEASURE;
    
    // Обновление дисплея для первого измерения
    float voltage = adcValue * (5.0 / 1023.0);
    lcd.setCursor(2, 0); lcd.print("              "); lcd.setCursor(2, 0); lcd.print(n);
    lcd.setCursor(11, 0); lcd.print("   "); lcd.setCursor(11, 0); lcd.print(pwmValue);
    lcd.setCursor(4, 1); lcd.print("    "); lcd.setCursor(4, 1); lcd.print(adcValue);
    lcd.setCursor(11, 1); lcd.print("     "); lcd.setCursor(11, 1); lcd.print(voltage, 3);
    lcd.setCursor(14, 0); lcd.print("...");
    lcd.setCursor(12, 0); lcd.print("REC ");
  } 
  else if (!ledToggleState && wasLedOn) {
    // Выключение светодиода – остановка записи
    recordingEnabled = false;
    lcd.setCursor(12, 0);
    lcd.print("    ");
  }
  wasLedOn = ledToggleState;

  // ---------- 3. ИНКРЕМЕНТ СЧЁТЧИКА n (каждые 100 мс) ----------
  if (now - lastNTime >= 100) {
    lastNTime = now;
    n++;
  }

  // ---------- 4. ИЗМЕНЕНИЕ PWM (только при активной записи) ----------
  if (recordingEnabled && !eepromFull) {
    if (now - pwmIntervalTimer >= 5000) {
      pwmIntervalTimer = now;
      pwmValue += PWM_STEP;
      if (pwmValue > PWM_MAX) pwmValue = 0;
      analogWrite(PWM_PIN, pwmValue);
    }
  }

  // ---------- 5. ПЛАНОВЫЕ ИЗМЕРЕНИЯ И ЗАПИСЬ (каждые 100 мс) ----------
  if (now - lastDisplayTime >= 100) {
    lastDisplayTime = now;

    int adcValue = analogRead(ADC_PIN);
    float voltage = adcValue * (5.0 / 1023.0);

    // Запись в EEPROM
    if (recordingEnabled && !eepromFull) {
      uint16_t valueToStore = adcValue;
      EEPROM.put(eepromAddr, valueToStore);
      eepromAddr += BYTES_PER_MEASURE;
      if (eepromAddr >= EEPROM_SIZE) {
        eepromFull = true;
        recordingEnabled = false;
        lcd.setCursor(12, 0);
        lcd.print("FULL");
        delay(1000);
        while (1) delay(1000);   // остановка
      }
    }

    // Вывод на дисплей
    lcd.setCursor(2, 0);
    lcd.print("              ");
    lcd.setCursor(2, 0);
    lcd.print(n);

    lcd.setCursor(11, 0);
    lcd.print("   ");
    lcd.setCursor(11, 0);
    lcd.print(pwmValue);

    lcd.setCursor(4, 1);
    lcd.print("    ");
    lcd.setCursor(4, 1);
    lcd.print(adcValue);

    lcd.setCursor(11, 1);
    lcd.print("     ");
    lcd.setCursor(11, 1);
    lcd.print(voltage, 3);

    lcd.setCursor(14, 0);
    if (recordingEnabled && !eepromFull) {
      lcd.print("...");
    } else {
      lcd.print("   ");
    }
  }
}