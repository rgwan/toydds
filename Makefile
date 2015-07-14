#某懒人专用AVR-GCC的Makefile
#创建时间：2015-7-10
#版权没有，翻版不久
#第一个使用者：万致远
CROSS=avr-
GCC=$(CROSS)gcc #GCC
NM=$(CROSS)nm
OBJCOPY=$(CROSS)objcopy #objcopy
SIZE=$(CROSS)size
OBJDUMP=$(CROSS)objdump

FLASH_SIZE=8192
MEM_SIZE=504 #memory limit

F_CPU   = 16000000# Hertz
CFLAGS= -O2 -DF_CPU=$(F_CPU)UL #GCC flags
LDFLAGS = -Wl,--defsym=__stack=0x8002ff\
			-Wl,-section-start=.Wavetable=0x400 #set the stack to 0x2ff(512B)
ASMFLAGS = -DF_CPU=$(F_CPU)UL
MACHINE= atmega164p
NICKNAME=l8a
VERSION=1.0

ProjectName= toydds
INCLUDEDIR = -I.

HEX := $(addsuffix .hex, $(ProjectName))
EEP := $(addsuffix .eep, $(ProjectName))
LSS := $(addsuffix .lss, $(ProjectName))
ELF := $(addsuffix .elf, $(ProjectName))
SYM := $(addsuffix .sym, $(ProjectName))
REV := $(addsuffix .S, $(ProjectName))
BIN := $(addsuffix .bin, $(ProjectName))

CSRCS = main.c#C源代码
ASMSRCS = #汇编源代码
HDRS = sine.h sqr.h blp.h
OBJS =  $(patsubst %.S,%.Os,$(ASMSRCS)) $(patsubst %.c,%.o,$(CSRCS))
genASMOBJS = $(patsubst %.c,%.s,$(CSRCS)) $(patsubst %.c,%.i,$(CSRCS))

all: $(HDRS) $(SRCS) $(OBJS) $(ELF) $(HEX) $(BIN) $(EEP) $(LSS) $(SYM) $(REV) wavegenerator size

%.Os:%.S
	@echo "CCASM		$(patsubst %.S,%.Os,$<)"
	@$(GCC) $(INCLUDEDIR) -mmcu=$(MACHINE) $(ASMFLAGS) -c $< -o $(patsubst %.S,%.Os,$<)	
%.o:%.c
	@echo "CC		$(patsubst %.c,%.o,$<)"
	@$(GCC) $(INCLUDEDIR) -mmcu=$(MACHINE) --save-temp $(CFLAGS) -c $< -o $(patsubst %.c,%.o,$<) 
$(ELF):$(INC) $(MAIN) $(OBJS)
	@echo "CCLD		$(ELF)"
	@$(GCC) $(INCLUDEDIR) -mmcu=$(MACHINE) $(CFLAGS) $(OBJS) -o $(ELF) $(LDFLAGS)
$(HEX):$(ELF) $(INC) $(SRCS) $(OBJS)
	@echo "OBJCOPY		$(HEX)"
	@$(OBJCOPY) -j .text -j .data -j .Wavetable -O ihex $(ELF) $(HEX)
$(BIN):$(ELF)
	@echo "OBJCOPY		$(BIN)"
	@$(OBJCOPY) -j .text -j .data -j .Wavetable -O binary $(ELF) $(BIN)
$(EEP):$(ELF) $(INC) $(SRCS) $(OBJS)
	@echo "OBJCOPY		$(EEP)"
	@$(OBJCOPY) $(HEX_EEPROM_FLAGS) -O ihex $(ELF) $(EEP)

$(LSS): $(ELF) $(INC) $(SRCS) $(OBJS)
	@echo "OBJDUMP		$(LSS)"
	@avr-objdump -h -S $(ELF) > $(LSS)

$(REV):$(HEX) $(INC) $(SRCS) $(OBJS)
	@echo "DISASM			$(REV)"
	@avr-objdump -d $(ELF) > $(REV)

wavegenerator:wavegenerator.c
	@echo "CC			wavegenerator"
	@gcc wavegenerator.c -o wavegenerator -lm

$(SYM):$(INC) $(SRCS) $(OBJS) $(ELF) 
	@echo "NM		$(SYM)"
	@$(NM) -n $(ELF) > $(SYM)
clean:
	@echo "CLEAN 		$(OBJS) $(IHEX) $(HEX) $(BIN) $(ELF) $(LSS) $(EEP)"
	@rm -f $(OBJS) $(IHEX) $(HEX) $(REV) $(ELF) $(LSS) $(EEP)
	@echo "CLEAN		$(genASMOBJS)"
	@rm -f $(genASMOBJS) $(SYM) $(BIN)
tarball:clean
	@tar -jcf ../$(ProjectName)-$(VERSION).tar.bz2 *
	@echo "TAR 	../$(ProjectName)-$(VERSION).tar.bz2"
	@echo "已打包 ../$(ProjectName)-$(VERSION).tar.bz2"
size:${ELF}
	@echo "****************************** 程序信息 *************************************"
	@avr-size -C --mcu=${MACHINE} ${ELF}
	@echo "*****************************************************************************"
	@./checksize $(ELF) $(FLASH_SIZE) $(MEM_SIZE)
flash:all
	avarice --program  --erase -v --file $(BIN) --part $(MACHINE)  --mkII -j usb  
avarice:$(ELF)
	avarice --program  --erase --file $(ELF) --part $(MACHINE) --mkII -j usb localhost:4242
gdb.conf:$(ELF)
	@echo "file $(ELF)" > gdb.conf
	@echo "target remote localhost:4242" >> gdb.conf
ddd:gdb.conf
	ddd --debugger "avr-gdb -x gdb.conf"
gdb:gdb.conf
	avr-gdb -x gdb.conf
wavetable:wavegenerator
	./wavegenerator sine 1024 > sine.h
	./wavegenerator triangle 1024 > tri.h
	./wavegenerator square 1024 > sqr.h
	#./wavegenerator saw 1024 > saw.h
	#./wavegenerator revsaw 1024 > revsaw.h
	./wavegenerator bioplar 1024 > blp.h
help:
	@echo "用 'make' 来编译"
	@echo "用 'make clean' 全部清除"
	@echo "用 'make tarball' 会清除代码数并且打包"
	@echo "用 'make size' 会告诉你程序大小"
