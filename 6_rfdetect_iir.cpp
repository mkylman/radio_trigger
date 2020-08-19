/* rfdetect_v6 for ATmega328
 * 8/19/2020
 * thanks to _MG_ and the DemonSeedEDU
 * changes: different method
 *          iir filtering and threshold
 *          ADC prescaler of 16
 *          range is down from 15-16ft to 12ft
 */
//#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/pgmspace.h>

// TIMER0 stuff
// count milliseconds

#define TIMERSET \
TCNT0   = 0; \
TCCR0A  = 0; \
TCCR0B |= (1<<CS00) | (1<<CS01); \
TIMSK0 |= (1<<TOIE0)

#define PERIOD 1000

uint32_t volatile ms = 0;

ISR(TIMER0_OVF_vect) {
  ms++;
}

// ADC stuff

// ADMUX set VCC as ref, select A0 pin
// ADCSRA enable freerun and interrupt
// prescaler [111: 128] 110: 64 101: 32 100: 16  | (1 << ADPS1) | (1 << ADPS0)
// prescaler x 13.5 x 16MHz
// reads per ms (0.972ms)
// 128 - 9 | 64 - 18 | 32 - 36 | 16 - 76

#define ADCSET ADMUX = 0b01000000; \
ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2);\
DIDR0 = 0

// 0b01000000 - enable ADSC to start a conversion
#define CONVERT ADCSRA |= (1 << ADSC)

// combine ADCL and ADCH for full resolution
#define TENBIT (ADCL | ADCH<<8)

uint32_t volatile peak      = 0;
uint32_t volatile peakSum   = 0;
uint32_t volatile thresh    = 0;
uint32_t volatile threshSum = 0;
uint32_t volatile iirStrong = 0;
uint16_t volatile strPeak   = 0;
uint32_t volatile iirSSum   = 0;
uint32_t volatile iirWeak   = 0;
uint16_t volatile wPeak     = 0;
uint32_t volatile iirWSum   = 0;
uint8_t STRG = 7;
uint8_t PEAK = 1;
uint8_t WEAK = 3;
uint8_t THRS = 15;

void iir(int32_t *avg, int32_t *sum, uint16_t input, uint8_t strength) {
  *sum = *sum - *avg + input;
  *avg = (*sum + (1<<(strength - 1))>>strength);
}

void threshPeaks() {

  uint16_t val = TENBIT;
  
  iir(&peak, &peakSum, val > peak ? val : (peak - 1), PEAK);
  iir(&iirStrong, &iirSSum, val, STRG);
  strPeak = (iirStrong > strPeak) ? iirStrong : (strPeak-1);
  iir(&iirWeak, &iirWSum, val, WEAK);
  
  wPeak = (iirWeak > wPeak) ? iirWeak : (wPeak-1);
  
  val = (peak > strPeak+2) ? ((peak - strPeak) >> 1) + strPeak : thresh;
  iir(&thresh, &threshSum, val, THRS);
}

ISR(ADC_vect) {
  threshPeaks();
}

// set led pin to output
#define LEDSET DDRB |= (1 << PORTB5)
// 0b00x00000 - LED ON/OFF
#define LEDON  PORTB |= (1 << PORTB5)
// 0b00000000 - LED OFF
#define LEDOFF PORTB &= (0 << PORTB5)

void initialize() {
  LEDSET;
  ADCSET;
  TIMERSET;
  sei();
  CONVERT; // start free running
}

int main(void) {
  uint32_t onOff = 300;
  uint32_t offTime = 0;
  uint32_t onTime = 0;
  uint32_t offFor = 0;
  uint16_t onFor = 0;

  uint8_t detect = 0;
  uint8_t count = 0;
  
  initialize();

  while (1) {
    if (wPeak < thresh || peak < thresh) {
      onTime = ms;
      onFor = onTime - offTime;

      if (!detect /* && onFor > 50*/) {
        detect = 1;
        onOff = 1100;
        count++;
        if (count == 3) {
          LEDON;
          count = 0;
        }
      }

    } else {
      offTime = ms;
      offFor = offTime - onTime;
      if (offFor > 300) {  // at least 300ms between
        if (detect) {
          detect = 0;
          LEDOFF;
          onOff = 300;
        }
        if (offFor > 2000) // 2 seconds of nothing, reset
          count = 0;
      }
    }
  }
}
