/* low_level_rfdetect3 for ATmega328
 * Michael Kylman
 * 8/14/2020
 * thanks to _MG_ and the DemonSeedEDU
 * changes: range increase to just under 10ft
 */
#include <avr/io.h>
#include <util/delay.h>

// SQUASH can use range of 0 - 7
#define SQUASH 3

byte aRead() {
  _delay_ms(1);
  ADCSRA |= (1 << ADSC); // 0b01000000 - enable ADSC to start a conversion
  while (ADCSRA & (1 << ADSC)); // wait until ADSC is reset and read is done
  
  return ((ADCL | ADCH<<8) >> SQUASH); // smash it way down
}

void minMax(byte val, byte *oMin, byte *oMax){
  if (val < *oMin) *oMin = val;
  if (val > *oMax) *oMax = val;
}

int main(void) {
  byte baseMax = 0;
  byte baseMin = 255;
  
  DDRB  |= (1 << PORTB5);// 0b00100000 - set led pin to output
  ADMUX |= 0b01000000;   // REFS1 to 0, REFS0 to 1: set VCC as reference voltage
                         // select ADC0
  ADCSRA |= 0b10000111;  // enable ADC, set prescalers to max
  
  for (byte i = 0; i < 30; i++)
    minMax(aRead(), &baseMin, &baseMax);
  
  while (1) {
    byte newMax = baseMin;
    byte newMin = baseMax;
    for (byte i = 0; i < 20; i++)
      minMax(aRead(), &newMin, &newMax);

    if (newMin > baseMin && newMax < baseMax)
      PORTB |= (1<<PORTB5); // 0b00100000 - LED ON
    else
      PORTB &= (0<<PORTB5); // 0b00000000 - LED OFF
  }
}
