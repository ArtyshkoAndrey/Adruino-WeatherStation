#include <LiquidCrystal.h>
#include <Wire.h>
#include <DS1307.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <EEPROM.h>

// В меню времени более часто опрашиваем кнопки
#define INTERVAL_IN_TIME_MODE 200
// Интервал опроса кнопок если не в меню времени
#define INTERVAL_ISNT_TIME_MODE 500
// Тот самый номер пина датчика
#define DHTPIN 10

#define VERSION "1.4"

LiquidCrystal lcd(5, 4, 9, 8, 7, 6);
DS1307 rtc(2, 3);
Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHT11);

long long viewerMillis = 0;
long long getButtonClickMillis = 0;
long long updateBMP180Millis = -5000;

// Переменная в которой лежит обьект времени с часов
Time t;

// Переменная для хранения Часа минут и секунд, что бы по позиции редактировтаь
String hour = "00";
String min = "00";
String sec = "00";

// С какой переодичностью будет выводить новую информацию на экран без режима изменения времени
long long viewerInterval = 1000;
// С какой переодичностью будет слежить за кнопками
long long buttonClickInterval = INTERVAL_IN_TIME_MODE;

long long updateBMP180Interval = 5000;

// Для режима что бы менять время
bool rewriteTimeMode = false;

// Если время изменили то записать, иначе оставить как есть
bool editableTime = false;

// Какой экран показывать
int screen = 2;

// Позиция курсора когда редачим время
unsigned int xPos = 0;

unsigned int mmg = 0;
unsigned int temp = 0;
unsigned int hum = 0;

unsigned int recordsTemp[2];
// 0 - minTemp
// 1 - maxTemp
String rescordsDates[2];
// 0 - date MaxTemp
// 1 - date MinTemp

//--------------------------//
// Fore Game Data           //
//--------------------------//
bool gamer = false;
unsigned int counterGamer = 0;
bool startGamer = false;
int level = 1;    // переменная для отсчета уровня
int pause = 400;  // переменная для задержки
byte p = 0;       // переменная для времени прыжка

// создаем массивы дракончика, дерева, камня и птицы
byte dracon[8] = {
  0b01110, 0b11011, 0b11111, 0b11100, 0b11111, 0b01100, 0b10010, 0b11011
};
byte derevo[8] = {
  0b00000, 0b00000, 0b00000, 0b11011, 0b11011, 0b11011, 0b01100, 0b01100
};
byte kamen[8] = {
  0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01110, 0b11111
};
byte ptica[8] = {
  0b00100, 0b00101, 0b01111, 0b11111, 0b10100, 0b00100, 0b00000, 0b00000
};

//--------------------------//
// End Game Data            //
//--------------------------//

void setup() {

  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  //GAME DATA
  lcd.createChar(0, dracon);
  lcd.createChar(1, derevo);
  lcd.createChar(2, kamen);
  lcd.createChar(3, ptica);

  //END GAME DATA

  // Set the clock to run-mode
  rtc.halt(false);

  // Print a message to the LCD.
  lcd.clear();
  cursor(0, 0);
  lcd.print("Meteostation");
  cursor(0, 1);
  lcd.print("Version ");
  lcd.print(VERSION);

  if (!bmp.begin()) {
    lcd.clear();
    lcd.print("BMP180 - ERROR");
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {
    }
  }

  dht.begin();

  // clearRecords
//   saveRecords(0, 0);

  getRecords();

  delay(1000);
}

void getRecords() {
  recordsTemp[0] = EEPROM.read(0);
  recordsTemp[1] = EEPROM.read(3);
}

void saveRecords(int min, int max) {
  EEPROM.write(0, min);
  EEPROM.write(3, max);
}

