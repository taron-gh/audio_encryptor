#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#include <SPI.h>

#include <GyverHacks.h>

#include <Keypad.h>

#include <Wire.h>

#include <EEPROM.h>

#include <LiquidCrystal_I2C.h>
//****************PINS*********
#define MIC_PIN 1 //Analog
#define PC_IN 0 //Analog
#define SS_USER 10
#define SS_PC 9
#define ENC_S1 A6     //Analog
#define ENC_S2 A3 //Analog
#define ENC_SW A2 //Analog
#define BUT_PIN A7 //Analog
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {12, 8, 7, 6}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

LiquidCrystal_I2C lcd(0x27, 16, 2);
//***********SETTINGS**********
#define KEY_DIGITS 8 //Maximum is your display's length / 2
#define COMPANIONS_FORMULA_MAP_ADDRESS 0
#define MY_FORMULA_MAP_ADDRESS 100
#define NUMBER_CODES_ADDRESS 200
#define RANDOM_BOOL_ADDRESS 300
//***********NUMBERS***********
//#define zero "5*"
//#define one "9#"
//#define two "57"
//#define three "94"
//#define four "*1"
//#define five "#8"
//#define six "73"
//#define seven "64"
//#define eight "#6"
//#define nine "7*"

char numbers[10][2] = {
  {'5', '*'},
  {'9', '#'},
  {'5', '7'},
  {'9', '4'},
  {'*', '1'},
  {'#', '8'},
  {'7', '3'},
  {'6', '4'},
  {'#', '6'},
  {'7', '*'},
};
byte myFormulaMap[KEY_DIGITS];
char myKey[KEY_DIGITS * 2];
char hisKey[KEY_DIGITS * 2];
byte hisFormulaMap[KEY_DIGITS];


//***********SYSTEM VARIABLES********
bool isOk = false;
byte pos = 0;
bool setupEnable = false;
bool secretMode = false;
//secret mode
byte num = 0;
bool isWaiting = true;
bool buttonWasPressed = false;
bool MODE = false;
bool firstGenerated = false;
//*********PROCESSING VARIABLES*******
byte encryptionFormulaCounter = 0;
byte decryptionFormulaCounter = 0;
bool syncAllowed = true;
bool syncStarted = false;
bool lastSyncStarted = false;
unsigned long encryptionTimer = 0;
unsigned long frequencyTimer = 0;
unsigned long receivingTimer = 0;
unsigned long countdownTimer = 0;
int frequencyCounter = 0;
unsigned long receivingCounter = 0;
byte syncCountdown = 0;
byte receivingCountdown = 0;
byte lastReceivingCountdown = 0;
bool countdownAllowed = false;
bool syncBool = false;
byte frequencyMeasureConrol = 0;
bool sendingAllowed = false;
bool receivingAllowed = false;
bool isInMiddle = false;
byte maxSignal = 0;
byte minSignal = 255;
byte pcSignal;
byte micSignal;


