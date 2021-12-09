#include <LiquidCrystal.h>
#include <Wire.h>
#include <DS1307.h>

LiquidCrystal lcd(5, 4, 9, 8, 7, 6);
DS1307 rtc(2, 3);

long viewerMillis = 0;
long getButtonClickMillis = 0;

// С какой переодичностью будет выводить новую информацию на экран без режима изменения времени
long viewerInterval = 1000;
// С какой переодичностью будет слежить за кнопками
long buttonClickInterval = 500;

// Для режима что бы менять время
bool rewriteTimeMode = false;

void setup() {
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Set the clock to run-mode
  rtc.halt(false);

  // Setup Serial connection
  Serial.begin(9600);
  // Print a message to the LCD.
  cursor(0, 0);
  lcd.print("Hello");
  cursor(0, 1);
  lcd.print("Version 1.0");
}

void ListenBooton() {
  unsigned long currentMillis = millis();
  if(currentMillis - getButtonClickMillis > buttonClickInterval) {

    getButtonClickMillis = currentMillis;
    int data = analogRead(A1);

    switch(data) {
      case 0 ... 30:
        Serial.println("RIGHT");
        break;
      case 31 ... 150:
        Serial.println("UP");
        break;
      case 151 ... 360:
        Serial.println("DOWN");
        break;
      case 361 ... 535:
        Serial.println("LEFT");
        break;
      case 536 ... 760:
        Serial.println("SELECT");
        funSelectButton();
        break;
    }
  }
}

void funSelectButton () {
  rewriteTimeMode = !rewriteTimeMode;

  Serial.print("Change rewriteTimeMode: ");
  Serial.println(rewriteTimeMode);
  
  // Если включили режим изменения времени, то выводит один раз время
  // Останавливаем вывод в лупе  
  // Запускаем мигающий курсор

  // Если выключили то убирает мигание и запускаем вывод всей информации  
  if (rewriteTimeMode == true) {
    buttonClickInterval = 100;
    cursor(0, 0);
    lcd.print(rtc.getTimeStr());
    lcd.blink();
    cursor(0, 0);
    delay(500);
  } else if (rewriteTimeMode == false) {
    buttonClickInterval = 500;
    lcd.noBlink();
    cursor(0, 0);
  }
}

void cursor (int x, int y) {
  lcd.setCursor(x, y);
}

void updateViewer() {
  unsigned long currentMillis = millis();
  if(currentMillis - viewerMillis > viewerInterval) {

    viewerMillis = currentMillis;
    if (!rewriteTimeMode) {
      cursor(0, 0);
      lcd.print(rtc.getTimeStr());
      cursor(0, 1);
      lcd.print("                ");
    }
  }
}

void loop() {
  ListenBooton ();

  updateViewer();
}


