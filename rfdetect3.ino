/* rfdetect3 for UNO
 * 8/14/2020
 * thanks to _MG_ and the DemonSeedEDU
 * changes: range increase to just under 10ft using byte and SQUASH on aRead pass to minMax
 *          range increase to 12ft using int16_t and no SQUASH
 */
// SQUASH can be between 0 - 7
//#define SQUASH 3

// this will change depending on MCU
#define PIN    A0
#define LEDPIN 13

void minMax(int16_t val, int16_t *oMin, int16_t *oMax){
  if (val < *oMin) *oMin = val;
  if (val > *oMax) *oMax = val;
}

int16_t baseMax = 0;
int16_t baseMin = 1023;
int16_t aRead = 0;

void setup() {
  pinMode(LEDPIN, OUTPUT);
  // get our baseline readings
  for (byte i = 0; i < 30; i++) {
    aRead = analogRead(PIN);
    minMax((aRead), &baseMin, &baseMax);
  }
}

void loop() {
  int16_t newMax = baseMin;
  int16_t newMin = baseMax;
  
  for (byte i = 0; i < 20; i++) {
    aRead = analogRead(PIN);
    minMax((aRead), &newMin, &newMax);
  }

  if (newMin > baseMin && newMax < baseMax)
    digitalWrite(LEDPIN, HIGH);
  else
    digitalWrite(LEDPIN, LOW);
}
