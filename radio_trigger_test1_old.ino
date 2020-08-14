/* radio_trigger_test1
 * first attempt at figuring this out, not great but it worked
 * see radio_trigger_test2
 */

//#define ADC_5V  (5.0 / 1023.0)
#define SAMPLES 30
#define AVG     128   /* Originally used 8 but this works better when delay is removed */
//#define WAIT    16   /* seems to smooth things out */
#define A_PIN   A0
#define MATCHES 1

int16_t baseMax = 0, baseMin = 1023;
//byte aRead = 0;

void setup() {
  int16_t avg = 0;
  pinMode(A_PIN, INPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  //Serial.begin(9600);
  
  for (int i = 0; i < SAMPLES; i++) {

    for (int j = 1; j <= AVG; j++) {
      //aRead = analogRead(A_PIN) * ADC_5V;
      avg += (analogRead(A_PIN) - avg) / j;
    }

    if (avg > baseMax) baseMax = avg;
    if (avg < baseMin) baseMin = avg;
  }
}

void loop() {
  int16_t avg = 0;
  byte match = 0;
  int16_t val[SAMPLES]  = { 0 };
  static bool sigDetect = false;
  static bool ledOn = true;
  static byte sigCount = 0;
  static int16_t sigLev = 0;
  
  for (int i = 0; i < SAMPLES; i++) {

    for (int j = 1; j <= AVG; j++) {
      //aRead = analogRead(A_PIN) * ADC_5V;
      avg += (analogRead(A_PIN) - avg) / j;
    }

    val[i] = avg;
    //Serial.println(val[i]);
    if (avg > baseMin && avg < baseMax) {
      sigLev = avg;
      if (!sigDetect) {
        //for (int j = 1; j <= MATCHES; j++)
          match += avg == val[i-1] ? 1 : 0;
          
        if (match == MATCHES) {
          sigCount += 1;
          sigDetect = true;
          digitalWrite(13, ledOn ? LOW : HIGH);
          ledOn = ledOn ? false : true;
        }
      }
    } else if (avg > sigLev || avg == 0) {
      for (int j = 1; j <= 3; j++)
          match += avg == val[i-j] ? 1 : 0;
      if (match == 3)
        sigDetect = false;
    }
    //Serial.println(sigDetect ? "true":"false");
    //delay(WAIT);
  }
}
