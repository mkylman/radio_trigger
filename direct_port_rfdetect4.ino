/* direct_port_rfdetect4 for ATmega328
 * 8/15/2020
 * thanks to _MG_ and the DemonSeedEDU
 * changes: range increase to just under 15ft
 *          using TIMER0 to measure unique presses
 */
 #include <avr/io.h>

// TIMER0 STUFF
// configure to count milliseconds
#define PERIOD 1000
#define CLEARTIMER TCNT0 = 0b00000000
#define CLEARMODE TCCR0A = 0
// overflow every ms
#define PRESCALE64 TCCR0B |= (1<<CS00) | (1<<CS01)
#define ENABLEOVER TIMSK0 |= (1<<TOIE0)

int16_t timerOverflow = 0;
uint32_t ms = 0;
uint32_t offTime = 0;
uint32_t onTime = 0;

ISR(TIMER0_OVF_vect) {
  timerOverflow++;
  ms++;
  if (timerOverflow >= PERIOD)
    timerOverflow = 0;
}


// ADC stuff

// REFS1 - 0, REFS0 - 1 = VCC as reference
// MUX3:0 - 0 = use ADC0
#define CONFIGADC ADMUX |= 0b01000000

// enable ADC, set prescalers to max
#define ENABLEADC ADCSRA |= 0b10000111

// 0b01000000 - enable ADSC to start a conversion
#define CONVERT ADCSRA |= (1 << ADSC)

// wait until ADSC is reset at conversion end
#define CONVERTING ADCSRA & (1 << ADSC)

// combine ADCL and ADCH for full resolution
#define TENBIT (ADCL | ADCH<<8)

// set led pin to output
#define LEDSET DDRB |= (1 << PORTB5)
// 0b00x00000 - LED ON/OFF
#define LEDON  PORTB |= (1 << PORTB5)
// 0b00000000 - LED OFF
#define LEDOFF PORTB &= (0 << PORTB5)

int16_t aRead() {
  CONVERT;
  while (CONVERTING);
  return (TENBIT);
}

void minMax(int16_t val, int16_t *oMin, int16_t *oMax){
  if (val < *oMin) *oMin = val;
  if (val > *oMax) *oMax = val;
}

void initialize() {
  LEDSET;
  CONFIGADC;
  ENABLEADC;

  CLEARTIMER;
  CLEARMODE;
  PRESCALE64;
  ENABLEOVER;
  sei();
}

int main(void) {
  initialize();

  int16_t baseMax = 0;
  int16_t baseMin = 1023;
  bool detecting = false;
  byte count = 0;
  
  for (byte i = 0; i < 30; i++)
    minMax(aRead(), &baseMin, &baseMax);

  while (1) {
    int16_t newMax = baseMin;
    int16_t newMin = baseMax;
    
    for (byte i = 0; i < 20; i++)
      minMax(aRead(), &newMin, &newMax);
    
    if (newMin > baseMin && newMax < baseMax) {
      onTime = ms;

      if (!detecting) {
        detecting = true;
        count++;
      }

    } else {
      offTime = ms;
      if (offTime - onTime > 200) {
        detecting = false;
        LEDOFF;
        if (offTime - onTime > 2000)
          count = 0;
      }
    }
    
    if (count == 3) {
      LEDON;
      count = 0;
    }
  }
}
