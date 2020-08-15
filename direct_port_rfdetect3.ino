/* direct_port_rfdetect3 for ATmega328
 * Michael Kylman
 * 8/14/2020
 * thanks to _MG_ and the DemonSeedEDU
 * changes: range increase to just under 10ft
 */
#include <avr/io.h>
#include <util/delay.h>

// 0b01000000 - enable ADSC to start a conversion
#define READADC ADCSRA |= (1 << ADSC)

// wait until ADSC is reset and read is done
#define WAITADC while (ADCSRA & (1 << ADSC))

// combine ADCL and ADCH for full resolution
#define TENBIT (ADCL | ADCH<<8)

// REFS1 - 0, REFS0 - 1 = VCC as reference
// MUX3:0 - 0 = use ADC0
#define CONFIGADC ADMUX |= 0b01000000

// enable ADC, set prescalers to max
#define ENABLEADC ADCSRA |= 0b10000111

// 0b00100000 - LED ON
#define LEDON  PORTB |= (1 << PORTB5)
// 0b00000000 - LED OFF
#define LEDOFF PORTB &= (0 << PORTB5)

int16_t aRead() {
  _delay_ms(1);
  READADC;
  WAITADC;
  return (TENBIT);
}

void minMax(int16_t val, int16_t *oMin, int16_t *oMax){
  if (val < *oMin) *oMin = val;
  if (val > *oMax) *oMax = val;
}

int main(void) {
  DDRB  |= (1 << PORTB5);// set led pin to output
  CONFIGADC;
  ENABLEADC;

  int16_t baseMax = 0;
  int16_t baseMin = 1023;
  
  for (byte i = 0; i < 30; i++)
    minMax(aRead(), &baseMin, &baseMax);
  
  while (1) {
    int16_t newMax = baseMin;
    int16_t newMin = baseMax;
    
    for (byte i = 0; i < 20; i++)
      minMax(aRead(), &newMin, &newMax);

    if (newMin > baseMin && newMax < baseMax)
      LEDON;
    else LEDOFF;
  }
}