// Нажатие на кнопку Слушаем на какие кнопки нажимаем
void ListenBooton() {
  unsigned long currentMillis = millis();
  if (currentMillis - getButtonClickMillis > buttonClickInterval) {

    getButtonClickMillis = currentMillis;
    int data = analogRead(A1);

    switch (data) {
      case 0 ... 30:
        Serial.println("RIGHT");
        funRightButton();
        break;
      case 31 ... 150:
        Serial.println("UP");
        funUpButton();
        break;
      case 151 ... 360:
        Serial.println("DOWN");
        if (rewriteTimeMode == false && gamer == false) {
          counterGamer++;

          if (counterGamer > 10) {
            gamer = true;
            startGamer = true;
            counterGamer = 0;
          }
        } else {
          funDownButton();
        }
        break;
      case 361 ... 535:
        Serial.println("LEFT");
        funLeftButton();
        break;
      case 536 ... 760:
        Serial.println("SELECT");
        if (rewriteTimeMode == false && gamer == true) {
          counterGamer++;

          if (counterGamer > 10) {
            gamer = false;
            lcd.clear();
            counterGamer = 0;
          }
        } else {
          funSelectButton();
        }

        break;
    }

    if ((data >= 31 && data <= 360) && rewriteTimeMode) {

      cursor(0, 0);
      Serial.println(hour + ":" + min + ":" + sec);
      lcd.print(hour + ":" + min + ":" + sec);
      cursor(0, 1);
      lcd.print("                ");
      cursor(xPos, 0);

      editableTime = true;
    }
  }
}

// Нажатие на кнопку Селект
void funSelectButton() {
  rewriteTimeMode = !rewriteTimeMode;

  Serial.print("Change rewriteTimeMode: ");
  Serial.println(rewriteTimeMode);

  // Если включили режим изменения времени, то выводит один раз время
  // Останавливаем вывод в лупе
  // Запускаем мигающий курсор

  // Если выключили то убирает мигание и запускаем вывод всей информации
  if (rewriteTimeMode == true) {
    buttonClickInterval = INTERVAL_IN_TIME_MODE;
    xPos = 0;
    cursor(0, 0);
    Serial.println(hour + ":" + min + ":" + sec);
    lcd.print(hour + ":" + min + ":" + sec);
    cursor(0, 1);
    lcd.print("                ");
    cursor(xPos, 0);
    lcd.blink();

    // Задержка системы что бы дважди не кликнуть за одно нажатие
    delay(500);
  } else if (rewriteTimeMode == false) {
    if (editableTime) {
      rtc.setTime(hour.toInt(), min.toInt(), sec.toInt());
    }
    buttonClickInterval = INTERVAL_ISNT_TIME_MODE;
    lcd.noBlink();
    xPos = 0;
    cursor(xPos, 0);
  }
}

// Нажатие на кнопку Вправо
void funRightButton() {
  if (rewriteTimeMode == true) {
    if (xPos < 7) {
      xPos++;
      if (xPos == 2 || xPos == 5) {
        xPos++;
      }


      cursor(xPos, 0);
    }
  }
}

// Нажатие на кнопку Влево
void funLeftButton() {
  if (rewriteTimeMode == true) {
    if (xPos > 0) {
      xPos--;
      if (xPos == 2 || xPos == 5) {
        xPos--;
      }


      cursor(xPos, 0);
    }
  }
}

// Нажатие на кнопку Вверх
void funUpButton() {
  if (rewriteTimeMode) {
    // Если 1 позиция
    if (xPos == 0) {
      char hDecChar = hour.charAt(0);
      int hD = hDecChar - '0';

      char hChar = hour.charAt(1);
      int h = hChar - '0';

      hD++;
      // Ограничение до 2 десятков
      if (hD >= 3) {
        hD = 2;
      }

      // Если сделали 20-ки часов то младшая ступень должна быть менее 5
      if (hD == 2 && h > 4) {
        h = 4;
        hChar = h + '0';
        hour.setCharAt(1, hChar);
      }

      hDecChar = hD + '0';

      hour.setCharAt(0, hDecChar);
    }
    // Если 2 позиция
    if (xPos == 1) {
      char hChar = hour.charAt(1);
      int h = hChar - '0';

      char hDecChar = hour.charAt(0);
      int hD = hDecChar - '0';

      h++;

      if (hD == 2 && h > 4) {
        h = 4;
      }

      if (h >= 10) {
        h = 9;
      }

      hChar = h + '0';

      hour.setCharAt(1, hChar);
    }

    if (xPos == 3) {
      char mChar = min.charAt(0);
      int m = mChar - '0';

      if (m < 5) {
        m++;
      }

      mChar = m + '0';

      min.setCharAt(0, mChar);
    }

    if (xPos == 4) {
      char mChar = min.charAt(1);
      int m = mChar - '0';

      if (m < 9) {
        m++;
      }

      mChar = m + '0';

      min.setCharAt(1, mChar);
    }

    if (xPos == 6) {
      char sChar = sec.charAt(0);
      int s = sChar - '0';

      if (s < 5) {
        s++;
      }

      sChar = s + '0';

      sec.setCharAt(0, sChar);
    }

    if (xPos == 7) {
      char sChar = sec.charAt(1);
      int s = sChar - '0';

      if (s < 9) {
        s++;
      }

      sChar = s + '0';

      sec.setCharAt(1, sChar);
    }
  }
}

