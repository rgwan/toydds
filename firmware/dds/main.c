/* Released at GPLv2
 * Author Zhiyuan Wan
 * E-mail: h@iloli.xyz
 * Have fun!
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h> 

#include <util/delay.h>
#include <avr/interrupt.h>
#include <lgt8f0xa.h>
#include <concat.h>

#include <sine.h>
#include <tri.h>
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
 * fs = SysClock / cycles(11) = 1.45MS/s (typically)
 * fo(MAX) = 1.45MS/s * 2^24 / 2^26 = 
 * fo(MIN&Resolution) = 1.45MS/s / 2^26
 * This WaveROM have to be aligned by 256bytes
*/
static void DDS_OUT(const unsigned char *wave, unsigned char acch, \
						unsigned char accm, unsigned char accl)
{//Once call it will never return.Until the INT0 is being triggered.
// The Z pointer will be automatically changed by the compiler
	asm volatile(	"ldi r21, 0x04		"		"\n\t"	
					"add r21, r31		;r20 = ZH+2""\n\t"
					"eor r19, r19 		;r18<-0"	"\n\t"
					"eor r20, r20 		;r19<-0"	"\n\t" 
					"1:"						"\n\t"
					"lpm 				;2 cycles" 	"\n\t"
					"out %0, __tmp_reg__	;1 cycle"	"\n\t"	
					"add r19, %1		;1 cycle"			"\n\t"
					"adc r20, %2		;1 cycle"			"\n\t"	
					"adc r30, %3		;1 cycle"			"\n\t"
					"adc r31, __zero_reg__;1 cycle"			"\n\t"
					"cpse r31, r21		;1/2 cycles"			"\n\t"
					"rjmp .+2			;2/0 cycles"					"\n\t"
					"subi r31, 4		;0/1 cycles"		"\n\t"
					"rjmp 1b			;0/2 cycles total 11 cycles"	"\n\t"
					:
					:"I" ( _SFR_IO_ADDR(OUTPORT(DAC_PORT))),\
							"r" (accl),"r" (accm),"r" (acch), "e" (wave)
		);
}
/* This is a simple way to generate Sawtooth Wave, the WaveROM is not required
 * r18,r19,r20 works as Phase_accumulator
 * But if you use SawWave generator, the calculate expression of fout is little different. 
 * This function generated samplerate up to 2.6MS/s */
static void Syn_SawWave( unsigned char acch, unsigned char accm, \
							unsigned char accl)
{
	asm volatile(	"eor r20, r20		"		"\n\t"	
					"eor r19, r19 		;r18<-0"	"\n\t"
					"eor r20, r20 		;r19<-0"	"\n\t" 
					"1:"						"\n\t"
					"out %0, r21		;1 cycle"	"\n\t"	
					"add r19, %1		;1 cycle"			"\n\t"
					"adc r20, %2		;1 cycle"			"\n\t"	
					"adc r21, %3;__zero_reg__		;1 cycle"			"\n\t"
					"rjmp 1b			;2 cycles, total 6cycles"	"\n\t"
					:
					:"I" ( _SFR_IO_ADDR(OUTPORT(DAC_PORT))),\
							"r" (accl),"r" (accm),"r" (acch)
		);
					
}
static void Syn_RevSawWave( unsigned char acch, unsigned char accm, \
							unsigned char accl)
{
	asm volatile(	"eor r21, r21		"		"\n\t"	
					"eor r19, r19 		;r18<-0"	"\n\t"
					"eor r20, r20 		;r19<-0"	"\n\t" 
					"1:"						"\n\t"
					"out %0, r21		;1 cycle"	"\n\t"	
					"sub r19, %1		;1 cycle"			"\n\t"
					"sbc r20, %2		;1 cycle"			"\n\t"	
					"sbc r21, %3		;1 cycle"			"\n\t"
					"rjmp 1b			;2 cycles, total 6cycles"	"\n\t"
					:
					:"I" ( _SFR_IO_ADDR(OUTPORT(DAC_PORT))),\
							"r" (accl),"r" (accm),"r" (acch)
		);
					
}

static void Syn_TriangleWave( unsigned char acch, unsigned char accm, \
							unsigned char accl)
{
	asm volatile(	"eor r20, r20		"		"\n\t"	
					"eor r18, r18 		;r18<-0"	"\n\t"
					"eor r19, r19 		;r19<-0"	"\n\t" 
					"1:"						"\n\t"
					"out %0, r20		;1 cycle"	"\n\t"	
					"add r18, %1		;1 cycle"			"\n\t"
					"adc r19, %2		;1 cycle"			"\n\t"	
					"adc r20, %3		;1 cycle"			"\n\t"
					"rjmp 1b			;2 cycles, total 6cycles"	"\n\t"
					:
					:"I" ( _SFR_IO_ADDR(OUTPORT(DAC_PORT))),\
							"r" (accl),"r" (accm),"r" (acch)
		);
					
}
/*
 * This function at 2MS/s speed to generate square wave from 0 - 80kHz
 */
static void Syn_SquareWave( unsigned char acch, unsigned char accm, \
							unsigned char accl)
{
	asm volatile(	"eor r21, r21		"		"\n\t"	
					"eor r19, r19 		;r18<-0"	"\n\t"
					"eor r20, r20 		;r19<-0"	"\n\t" 
					"ldi r22, 0xff		;r21=0xff"	"\n\t"
					"1:"						"\n\t"
					"add r19, %0		;1 cycle"			"\n\t"
					"adc r20, %1		;1 cycle"			"\n\t"	
					"adc r21, %2		;1 cycle"			"\n\t"
					"sbrc r21, 7		;2 cycles" "\n\t"
					"rjmp .+4			;2 cycles" "\n\t"
					"out %3, r22		;outputs 1"	"\n\t"
					"rjmp 1b			;2 cycles"	"\n\t"
					"out %3, __zero_reg__;outputs 0"	"\n\t"
					"rjmp 1b			;2 cycles, 8 cycles total"	"\n\t"
					:
					:		"r" (accl),"r" (accm),"r" (acch), \
						"I" ( _SFR_IO_ADDR(OUTPORT(DAC_PORT)))
		);
					
}

int main()
{
	hw_init();
	//Syn_RevSawWave(0x4, 0xff, 0xf6);
	DDS_OUT(Sine_Table, 0x38, 0xab, 0x00); //80kHz
}
