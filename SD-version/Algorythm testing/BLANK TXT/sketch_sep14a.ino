#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
#define SS_SD 10
#define ENC_SW 3
TMRpcm audio;

volatile bool recStarted = false;
volatile bool recEnded = true;
unsigned long i = 0;
//bool recMode = false;
//byte arr[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
byte arr[16] = {254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, 239};
File f;
bool encrypted = false;

int encryptIndex = 1;
int decryptIndex = 16;
void setup() {
  Serial.begin(115200);
  SD.begin(SS_SD);
  audio.CSPin = SS_SD;
  pinMode(ENC_SW, INPUT_PULLUP);

  for (int i = 1; i < 17; i++) {
    _encrypt(arr, i);
  }
  for (int i = 0; i < 16; i++) {
    Serial.print(arr[i]);
    Serial.print("  ");
  }
  Serial.println();
  for (int i = 16; i > 0; i--) {
    _decrypt(arr, i);
  }
  for (int i = 0; i < 16; i++) {
    Serial.print(arr[i]);
    Serial.print("  ");
  }
  if (!SD.begin(SS_SD)) {
    return;
  } else {
    Serial.println("SD OK");
  }
  if (digitalRead(3) == LOW) {
    attachInterrupt(1, button, FALLING);
  }
  Serial.println(SD.exists("2.wav"));
  f = SD.open("1.txt", FILE_WRITE);
  
}

void loop() {
  for(int i = 0; i < 256; i++){
    f.write(i);
  }
  f.close();
  Serial.println();
//  if (digitalRead(3) == LOW && !encrypted) {
//    Serial.println(f.size());
//    long sizE = f.size();
//    f.seek(45);
//    unsigned long pos = f.position();
//    for (long i = 45; i < sizE; i += 16) {
//
//      byte arr[16];
//      for (int j = 0; j < 16; j++) {
//        arr[j] = f.read();
//      }
//      f.seek(pos);
//      for (int g = 1; g <= 16; g++) {
//        _encrypt(arr, g);
//      }
//      f.write(arr, 16);
////      encryptIndex++;
////      if (encryptIndex > 16) encryptIndex = 1;
//      pos += 16;
//      Serial.print(i);
//      Serial.print(" ");
//      Serial.println(f.position());
//
//    }
//    f.close();
//    Serial.println("Encrypted");
//    encrypted = true;
//  }
}


void button() {
  if (!recStarted) {
    recStarted = true;
    //    recEnded = true;
    Serial.println("rec started");
    audio.startRecording("1.wav", 16000, A0);

  } else if (recStarted) {

    audio.stopRecording("1.wav");
    //    recEnded = true;
    Serial.println("rec ended");

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

byte reverseArray(byte arr[], byte start, byte enD)
{
  while (start < enD)
  {
    byte temp = arr[start];
    arr[start] = arr[enD];
    arr[enD] = temp;
    start++;
    enD--;
  }
}