// Нажатие на кнопку Вниз
void funDownButton() {
  if (rewriteTimeMode) {
    // Если 1 позиция
    if (xPos == 0) {
      char hDecChar = hour.charAt(0);
      int hD = hDecChar - '0';

      // Ограничение до 0 десятков
      if (hD > 0) {
        hD--;
        hDecChar = hD + '0';
        hour.setCharAt(0, hDecChar);
      }
    }
    // Если 2 позиция
    if (xPos == 1) {
      char hChar = hour.charAt(1);
      int h = hChar - '0';

      if (h > 0) {
        h--;
        hChar = h + '0';
        hour.setCharAt(1, hChar);
      }
    }

    if (xPos == 3) {
      char mChar = min.charAt(0);
      int m = mChar - '0';

      if (m > 0) {
        m--;
        mChar = m + '0';
        min.setCharAt(0, mChar);
      }
    }

    if (xPos == 4) {
      char mChar = min.charAt(1);
      int m = mChar - '0';

      if (m > 0) {
        m--;
        mChar = m + '0';
        min.setCharAt(1, mChar);
      }
    }

    if (xPos == 6) {
      char sChar = sec.charAt(0);
      int s = sChar - '0';

      if (s > 5) {
        s--;
        sChar = s + '0';
        sec.setCharAt(0, sChar);
      }
    }

    if (xPos == 7) {
      char sChar = sec.charAt(1);
      int s = sChar - '0';

      if (s > 0) {
        s--;
        sChar = s + '0';
        sec.setCharAt(1, sChar);
      }
    }
  }
}

// Сдвиг курсота на осям
void cursor(int x, int y) {
  lcd.setCursor(x, y);
}

// Вывод информации на экран
void updateViewer() {
  unsigned long currentMillis = millis();
  if (currentMillis - viewerMillis > viewerInterval) {

    viewerMillis = currentMillis;
    if (!rewriteTimeMode && !gamer) {

      if (screen == 1) {
        cursor(0, 0);
        updateLocaleTime();
        lcd.print(hour + ":" + min + ":" + sec + "        ");
        cursor(0, 1);
        lcd.print(temp);
        lcd.print("*C ");

        lcd.print(hum);
        lcd.print("%  ");

        lcd.print(mmg);
        lcd.print("Mmg");
      } else if (screen == 2) {
        cursor(0, 0);
        lcd.print("Max Temp: ");
        lcd.print(recordsTemp[1]);
        lcd.print("*C  ");
        cursor(0, 1);
        lcd.print("Min Temp: ");
        lcd.print(recordsTemp[0]);
        lcd.print("*C   ");
      }
    }
  }
}

// Взятие данных о времени с датчика
void updateLocaleTime() {
  t = rtc.getTime();
  int h = t.hour;
  hour = h;
  if (h < 10) {
    hour = "0" + String(h);
  }

  int m = t.min;
  min = m;
  if (m < 10) {
    min = "0" + String(m);
  }

  int s = t.sec;
  sec = s;
  if (s < 10) {
    sec = "0" + String(s);
  }

  Serial.println(hour + ":" + min + ":" + sec);
}

