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

void setup() {
  Serial.begin(115200);
  SD.begin(SS_SD);
  audio.CSPin = SS_SD;
  pinMode(ENC_SW, INPUT);

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
  attachInterrupt(1, button, FALLING);
}

void loop() {
  //  if(Serial.available()){                          //Send commands over serial to play
  //      switch(Serial.read()){
  //        case 'r': audio.startRecording("test.wav",16000,A0); break;    //Record at 16khz sample rate on pin A0
  //        case 'R': audio.startRecording("test.wav",16000,A0,1); break;  //Record, but with passthrough to speaker.
  //        case 't': audio.startRecording("test.wav",16000,A0,2); break;  //Do not record. Output direct to speaker
  //                             //Note: If samples are dropped before writing, it
  //                             //      will not be heard in passthrough mode
  //        case 's': audio.stopRecording("test.wav"); break;              //Stop recording
  ////        case 'p': audio.play("test.wav"); break;                       //Play the recording
  ////        case '=': audio.volume(1); break;                              //Increase volume by 1. Does not affect recording
  ////        case '-': audio.volume(0); break;                              //Decrease volume by 1. Does not affect recording
  ////        case 'S': audio.stopPlayback(); break;                         //Stop all playback
  //
  //      }
  //    }
//  recEnded = false;
}
void button() {
  if (!recStarted) {
    recStarted = true;
//    recEnded = true;
    Serial.println("rec started");
    audio.startRecording("1.wav", 16000, A0);
    
  } else if (recStarted){

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
