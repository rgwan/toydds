#ifndef __concat_h
#define __concat_h
//Some macros to hack on AVR IO port

#define CONCAT(a, b) a ## b
#define CONCAT_EAPANDED(a, b) CONCAT(a, b)

#define OUTPORT(name) CONCAT(PORT, name)
#define INPORT(name)  CONCAT(PIN, name)
#define DDRPORT(name) CONCAT(DDR, name)

#define Set_Bit(val, bitn)    (val |=(1<<(bitn)))
#define Clr_Bit(val, bitn)    (val&=~(1<<(bitn)))
#define Get_Bit(val, bitn)    (val &(1<<(bitn)) )

#endif