// Обновление температуры и давления
void updateBMP180() {
  unsigned long currentMillis = millis();
  if (currentMillis - updateBMP180Millis > updateBMP180Interval) {
    if (screen == 1) {
      screen = 2;
    } else {
      screen = 1;
    }

    updateBMP180Millis = currentMillis;
    bool flagUpdateEeprom = false;
    temp = bmp.readTemperature();

    float pa = bmp.readPressure();
    pa = pa * 0.007500637554192;
    mmg = pa;

    hum = dht.readHumidity();

    if (temp < recordsTemp[0] || recordsTemp[0] == 0) {
      flagUpdateEeprom = true;
      recordsTemp[0] = temp;
    }

    if (temp > recordsTemp[1]) {
      flagUpdateEeprom = true;
      recordsTemp[1] = temp;
    }

    if (flagUpdateEeprom) {
      saveRecords(recordsTemp[0], recordsTemp[1]);
    }
  }
}

void loop() {

  ListenBooton();
  updateBMP180();

  updateViewer();

//  Game();
}

void Game() {
  if (startGamer) {
    lcd.clear();
    lcd.setCursor(7, 0);
    lcd.print("GO!");
    delay(1000);
    tone(10, 600);
    delay(100);
    noTone(10);
    lcd.clear();
    startGamer = false;
  } else if (gamer) {

    // первоначальное положение дракончика и препятствия
    byte d = 1;
    byte x = 15;
    byte y = 1;
    // выбираем препятствие, которое появится, рандомно
    byte i = random(1, 4);
    if (i == 3) y = 0;
    else
      y = 1;

    while (x > 0) {
      // очищаем экран и выводим номер уровня
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(level);

      // считываем данные с кнопки и учитываем количество циклов в прыжке
      // если дакончик находится в прыжке долго - возвращаем его вниз
      int buttonData = analogRead(A1);
      if (buttonData >= 31 && buttonData <= 150) d = 0;
      else
        d = 1;

      if (p > 3) d = 1;

      if (buttonData >= 536 && buttonData <= 760) {
        x = -1;
        gamer = false;
        break;
      }

      // выводим дракончика в нужной строке
      lcd.setCursor(4, d);
      lcd.print(char(0));
      // выводим препятствие
      lcd.setCursor(x, y);
      tone(10, 50);
      lcd.print(char(i));
      noTone(10);

      // если дракончик наткнулся на препятствие выводим надпись GAME OVER!
      if (x == 4 && y == d) {
        delay(400);
        tone(10, 50);
        delay(100);
        noTone(10);
        delay(100);
        tone(10, 20);
        delay(300);
        noTone(10);
        lcd.clear();
        delay(200);
        lcd.setCursor(3, 0);
        lcd.print("GAME OVER!");
        delay(600);
        lcd.clear();
        delay(400);
        lcd.setCursor(3, 0);
        lcd.print("GAME OVER!");
        delay(600);
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("LEVEL: ");
        lcd.print(level);
        delay(400);
        lcd.setCursor(3, 0);
        lcd.print("GAME OVER!");
        delay(3000);
        lcd.clear();

        // начинаем игру заново, обнулив уровень игры
        lcd.setCursor(7, 0);
        lcd.print("GO!");
        delay(400);
        tone(10, 600);
        delay(100);
        noTone(10);
        lcd.clear();

        level = 0;
        pause = 400;
        p = 0;
        y = 1;
        x = 0;
        break;
      }

      // если дракончик прыгнул, издаем звук
      if (d == 0) {
        tone(10, 200);
        delay(100);
        noTone(10);
      } else {
        delay(100);
      }

      // если дракончик не столкнулся, то меняем положение препятствия
      // начинаем считать сколько циклов дракончик находится в прыжке
      delay(pause);
      x = x - 1;
      p = p + 1;
      if (p > 4) p = 0;
    }

    // переходим на следующий уровень и сокращаем время задержки
    tone(10, 800);
    delay(20);
    level = level + 1;
    pause = pause - 50;
    if (pause < 0) pause = 0;
  }
}
