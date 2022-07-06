#define PC_IN A0
byte pcSignal;
const byte buffSize = 5;
byte buff[buffSize];
byte index;
unsigned long timer;
unsigned long outputTimer;
bool wasRising = false;
long frequency;
int syncInputFrequencyCounter;
unsigned long test = 0;
void setup() {
  Serial.begin(115200);
  cbi(ADMUX, REFS1);
  sbi(ADMUX, REFS0);
  sbi(ADMUX, ADLAR);
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
//  test = micros();
//  analogRead(PC_IN);
//  test = micros() - test;
//  Serial.println(test);
}

void loop() {
  pcSignal = (analogRead(PC_IN) >> 2) + 40;
  index = index < buffSize - 1 ? index + 1 : 0;
  buff[index] = pcSignal;
  //  if(index == 0){
  //    timer = micros();
  //  }else if(index == 19){
  //    Serial.println(micros() - timer);
  //    timer = micros();
  //  }
  if (buff[0] < buff[buffSize - 1] && index == buffSize - 1 && !wasRising) {
    frequency = micros() - timer;
    timer = micros();
    Serial.println(frequency);
    wasRising = true;
//    if (frequency > 1950 && frequency < 2220) {
//      syncInputFrequencyCounter++;
//      Serial.println(syncInputFrequencyCounter);
//    }
  } else if (buff[0] > buff[buffSize - 1] && index == buffSize - 1) {
    wasRising = false;
  }
  //  if(millis() - outputTimer > 200){

  //    outputTimer = millis();
  //  }
}
