#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>


LiquidCrystal_I2C lcd(0x27, 20, 4);
TMRpcm audio;



#define SS_SD 10
#define ENC_SW 3
#define MODE 0 //0-Serial control, 1 - encoder
#define configStateAddress 100 //50 - no config, 100 - configured,  
#define configNameAddress 300



char* configName = " ";
char current;
byte coding[16][3];
//unsigned long i = 0;
//File f;

//bool encrypted = false;
//
//int encryptIndex = 1;
//int decryptIndex = 16;


void setup() {
  Serial.begin(115200);
  audio.CSPin = SS_SD;
  audio.speakerPin = 9;
  pinMode(ENC_SW, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();

  if (!SD.begin(SS_SD)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error");
    lcd.setCursor(0, 1);
    lcd.print("No SD card");
    while (1);
  }

  if (EEPROM.read(configStateAddress) != 100) {
    Serial.println("no 100!");
    if (prompt("Generate random?", "Yes", "No")) {
      Serial.println("No config, generating");
      generateConfig();
      
    } else {
      Serial.println("No config, choosing");
      configChoosing();
    }
    EEPROM.write(configStateAddress, 100);
  } else {
    Serial.println("yes 100!");
    for(int i = 0; i < 13; i++){
      configName[i] = (char)EEPROM.read(configNameAddress + i);
    }
    char* configPath = "/Config/";
    for (int i = 8; i < strlen(configName) + 8; i++) {
      configPath[i] = configName[i - 8];
    }
//    Serial.println(configPath);
    File configFile = SD.open(configPath, FILE_WRITE);
    writeConfig(configFile);
  }
}

void loop() {
  if (MODE == 0) serialTick();
  if (current == 'C') configChoosing();
  if (current == 'G') generateConfig();
  
}




void serialTick() {
  if (Serial.available()) {
    current = Serial.read();
  } else {
    current = 'z';
  }
}

bool prompt(char* question, char* positive, char* negative) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(question);
  lcd.setCursor(0, 1);
  lcd.print(">");
  lcd.setCursor(1, 1);
  lcd.print(positive);
  lcd.setCursor(9, 1);
  lcd.print(negative);
  bool result = true;
  bool b = true;
  while (b) {
    if (MODE == 0) {
      if (Serial.available()) {
        switch (Serial.read()) {
          case 'y':
            result = true;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(question);
            lcd.setCursor(0, 1);
            lcd.print(">");
            lcd.setCursor(1, 1);
            lcd.print(positive);
            lcd.setCursor(9, 1);
            lcd.print(negative);
            break;
          case 'n':
            result = false;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(question);
            lcd.setCursor(8, 1);
            lcd.print(">");
            lcd.setCursor(1, 1);
            lcd.print(positive);
            lcd.setCursor(9, 1);
            lcd.print(negative);
            break;
          case 's':
            b = false;
            break;
        }
      }
    }
  }
  return result;
}

void configChoosing() {
  File dir = SD.open("/Config");
  byte b;
  Serial.println(b = getFilesCount(dir));
  char* names[b];
  getFileNames(dir, b, names);
  dir.close();
  //    for (int i = 0; i < b; i++) {
  //      Serial.println(names[i]);
  //    }
  Serial.println(configName = lcdDisplayChoice(b, names, "Choose config:"));
  File configFile = SD.open(configName);
  writeConfig(configFile);
  
}

void generateConfig() {
  char* path = "/Config/CONFIG_G.TXT";
  File configFile = SD.open("/Config/CONFIG_G.TXT", FILE_WRITE);
  configFile.seek(0);
  Serial.println(configFile.position());
  byte buff[48];
  randomSeed(analogRead(A0));
  for (byte i = 0; i < 48; i++) {
    bool pass = false;
    while (!pass) {
      bool repeated = false;
      buff[i] = random(0, 256);
      Serial.print(buff[i]);
      Serial.print(" ");
      for (int j = 0; j < i - 1; j++) {
        if (buff[i] == buff[j]) {
          repeated = true;
        }
      }
      if (!repeated) pass = true;
    }
    configFile.write(buff[i]);
    Serial.println(configFile.position());
  }
  Serial.println(configFile.name());
  configFile.seek(0);
  for (byte i = 0; i < 48; i++) {
    Serial.print(configFile.position());
    Serial.print("  ");
    Serial.print(configFile.read());
    Serial.print(" | ");
  }
  Serial.println();
  writeConfig(configFile);
}

