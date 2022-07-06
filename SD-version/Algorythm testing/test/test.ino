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

char* configName;
char current;
byte coding[16][3];


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
//      File dir = SD.open("/Config");
//      Serial.println(getFilesCount(dir));
    }
    
  }
}

void loop() {
  // put your main code here, to run repeatedly:

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
  if (!((configName[strlen(configName) - 1] == 'A' || configName[strlen(configName) - 2] == 'R' || configName[strlen(configName) - 3] == 'T') || (configName[strlen(configName) - 1] == 'a' || configName[strlen(configName) - 2] == 'r' || configName[strlen(configName) - 3] == 't'))) {
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

  EEPROM.write(configStateAddress, 100);
  EEPROM.put(configNameAddress, configName);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Config file:");
  lcd.setCursor(0, 1);
  lcd.print(configName);
  EEPROM.write(configStateAddress, 100);
  configFile.close();
  delay(4000);
  lcd.clear();
  
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

void configChoosing() {
  File dir = SD.open("/Config");
  byte b = getFilesCount(dir);
//  Serial.println(b = getFilesCount(dir));
  Serial.println(b);
  char* names[b];
  getFileNames(dir, b, names);
  dir.close();
  for (int i = 0; i < b; i++) {
    Serial.println(names[i]);
  }
//  lcdDisplayChoice(b, names, "Choose config:", configName);
//  Serial.println(configName);
//  File configFile = SD.open(configName);
//  writeConfig(configFile);
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

char* lcdDisplayChoice(int optionsLength, char** options, char* message, char* res) {
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
    if (!MODE) {
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
  res = result;
}
