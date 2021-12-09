#include <LiquidCrystalRus.h>
#include <Wire.h>
#include <DS1307.h>
   
LiquidCrystalRus lcd(5, 4, 9, 8, 7, 6);
DS1307 rtc(2, 3);

void setup() {
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Set the clock to run-mode
  rtc.halt(false);
  
  // Setup Serial connection
  Serial.begin(9600);
  // Print a message to the LCD.
  lcd.setCursor(0,0);
  lcd.print("Hello");
  lcd.setCursor(0,1);
  lcd.print("Fulliton");
}

void loop() {
 delay(1000);
 lcd.setCursor(0,0);
 lcd.print(rtc.getTimeStr());
 lcd.setCursor(0,1);
 lcd.print("");
}
