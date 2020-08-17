/* direct_port_rfdetect4 for ATmega328
 * 8/16/2020
 * thanks to _MG_ and the DemonSeedEDU
 * changes: range increase to:
 *              16ft held horizontal to
 *              13ft pointed at
 *              10ft held vertical to
 *          using TIMER0 to measure unique presses
 *          lowered ADC prescaler to 64 for faster samples
 */
//#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/pgmspace.h>

// TIMER0 STUFF
// configure to count milliseconds
#define TIMERSET \
TCNT0   = 0; \
TCCR0A  = 0; \
TCCR0B |= (1<<CS00) | (1<<CS01); \
TIMSK0 |= (1<<TOIE0)

#define PERIOD 1000

uint16_t timerOverflow = 0;
uint32_t ms = 0;

ISR(TIMER0_OVF_vect) {
  timerOverflow++;
  ms++;
  if (timerOverflow >= PERIOD)
    timerOverflow = 0;
}


// ADC stuff

#define ADCSET ADMUX |= 0b01000000; \
ADCSRA |= 0b10000110

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
/* use with lower prescaler below 64. However, affects range
uint32_t avg = 0;
uint16_t iir(uint16_t input, uint8_t strength) {
  avg = ((avg * strength) + input) / (strength + 1);
  input = avg; 
  return input;
}
*/
uint16_t aRead() {
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
  ADCSET;
  TIMERSET;
  sei();
}

int main(void) {
  uint32_t offTime = 0;
  uint32_t onTime = 0;
  uint16_t baseMax = 0;
  uint16_t baseMin = 1023;
  uint8_t detecting = 0;
  uint8_t count = 0;
  
  initialize();

  for (uint8_t i = 0; i < 30; i++)
    minMax(aRead(), &baseMin, &baseMax);

  while (1) {
    uint16_t newMax = baseMin;
    uint16_t newMin = baseMax;
    
    for (uint8_t i = 0; i < 20; i++)
      minMax(aRead(), &newMin, &newMax);
    
    if (newMin > baseMin && newMax < baseMax) {
      onTime = ms;

      if (!detecting) {
        detecting = 1;
        count++;
        if (count == 3) {
          LEDON;
          count = 0;
        }
      }

    } else {
      offTime = ms;
      if (offTime - onTime > 200) {
        detecting = 0;
        LEDOFF;
        if (offTime - onTime > 2000)
          count = 0;
      }
    }
  }
}}
