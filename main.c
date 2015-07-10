#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include <util/delay.h>
#include <avr/interrupt.h>
#include <lgt8f0xa.h>
#include <concat.h>

#include <sine.h>
#include <tri.h>
#include <sqr.h>
#include <blp.h>
#include <saw.h>
#include <revsaw.h>

#define LED_PORT			B
#define LED_PORT_BIT		3

#define DAC_PORT			A

static inline void hw_init()
{
CLKPR=0x80;
CLKPR=0x00;
	Set_Bit(DDRPORT(LED_PORT), LED_PORT_BIT);
	Set_Bit(OUTPORT(LED_PORT), LED_PORT_BIT);

	OUTPORT(DAC_PORT) = 0xff;
	DDRPORT(DAC_PORT) = 0xFF; //Set R2R DAC to output
}

static inline void DDS_OUT()
{

}

int main()
{
	hw_init();
	unsigned int i = 0;
	while(1)
	{
		OUTPORT(DAC_PORT) = pgm_read_byte(Sine_Table + i);
		i++;
if(i > 2047) i =0;
	}
}