void setup() {
  pinMode(SS_USER, OUTPUT);
  pinMode(SS_PC, OUTPUT);
  Serial.begin(115200);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));
  EEPROM.begin();
  bool temp;
  EEPROM.get(RANDOM_BOOL_ADDRESS, temp);
  if (!temp) {
    firstGenerated = false;
  } else {
    firstGenerated = true;
  }
  if (EEPROM.read(MY_FORMULA_MAP_ADDRESS) == 255 && EEPROM.read(MY_FORMULA_MAP_ADDRESS + 1) == 255 && EEPROM.read(MY_FORMULA_MAP_ADDRESS + 2) == 255 && EEPROM.read(MY_FORMULA_MAP_ADDRESS + 3) == 255 && EEPROM.read(MY_FORMULA_MAP_ADDRESS + 4) == 255 && EEPROM.read(MY_FORMULA_MAP_ADDRESS + 5) == 255 && EEPROM.read(MY_FORMULA_MAP_ADDRESS + 6) == 255 && EEPROM.read(MY_FORMULA_MAP_ADDRESS + 7) == 255) {
    for (int i = 0; i < KEY_DIGITS; i++) {
      myFormulaMap[i] = 0;
    }
    EEPROM.put(MY_FORMULA_MAP_ADDRESS, myFormulaMap);
  } else {
    EEPROM.get(MY_FORMULA_MAP_ADDRESS, myFormulaMap);
  }
  if (EEPROM.read(COMPANIONS_FORMULA_MAP_ADDRESS) == 255 && EEPROM.read(COMPANIONS_FORMULA_MAP_ADDRESS + 1) == 255 && EEPROM.read(COMPANIONS_FORMULA_MAP_ADDRESS + 2) == 255 && EEPROM.read(COMPANIONS_FORMULA_MAP_ADDRESS + 3) == 255 && EEPROM.read(COMPANIONS_FORMULA_MAP_ADDRESS + 4) == 255 && EEPROM.read(COMPANIONS_FORMULA_MAP_ADDRESS + 5) == 255 && EEPROM.read(COMPANIONS_FORMULA_MAP_ADDRESS + 6) == 255 && EEPROM.read(COMPANIONS_FORMULA_MAP_ADDRESS + 7) == 255) {
    for (int i = 0; i < KEY_DIGITS; i++) {
      hisFormulaMap[i] = 0;
    }
    EEPROM.put(COMPANIONS_FORMULA_MAP_ADDRESS, hisFormulaMap);
  } else {
    EEPROM.get(COMPANIONS_FORMULA_MAP_ADDRESS, hisFormulaMap);
  }
  byte emptyBytes = 0;
  for (int i = NUMBER_CODES_ADDRESS; i < NUMBER_CODES_ADDRESS + 20; i++) {
    if (EEPROM.read(i) == 255 || EEPROM.read(i) == 0) {
      emptyBytes++;
    } else {
      //      Serial.println(EEPROM.read(i));
      break;
    }
  }
  if (emptyBytes > 0) {
    EEPROM.put(NUMBER_CODES_ADDRESS, numbers);
  } else {
    EEPROM.get(NUMBER_CODES_ADDRESS, numbers);
  }
  lcd.init();
  lcd.backlight();
  //INTERNAL REFERENCE, LEFT ADJSUTED RESULT
  cbi(ADMUX, REFS1);
  sbi(ADMUX, REFS0);
  sbi(ADMUX, ADLAR);
  setPin(SS_USER, HIGH);
  setPin(SS_PC, HIGH);
  //Prescaler
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  //   Пины D9 и D10 - 62.5 кГц
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00001001;  // x1 fast pwm
  // Пины D3 и D11 - 62.5 кГц
  TCCR2B = 0b00000001;  // x1
  TCCR2A = 0b00000011;  // fast pwm
  if (analogRead(BUT_PIN) > 500 && analogRead(ENC_SW) == 0) {
    syncStarted = true;
    Serial.println("Sync started");
  }

  if (analogRead(BUT_PIN) > 500) {
    MODE = true;
    buttonWasPressed = true;
  }
  randomSeed(analogRead(MIC_PIN));
  Serial.println("   Bool    ");
  Serial.print(firstGenerated);
  Serial.println();
  if (analogRead(ENC_SW) == 0) {
    MODE = true;
    for (int i = 0; i < KEY_DIGITS; i++) {
      if (!firstGenerated) {
        myFormulaMap[i] = (byte)random(0, 10);
        EEPROM.put(RANDOM_BOOL_ADDRESS, true);
      }

      Serial.print(myFormulaMap[i]);
      for (int j = 0; j < 2; j++) {
        myKey[pos + j] = numbers[myFormulaMap[i]][j];
      }
      pos += 2;
    }
    EEPROM.put(MY_FORMULA_MAP_ADDRESS, myFormulaMap);
    Serial.println();
    lcd.setCursor(0, 0);
    lcd.print("Key |Continue:OK");
    lcd.setCursor(0, 1);
    for (int i = 0; i < KEY_DIGITS * 2; i++) {
      lcd.write(myKey[i]);
      Serial.print(myKey[i]);
    }
  } else {
    for (int i = 0; i < KEY_DIGITS; i++) {
      Serial.print(myFormulaMap[i]);
      for (int j = 0; j < 2; j++) {
        myKey[pos + j] = numbers[myFormulaMap[i]][j];
      }
      pos += 2;
    }
  }
  pos = 0;

  Serial.println();
  Serial.print("My key: ");
  for (int i = 0; i < KEY_DIGITS; i++) {
    Serial.print(myFormulaMap[i]);
  }
  Serial.print("   ");
  if (!MODE) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("My key:");
    lcd.setCursor(0, 1);
  }

  for (int i = 0; i < KEY_DIGITS * 2; i++) {
    Serial.print(myKey[i]);
    if (!MODE) lcd.write(myKey[i]);
  }
  Serial.println();
  Serial.print("Companion's key: ");
  for (int i = 0; i < KEY_DIGITS; i++) {
    Serial.print(hisFormulaMap[i]);
  }
  Serial.println();
  Serial.print("Number codes: ");
  Serial.println();
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 2; j++) {
      Serial.print(numbers[i][j]);
      Serial.print("  ");
    }
    Serial.println();
  }
}

