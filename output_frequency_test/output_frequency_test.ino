#define SS_PC 9

#include <SPI.h>

#include <GyverHacks.h>

bool outputSyncSignalBool = false;
unsigned long outputSyncSignalTimer = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(SS_PC, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00001001;
  setPin(SS_PC, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (micros() - outputSyncSignalTimer > 996) {
    setPin(SS_PC, LOW);
    SPI.transfer(outputSyncSignalBool ? 200 : 0);
    setPin(SS_PC, HIGH);
    outputSyncSignalTimer = micros();
    outputSyncSignalBool = !outputSyncSignalBool;
  }
}