void writeConfig(File configFile) {
  configFile.seek(0);
  configName = configFile.name();
  //  configFile.position();
  byte buff[48];
  byte index = 0;
  for (int i = 0; i < 16; i++) {
    Serial.print(i);
    Serial.print("  -  ");
    for (int j = 0; j < 3; j++) {
      coding[i][j] = configFile.read();
      buff[index] = coding[i][j];
      Serial.print(coding[i][j]);
      Serial.print("  ");
      index++;
    }
    Serial.println();
  }
  index = 0;
  bool isCorrect = true;
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 3; j++) {
      for (int g = 0; g < index - 1; g++) {
        if (coding[i][j] == buff[g]) {
          isCorrect = false;
          Serial.println("Writeconfig, config repeats");
        }
      }
      index++;
    }
  }
  Serial.println(strlen(configName));
  Serial.println(configName);
  if (!((configName[strlen(configName) - 1] == 'T' || configName[strlen(configName) - 2] == 'X' || configName[strlen(configName) - 3] == 'T') || (configName[strlen(configName) - 1] == 't' || configName[strlen(configName) - 2] == 'x' || configName[strlen(configName) - 3] == 't'))) {
    isCorrect = false;
    Serial.println("Write config, extension");
  }
  if (!isCorrect) {
    Serial.println("Write config, prompt");
    if (prompt("Generate random?", "Yes", "No")) {
      generateConfig();
      Serial.println("Write config, generating...");
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Config incorrect");
      lcd.setCursor(1, 1);
      lcd.print("Please reboot");
      while (1);
    }
  }

  for(int i = 0; i < 13; i++){
    EEPROM.write(configNameAddress + i, configName[i]);
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Config file:");
  lcd.setCursor(0, 1);
  lcd.print(configName);
  configFile.close();
  delay(4000);
  lcd.clear();
  
}


void sdCheck() {
  byte counter = 0;
  if (SD.exists("/Enc-ed")) {
    Serial.println("1");
    File f = SD.open("/Enc-ed");
    if (f.isDirectory()) counter++;
    f.close();
  }
  if (SD.exists("/Dec-ed")) {
    Serial.println("2");
    File f = SD.open("/Dec-ed");
    if (f.isDirectory()) counter++;
    f.close();
  }
  if (SD.exists("/For-enc")) {
    Serial.println("3");
    File f = SD.open("/For-enc");
    if (f.isDirectory()) counter++;
    f.close();
  }
  if (SD.exists("/For-dec")) {
    Serial.println("4");
    File f = SD.open("/For-dec");
    if (f.isDirectory()) counter++;
    f.close();
  }
  if (SD.exists("/Config")) {
    Serial.println("5");
    File f = SD.open("/Config");
    if (f.isDirectory()) counter++;
    f.close();
  }
  if (SD.exists("/Recs")) {
    Serial.println("6");
    File f = SD.open("/Recs");
    if (f.isDirectory()) counter++;
    f.close();
  }
  if (counter != 6) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error");
    lcd.setCursor(0, 1);
    lcd.print("Wrong SD format");
    while (1);
  }
}

void _encrypt(byte arr[], byte n) {
  if (n > 16 && n < 1) return;
  byte iterations;
  if (16 % n != 0) {
    iterations = (16 / n) + 1;
  } else {
    iterations = 16 / n;
  }
  byte start = 0;
  byte enD = n - 1;
  for (byte i = 0; i < iterations; i++) {
    reverseArray(arr, start, enD);
    start += n;
    enD += n;
    if (enD > 15) enD = 15;
    if (start > 15) start = 15;
  }
  for (int i = 0; i < 16; i++) {
    arr[i] ^= n;
    arr[i] += n * (n % (15 - n));
  }
}