void loop() {
  if (MODE == true) {
    SPI.end();
    pinMode(12, INPUT);
    if (analogRead(ENC_SW) > 0) {
      setupEnable = true;
      delay(100);
    }
    if (analogRead(BUT_PIN) == 0 && buttonWasPressed) {
      setupEnable = true;
      delay(100);
    }
    if (analogRead(ENC_SW) < 20 && setupEnable) {
      isOk = true;
    } else if (analogRead(BUT_PIN) > 500 && setupEnable) {
      secretMode = true;
    }

    //**************COMPAMION KEY CHANGE MODE***********
    if (isOk) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Companion's key:");
      lcd.setCursor(0, 1);
      while (pos < KEY_DIGITS * 2) {
        char ch = keypad.getKey();
        if (ch && ch != 'A' && ch != 'B' && ch != 'C' && ch != 'D') {
          lcd.write(ch);
          Serial.print(ch);
          hisKey[pos] = ch;
          pos++;
        }
      }
      Serial.println();
      char buff[2];
      byte index = 0;
      for (int i = 0; i < pos; i++) {
        buff[i % 2] = hisKey[i];
        if (i % 2 == 1) {
          for (int j = 0; j < 10; j++) {
            if (isArrEqual(buff, numbers[j], 2, 2)) {
              hisFormulaMap[index] = j;
              Serial.print(hisFormulaMap[index]);
            }
          }
          index++;
        }
      }
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Done!");
      lcd.setCursor(1, 1);
      lcd.print("Please reset.");
      EEPROM.put(COMPANIONS_FORMULA_MAP_ADDRESS, hisFormulaMap);
      isOk = false;
    }

    //***********NUMBER CODES CHANGE MODE**************
    if (secretMode) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Confirm-C");
      lcd.setCursor(0, 1);
      lcd.print("Delete-D");
      lcd.setCursor(11, 0);
      lcd.print("Num:");
      lcd.setCursor(9, 1);
      lcd.print("Code:");
      byte filled = 2;
      while (num < 10) {
        char ch = keypad.getKey();
        lcd.setCursor(15, 0);
        lcd.print(num);
        lcd.setCursor(14, 1);
        lcd.print(numbers[num][0]);
        lcd.setCursor(15, 1);
        lcd.print(numbers[num][1]);
        char oldFirst = numbers[num][0];
        char oldSecond = numbers[num][1];
        if (ch) {
          if (ch == 'A') {
            if (filled == 2) {
              byte counter = 0;
              for (int i = 0; i < 10; i++) {
                if (numbers[i][0] == numbers[num][0] && numbers[i][1] == numbers[num][1]) {
                  counter++;
                }
              }
              if (counter == 1) {
                num++;
              } else {
                lcd.clear();
                lcd.setCursor(3, 0);
                lcd.print("This code");
                lcd.setCursor(0, 1);
                lcd.print("already exists!");
                delay(2000);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Confirm-C");
                lcd.setCursor(0, 1);
                lcd.print("Delete-D");
                lcd.setCursor(11, 0);
                lcd.print("Num:");
                lcd.setCursor(9, 1);
                lcd.print("Code:");
                lcd.setCursor(15, 0);
                lcd.print(num);
                lcd.setCursor(14, 1);
                lcd.print(numbers[num][0]);
                lcd.setCursor(15, 1);
                lcd.print(numbers[num][1]);
              }
              //              num++;
              //              break;
            } else {
              numbers[num][0] = oldFirst;
              numbers[num][1] = oldSecond;
              num++;
              //              break;
            }
          } else if (ch == 'B') {
            if (filled == 2) {
              byte counter = 0;
              for (int i = 0; i < 10; i++) {
                if (numbers[i][0] == numbers[num][0] && numbers[i][1] == numbers[num][1]) {
                  counter++;
                }
              }
              if (counter == 1) {
                num--;
              } else {
                lcd.clear();

                lcd.setCursor(3, 0);
                lcd.print("This code");
                lcd.setCursor(0, 1);
                lcd.print("already exists!");
                delay(2000);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Confirm-C");
                lcd.setCursor(0, 1);
                lcd.print("Delete-D");
                lcd.setCursor(11, 0);
                lcd.print("Num:");
                lcd.setCursor(9, 1);
                lcd.print("Code:");
                lcd.setCursor(15, 0);
                lcd.print(num);
                lcd.setCursor(14, 1);
                lcd.print(numbers[num][0]);
                lcd.setCursor(15, 1);
                lcd.print(numbers[num][1]);
              }
              //              break;
            } else {
              numbers[num][0] = oldFirst;
              numbers[num][1] = oldSecond;
              num--;
              //              break;
            }
          } else if (ch == 'C') {
            if (filled == 2) {
              byte counter = 0;
              for (int i = 0; i < 10; i++) {
                if (numbers[i][0] == numbers[num][0] && numbers[i][1] == numbers[num][1]) {
                  counter++;
                }
              }
              if (counter == 1) {
                num++;
              } else {
                lcd.clear();
                lcd.setCursor(3, 0);
                lcd.print("This code");
                lcd.setCursor(0, 1);
                lcd.print("already exists!");
                delay(2000);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Confirm-C");
                lcd.setCursor(0, 1);
                lcd.print("Delete-D");
                lcd.setCursor(11, 0);
                lcd.print("Num:");
                lcd.setCursor(9, 1);
                lcd.print("Code:");
                lcd.setCursor(15, 0);
                lcd.print(num);
                lcd.setCursor(14, 1);
                lcd.print(numbers[num][0]);
                lcd.setCursor(15, 1);
                lcd.print(numbers[num][1]);
              }
              //              break;
            }
          } else if (ch == 'D') {
            if (filled == 2) {
              numbers[num][1] = '-';
              filled--;
            } else if (filled == 1) {
              numbers[num][0] = '-';
              filled--;
            }
          } else {
            if (filled == 0) {
              numbers[num][0] = ch;
              filled++;
            } else if (filled == 1) {
              numbers[num][1] = ch;
              filled++;
            }
          }
        }
      }
      EEPROM.put(NUMBER_CODES_ADDRESS, numbers);
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Done!");
      lcd.setCursor(1, 1);
      lcd.print("Please reset.");
      secretMode = false;
    }
    SPI.begin();
  } else {
    pcSignal = (analogRead(PC_IN) >> 2) + 40;
    micSignal = (analogRead(MIC_PIN) >> 2) + 40;
    if (syncAllowed && syncStarted) {
      if (micros() - encryptionTimer > 970) { //500Hz
        encryptionTimer = micros();
        setPin(SS_PC, LOW);
        SPI.transfer(syncBool ? 200 : 50);
        setPin(SS_PC, HIGH);
        syncBool = !syncBool;
      }
    }
    if (syncStarted) {
      if(!lastSyncStarted) delay(1500);
      if (pcSignal > maxSignal) {
        maxSignal = pcSignal;
      }
      if (pcSignal < minSignal) {
        minSignal = pcSignal;
      }
      lastSyncStarted = true;

    }

    //    Serial.println(isInMiddle);
    if (pcSignal <= ((maxSignal + minSignal) >> 1) + 5 && pcSignal >= ((maxSignal + minSignal) >> 1) - 5 && syncStarted) {
      if (!isInMiddle) {
        //        Serial.println(micros() - frequencyTimer);
        if (micros() - frequencyTimer > 1250) {
          frequencyCounter = 0;

        } else if (micros() - frequencyTimer > 750) {
          frequencyCounter++;

        }
        if (micros() - frequencyTimer > 400000) {
          lastReceivingCountdown = receivingCountdown;
        }
//                int a = micros() - frequencyTimer;
//        Serial.println(frequencyCounter);
        if (frequencyCounter > 500) {

          countdownAllowed = true;
          Serial.println("Countdown allowed");
          syncAllowed = true;

        } else if (frequencyCounter > 100) {

          receivingCountdown = lastReceivingCountdown + 1;
          Serial.println(receivingCountdown);

        }

        frequencyTimer = micros();
      }
      isInMiddle = true;
    } else {
      frequencyMeasureConrol++;
      if (frequencyMeasureConrol > 1) {
        isInMiddle = false;
      }
    }
    if (countdownAllowed && syncCountdown < 6) {
      if (millis() - countdownTimer > 1000) {
        countdownTimer = millis();
        syncAllowed = !syncAllowed;
        syncCountdown++;
        //        Serial.println(syncCountdown);
      }
    } else if (countdownAllowed) {
      syncAllowed = false;
      sendingAllowed = true;
      //      Serial.println("Synced");
    }

    if (receivingCountdown > 5) {
      if (!receivingAllowed) Serial.println("Synced");
      receivingAllowed = true;
    }
    if (analogRead(BUT_PIN) < 50 && sendingAllowed) {
      setPin(SS_PC, LOW);
      SPI.transfer(encrypt(((analogRead(MIC_PIN) >> 2) + 40), myFormulaMap[encryptionFormulaCounter]));
      setPin(SS_PC, HIGH);
      encryptionFormulaCounter++;
    } else if (receivingAllowed) {
      setPin(SS_USER, LOW);
      SPI.transfer(decrypt(((analogRead(PC_IN) >> 2) + 40), hisFormulaMap[decryptionFormulaCounter]));
      setPin(SS_USER, HIGH);
      decryptionFormulaCounter++;
    }
  }
  if (encryptionFormulaCounter > 9) {
    encryptionFormulaCounter = 0;
  }
  if (decryptionFormulaCounter > 9) {
    decryptionFormulaCounter = 0;
  }
  if (analogRead(BUT_PIN) > 500 && analogRead(ENC_SW) == 0 && !sendingAllowed && !receivingAllowed) {
    syncStarted = true;
    Serial.println("Sync started");
  }
}
byte encrypt(byte num, byte formula) {
  switch (formula) {
    case 0:
      return num + 10;
      break;
    case 1:
      return num + 20;
      break;
    case 2:
      return num + 30;
      break;
    case 3:
      return num + 40;
      break;
    case 4:
      return num + 50;
      break;
    case 5:
      return num + 60;
      break;
    case 6:
      return num + 70;
      break;
    case 7:
      return num + 80;
      break;
    case 8:
      return num + 90;
      break;
    case 9:
      return num + 100;
      break;
  }
}

byte decrypt(byte num, byte formula) {
  switch (formula) {
    case 0:
      return num - 10;
      break;
    case 1:
      return num - 20;
      break;
    case 2:
      return num - 30;
      break;
    case 3:
      return num - 40;
      break;
    case 4:
      return num - 50;
      break;
    case 5:
      return num - 60;
      break;
    case 6:
      return num - 70;
      break;
    case 7:
      return num - 80;
      break;
    case 8:
      return num - 90;
      break;
    case 9:
      return num - 100;
      break;
  }
}


bool isArrEqual(byte A[], byte B[], byte lenA, byte lenB) {
  if (lenA != lenB)
    return false;
  for (int i = 0; i < lenA; i++)
    if (A[i] != B[i])
      return false;
  return true;
}
