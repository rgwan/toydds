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

#define LED_PORT			B
#define LED_PORT_BIT		3

#define DAC_PORT			A


static inline void hw_init()
{
	CLKPR=0x80;
	CLKPR=0x00;
	Set_Bit(DDRPORT(LED_PORT), LED_PORT_BIT);
	Set_Bit(OUTPORT(LED_PORT), LED_PORT_BIT);

	OUTPORT(DAC_PORT) = 0x80;
	DDRPORT(DAC_PORT) = 0xFF; //Set R2R DAC to output
}
/*
 * Arguments:
 * wave is the DDS wave that the function will generate
 * acc[h-l] is the phase adder value.
 * The function typically require 1K-aligned 1K wave(1Kpts)
 * If you want to stop the signal out, you have to Pull up the Stack Pointer! 
 * and it will return to main()
 * The phase adder is 24+2bit length. By using r18 r19 r30 & r31[0-1]
 * fs = SysClock / cycles(12) = 1.3MS/s (typically)
 * fo(MAX) = 1.3MS/s * 2^24 / 2^26
 * fo(MIN&Resolution) = 1.3MS/s / 2^26
 * 
*/
static void DDS_OUT(const unsigned char *wave, unsigned char acch, \
						unsigned char accm, unsigned char accl)
{//Once call it will never return.Until the INT0 is being triggered.
// The Z pointer will be automatically changed by the compiler
	asm volatile(	"ldi r20, 0x04		"		"\n\t"	
					"add r20, r31		;r20 = ZH+2""\n\t"
					"eor r18, r18 		;r18<-0"	"\n\t"
					"eor r19, r19 		;r19<-0"	"\n\t" 
					"1:"						"\n\t"
					"lpm 				;2 cycles" 	"\n\t"
					"out %0, __tmp_reg__	;1 cycle"	"\n\t"	
					"add r18, %1		;1 cycle"			"\n\t"
					"adc r19, %2		;1 cycle"			"\n\t"	
					"adc r30, %3		;1 cycle"			"\n\t"
					"adc r31, __zero_reg__;1 cycle"			"\n\t"
					"cpse r31, r20		;1/2 cycles"			"\n\t"
					"rjmp .+2			;2/0 cycles"					"\n\t"
					"subi r31, 4		;0/1 cycles"		"\n\t"
					"rjmp 1b			;0/2 cycles"	"\n\t"
					:
					:"I" ( _SFR_IO_ADDR(OUTPORT(DAC_PORT))),\
							"r" (accl),"r" (accm),"r" (acch), "e" (wave)
		);
}

int main()
{
	hw_init();
	DDS_OUT(Square_Table,0x40, 0xff, 0xff);
}