void _decrypt(byte arr[], byte n) {
  if (n > 16 && n < 1) return;
  byte iterations;
  if (16 % n != 0) {
    iterations = (16 / n) + 1;
  } else {
    iterations = 16 / n;
  }
  byte start = 0;
  byte enD = n - 1;
  for (byte i = 0; i < iterations; i++) {
    reverseArray(arr, start, enD);
    start += n;
    enD += n;
    if (enD > 15) enD = 15;
    if (start > 15) start = 15;
  }
  for (int i = 0; i < 16; i++) {
    arr[i] -= n * (n % (15 - n));
    arr[i] ^= n;

  }
}

byte reverseArray(byte arr[], byte start, byte enD) {
  while (start < enD)
  {
    byte temp = arr[start];
    arr[start] = arr[enD];
    arr[enD] = temp;
    start++;
    enD--;
  }
}

byte getFilesCount(File f) {
  byte filesCount = 0;
  bool areFiles = false;
  File entry1 =  f.openNextFile();
  if (! entry1) {

    return;
  } else {
    filesCount++;
    areFiles = true;
  }
  while (areFiles) {
    File entry2 =  f.openNextFile();
    if (! entry2) {
      areFiles = false;
    } else {
      filesCount++;
    }
    entry2.close();
  }
  entry1.close();
  return filesCount;
}

void getFileNames(File f, byte fileCount, char** names) {
  f.rewindDirectory();
  for (byte i = 0; i < fileCount; i++) {
    File result = f.openNextFile();
    names[i] = strdup(result.name());
    result.close();
  }
  //  for (int i = 0; i < fileCount; i++) {
  //    Serial.println(names[i]);
  //  }
}

char* lcdDisplayChoice(int optionsLength, char** options, char* message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  lcd.setCursor(0, 1);
  lcd.print(">");
  lcd.setCursor(1, 1);

  char* result;
  lcd.print(options[0]);
  int pos = 0;
  bool b = true;
  //  for (int i = 0; i < optionsLength; i++) {
  //    Serial.println(options[i]);
  //  }
  while (b) {
    if (MODE == 0) {
      if (Serial.available()) {
        switch (Serial.read()) {
          case 'd':
            pos++;
            if (pos == optionsLength) pos = 0;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(message);
            lcd.setCursor(0, 1);
            lcd.print(">");
            lcd.setCursor(1, 1);
            lcd.print(options[pos]);
            break;
          case 'u':
            pos--;
            if (pos == -1) pos = optionsLength - 1;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(message);
            lcd.setCursor(0, 1);
            lcd.print(">");
            lcd.setCursor(1, 1);
            lcd.print(options[pos]);
            break;
          case 's':
            b = false;
            result = options[pos];
            break;
        }
      }

    }
    if (pos == -1) pos = optionsLength - 1;
    if (pos == optionsLength) pos = 0;
  }
  return result;
}

























//void loop() {
//  if (digitalRead(3) == LOW && !encrypted) {
//    Serial.println(f.size());
//    long sizE = f.size();
//        f.seek(45);
////    f.seek(0);
//    unsigned long pos = f.position();
//    for (long i = 45; i < sizE; i += 16) {
//
//      byte arr[16];
//      for (int j = 0; j < 16; j++) {
//        arr[j] = f.read();
//        Serial.print(arr[j]);
//        Serial.print(" ");
//      }
//      f.seek(pos);
//      Serial.println();
////      for (int g = 1; g < 17; g++) {
////        _encrypt(arr, g);
////      }
//      for (int g = 16; g > 0; g--) {
//        _decrypt(arr, g);
//      }
//      for (int g = 0; g < 16; g++) {
//        Serial.print(arr[g]);
//        Serial.print(" ");
//
//      }
//      Serial.println();
//      f.write(arr, 16);
//      //      encryptIndex++;
//      //      if (encryptIndex > 16) encryptIndex = 1;
//      pos += 16;
//      Serial.print(i);
//      Serial.print("    ");
//      Serial.println(f.position());
//      Serial.println();
//
//    }
//    f.close();
//    Serial.println("Encrypted");
//    encrypted = true;
//  }
//}
