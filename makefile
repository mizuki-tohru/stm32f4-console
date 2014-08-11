#TCHAIN  = /usr/cross/bin/arm-none-eabi
TCHAIN  = /usr/cross/bin/arm-m4-eabi
#TCHAIN  = arm-none-eabi
TOOLDIR = /bin
REMOVAL = /bin/rm
WSHELL  = /bin/bash
MSGECHO = /bin/echo

# OPTIMIZE Definition
#OPTIMIZE		= 2
OPTIMIZE		= 0

# FPU Definition
USING_FPU		= -mfloat-abi=softfp  -mfpu=fpv4-sp-d16
#USING_FPU		= -mfloat-abi=soft

# Apprication Version
APP_VER = W.I.P

SUBMODEL		= STM32F4XX

MPU_DENSITY		= STM32F40_41xxx
HSE_CLOCK 		= 8000000
USE_CLOCK		= USE_HSE
#USE_CLOCK		= USE_HSI
PERIF_DRIVER	= USE_STDPERIPH_DRIVER
USB				= USE_USB
BLUETOOTH		= 
#BLUETOOTH		= USE_BLUETOOTH
#BLUETOOTH		= USE_BLUETOOTH_HID
MRUBY			= 
#MRUBY			= USE_MRUBY

UART_DEFAULT_NUM	= 1
# UART_DEFAULT_NUM	= 2
#USE_FONT		= USE_KANJI
USE_FONT		= 

# Use FreeRTOS?
OS_SUPPORT		= BARE_METAL
#OS_SUPPORT		= USE_FREERTOS

#USING_DEBUG		= SERIAL_DEBUG

USE_BASIC = BASIC

# Synthesis makefile Defines
DEFZ = $(EVAL_BOARD) $(MPU_DENSITY) $(PERIF_DRIVER) $(VECTOR_START) $(ROM_START)		\
	   $(OS_SUPPORT)	$(SUBMODEL)	$(USE_CLOCK) $(BLUETOOTH) $(MRUBY) \
	   $(USING_DEBUG) $(USE_FONT) \

SYNTHESIS_DEFS	= $(addprefix -D,$(DEFZ)) 								\
				 -DPACK_STRUCT_END=__attribute\(\(packed\)\) 				\
				 -DALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\)			\
				 -DMPU_SUBMODEL=\"$(SUBMODEL)\"					\
				 -DAPP_VERSION=\"$(APP_VER)\"						\
				 -DHSE_VALUE=$(HSE_CLOCK)UL						\
				 -DUART_DEFAULT_NUM=$(UART_DEFAULT_NUM) 

# TARGET definition
TARGET 	    = main
TARGET_ELF  = $(TARGET).elf
TARGET_SREC = $(TARGET).s19
TARGET_HEX  = $(TARGET).hex
TARGET_BIN  = $(TARGET).bin
TARGET_LSS  = $(TARGET).lss
TARGET_SYM  = $(TARGET).sym

# define Cortex-M4 LIBRARY PATH
LIBVER	    = ../STM32F4xx_DSP_StdPeriph_Lib_V1.3.0
FWLIB  		= $(LIBVER)/Libraries/STM32F4xx_StdPeriph_Driver
USB_DEVICE 	= $(LIBVER)/Libraries/STM32_USB_Device_Library
USB_HOST 	= $(LIBVER)/Libraries/STM32_USB_HOST_Library
USB_OTG 	= $(LIBVER)/Libraries/STM32_USB_OTG_Driver
CM4LIB 		= ../STM32F4xx_DSP_StdPeriph_Lib_V1.3.0/Libraries/CMSIS
CM4_DEVICE 	= $(CM4LIB)/Device/ST/STM32F4xx
CM4_CORE	= $(CM4LIB)/Include
ifeq ($(MRUBY),USE_MRUBY)
MRUBY_DIR   = ../mruby-master
MRUBY_LIB   = $(MRUBY_DIR)/build/host/lib/libmruby.a
endif

FATFS		= ../STM32F4xxxx_FatFS_DISP_20140803
#FATFS		= ../STM32F4x7xxT6_FatFS_DISP_20130510
#FATFSLIB	= $(FATFS)/lib/STM32F4xx_StdPeriph_Driver
FATFSFF		= $(FATFS)/lib/ff

ifeq ($(USE_FONT),USE_KANJI)
LIBNKF 	= ../libnkf-1.0.0/libnkf
endif

# include PATH
ifeq ($(MRUBY),USE_MRUBY)
INCPATHS	 = 	./					\
			./inc					\
			$(FWLIB)/inc  			\
			$(USB_DEVICE)/Core/inc		\
			$(USB_DEVICE)/Class/MSC/inc		\
			$(USB_HOST)/Core/inc		\
			$(USB_OTG)/inc			\
			$(CM4_DEVICE)/Include	\
			$(MRUBY_DIR)/include	\
			$(CM4_CORE)				\
			$(LIBNKF)				\
			$(FATFSFF)
else
INCPATHS	 = 	./					\
			./inc					\
			$(FWLIB)/inc  			\
			$(USB_DEVICE)/Core/inc		\
			$(USB_DEVICE)/Class/MSC/inc		\
			$(USB_HOST)/Core/inc		\
			$(USB_OTG)/inc			\
			$(CM4_DEVICE)/Include	\
			$(CM4_CORE)				\
			$(LIBNKF)				\
			$(FATFSFF)
endif

#INCPATHS += \
#	$(FATFSLIB)

INCLUDES     = $(addprefix -I ,$(INCPATHS))

# Set library PATH
ifeq ($(MRUBY),USE_MRUBY)
LIBPATHS     = $(FWLIB) $(CM4LIB) $(FATFSFF) $(MRUBY_DIR)/build/host/lib
else
#LIBPATHS     = $(FWLIB) $(CM4LIB) $(FATFSFF)
endif
LIBRARY_DIRS = $(addprefix -L,$(LIBPATHS))
# if you use math-library, put "-lm" 
MATH_LIB	 =	-lm

# LinkerScript PATH
LINKER_PATH = $(FATFS)/lib/linker
LINKER_DIRS = $(addprefix -L,$(LINKER_PATH)) 

# Object definition
OBJS 	 = $(CFILES:%.c=%.o) 
LIBOBJS  = $(LIBCFILES:%.c=%.o) $(SFILES:%.s=%.o)

# C code PATH
SOURCE  = ./src
CFILES = \
 $(SOURCE)/$(TARGET).c 			\
 $(SOURCE)/glcd.c					\
 $(SOURCE)/editor.c					\
 $(SOURCE)/hw_config.c				\
 $(SOURCE)/rtc_support.c			\
 $(SOURCE)/uart_support.c			\
 $(SOURCE)/stm32f4xx_it.c			\
 $(SOURCE)/usb_bsp.c				\
 $(SOURCE)/usbd_desc.c				\
 $(SOURCE)/usbd_usr.c				\
 $(SOURCE)/usbd_storage_msd.c		\
 $(SOURCE)/usb_dcd_int.c 			\
 $(SOURCE)/systick.c				\
 $(FATFSFF)/ff.c 					\
 $(FATFSFF)/sdio_stm32f4.c 			\
 $(FATFSFF)/option/cc932.c

# $(FATFSFF)/diskio_sdio.c 			\


ifeq ($(USE_FONT),USE_KANJI)
CFILES += \
 $(LIBNKF)/libnkf.c					\
 $(LIBNKF)/utf8tbl.c
endif

ifeq ($(USE_BASIC),BASIC)
CFILES += \
 $(SOURCE)/ubasic.c                 \
 $(SOURCE)/tokenizer.c              
endif

#/*----- STARTUP code PATH -----*/
STARTUP_DIR = $(CM4_DEVICE)/Source/Templates/gcc_ride7
ifeq ($(OS_SUPPORT),USE_FREERTOS)
SFILES += \
	$(SOURCE)/startup_stm32f40xx_rtos.s
else
SFILES += \
	$(STARTUP_DIR)/startup_stm32f40xx.s
endif

#/*----- STM32 library PATH -----*/
LIBCFILES = 						\
 $(FWLIB)/src/misc.c 				\
 $(FWLIB)/src/stm32f4xx_gpio.c 		\
 $(FWLIB)/src/stm32f4xx_rcc.c 		\
 $(FWLIB)/src/stm32f4xx_tim.c 		\
 $(FWLIB)/src/stm32f4xx_usart.c 	\
 $(FWLIB)/src/stm32f4xx_pwr.c 		\
 $(FWLIB)/src/stm32f4xx_rtc.c 		\
 $(FWLIB)/src/stm32f4xx_spi.c 		\
 $(FWLIB)/src/stm32f4xx_dcmi.c 		\
 $(FWLIB)/src/stm32f4xx_adc.c 		\
 $(FWLIB)/src/stm32f4xx_dma.c		\
 $(FWLIB)/src/stm32f4xx_sdio.c		\
 $(FWLIB)/src/stm32f4xx_syscfg.c	\
 ./src/system_stm32f4xx.c			\
  \
 $(USB_OTG)/src/usb_dcd.c 			\
 $(USB_OTG)/src/usb_core.c 			\
 $(USB_DEVICE)/Class/msc/src/usbd_msc_core.c 	\
 $(USB_DEVICE)/Class/msc/src/usbd_msc_bot.c 	\
 $(USB_DEVICE)/Class/msc/src/usbd_msc_scsi.c 	\
 $(USB_DEVICE)/Class/msc/src/usbd_msc_data.c 	\
 $(USB_DEVICE)/Core/src/usbd_core.c 	\
 $(USB_DEVICE)/Core/src/usbd_req.c 		\
 $(USB_DEVICE)/Core/src/usbd_ioreq.c 	


#/*----- STM32 Debug library -----*/
ifeq ($(OPTIMIZE),0)
CFILES += $(FATFS)/lib/IOView/stm32f4xx_io_view.c
else
endif


# TOOLCHAIN SETTING
CC 			= $(TCHAIN)-gcc
CPP 		= $(TCHAIN)-g++
OBJCOPY 	= $(TCHAIN)-objcopy
OBJDUMP 	= $(TCHAIN)-objdump
SIZE 		= $(TCHAIN)-size
AR 			= $(TCHAIN)-ar
LD 			= $(TCHAIN)-gcc
NM 			= $(TCHAIN)-nm
REMOVE		= $(REMOVAL) -f
REMOVEDIR 	= $(REMOVAL) -rf

# C and ASM FLAGS
CFLAGS  = -MD -mcpu=cortex-m4 -march=armv7e-m -mtune=cortex-m4
#CFLAGS  = -MD -mcpu=cortex-m3
CFLAGS += -mthumb -mlittle-endian $(ALIGNED_ACCESS)
CFLAGS += -mapcs-frame -mno-sched-prolog $(USING_FPU)
CFLAGS += -std=gnu99
CFLAGS += -gdwarf-2 -O$(OPTIMIZE) $(USE_LTO)
CFLAGS += -fno-strict-aliasing -fsigned-char
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-schedule-insns2
CFLAGS += --param max-inline-insns-single=1000
CFLAGS += -fno-common -fno-hosted
CFLAGS += -Wall
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<) 
CFLAGS += $(SYNTHESIS_DEFS)  

# Linker FLAGS -mfloat-abi=softfp -msoft-float
LDFLAGS  = -mcpu=cortex-m4 -march=armv7e-m -mthumb
#LDFLAGS  = -mcpu=cortex-m3 -march=armv7e-m-mthumb
LDFLAGS += -u g_pfnVectors -Wl,-static -Wl,--gc-sections -nostartfiles
LDFLAGS += -Wl,-Map=$(TARGET).map
#MRUBY			= USE_MRUBY

ifeq ($(MRUBY),USE_MRUBY)
LDFLAGS += $(LIBRARY_DIRS) $(LINKER_DIRS) $(MATH_LIB) -lmruby
else
LDFLAGS += $(LIBRARY_DIRS) $(LINKER_DIRS) $(MATH_LIB)
endif

ifeq ($(USE_EXT_SRAM),DATA_IN_ExtSRAM)
LDFLAGS +=-T$(LINKER_PATH)/$(SUBMODEL)_EXTRAM.ld
else
LDFLAGS +=-T$(LINKER_PATH)/STM32F407ZGT6.ld
#LDFLAGS +=-T$(LINKER_PATH)/$(SUBMODEL).ld
endif

# Object Copy and dfu generation FLAGS
OBJCPFLAGS = -O
OBJDUMPFLAGS = -h -S -C
DFU	  = hex2dfu
DFLAGS = -w


# Build Object
all: fontdata.h gccversion build sizeafter


# make test
test: ./src/test_glcd.c
	gcc $(INCLUDES) -I /usr/local/include/CUnit -L /usr/local/lib -DTEST -D$(SUBMODEL) -D$(MPU_DENSITY) -o test_glcd.exe ./src/test_glcd.c ./src/editor.c /usr/local/lib/libcunit.a

# make basictest
basictest: ./src/test_basic.c
	gcc $(INCLUDES) -I /usr/local/include/CUnit -L /usr/local/lib -DTEST -D$(SUBMODEL) -D$(MPU_DENSITY) -o test_basic.exe ./src/test_basic.c ./src/ubasic.c ./src/tokenizer.c /usr/local/lib/libcunit.a

# Object Size Infomations
ELFSIZE = $(SIZE) -A -x $(TARGET).elf
sizeafter:
	@$(MSGECHO) 
	@$(MSGECHO) Size After:
	$(SIZE) $(TARGET).elf
	@$(SIZE) -A -x $(TARGET).elf
	
# Display compiler version information.
gccversion : 
	@$(CC) --version
	@$(MSGECHO) "BUILD_TYPE = "$(OS_SUPPORT)
	@$(MSGECHO) 

build: $(TARGET_ELF) $(TARGET_LSS) $(TARGET_SYM) $(TARGET_HEX) $(TARGET_SREC) $(TARGET_BIN)

ifeq ($(MRUBY),USE_MRUBY)
$(MRUBY_LIB): $(LIBOBJS)
	cd ../mruby-master/; ./minirake
endif

.SUFFIXES: .o .c .s   

$(TARGET_LSS): $(TARGET_ELF)
	@$(MSGECHO)
	@$(MSGECHO) Disassemble: $@
	$(OBJDUMP) $(OBJDUMPFLAGS) $< > $@ 
$(TARGET_SYM): $(TARGET_ELF)
	@$(MSGECHO)
	@$(MSGECHO) Symbol: $@
	$(NM) -n $< > $@
$(TARGET).hex: $(TARGET).elf
	@$(MSGECHO)
	@$(MSGECHO) Objcopy: $@
	$(OBJCOPY) $(OBJCPFLAGS) ihex $^ $@    
$(TARGET).s19: $(TARGET).elf
	@$(MSGECHO)
	@$(MSGECHO) Objcopy: $@
	$(OBJCOPY) $(OBJCPFLAGS) srec $^ $@ 
$(TARGET).bin: $(TARGET).elf
	@$(MSGECHO)
	@$(MSGECHO) Objcopy: $@
	$(OBJCOPY) $(OBJCPFLAGS) binary $< $@ 
$(TARGET).dfu: $(TARGET).hex
	@$(MSGECHO)
	@$(MSGECHO) Make STM32 dfu: $@
	$(DFU) $(DFLAGS) $< $@
	@$(MSGECHO)
ifeq ($(MRUBY),USE_MRUBY)
$(TARGET).elf: $(OBJS) stm32.a $(MRUBY_LIB)
	@$(MSGECHO) Link: $@
	$(LD) $(CFLAGS) $(LDFLAGS) $^ -o $@
	@$(MSGECHO)
else
$(TARGET).elf: $(OBJS) stm32.a
	@$(MSGECHO) Link: $@
	$(LD) $(CFLAGS) $(LDFLAGS) $^ -o $@
	@$(MSGECHO)
endif
stm32.a: $(LIBOBJS)
	@$(MSGECHO) Archive: $@
	$(AR) cr $@ $(LIBOBJS)    
	@$(MSGECHO)
.c.o:
	@$(MSGECHO) Compile: $<
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@
	@$(MSGECHO)
.s.o:
	@$(MSGECHO) Assemble: $<
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@
	@$(MSGECHO)
fontdata.h:	mkfont.rb
	./mkfont.rb $(USE_FONT)

# Drop files into dust-shoot
clean:
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).bin
	$(REMOVE) $(TARGET).obj
	$(REMOVE) stm32.a
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).s19
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(TARGET).dfu
	$(REMOVE) $(wildcard *.stackdump)
	$(REMOVE) $(OBJS)
	$(REMOVE) $(AOBJ)
	$(REMOVE) $(LIBOBJS)
	$(REMOVE) $(LST)
	$(REMOVE) $(CFILES:.c=.lst)
	$(REMOVE) $(CFILES:.c=.d)
	$(REMOVE) $(LIBCFILES:.c=.lst)
	$(REMOVE) $(LIBCFILES:.c=.d)
	$(REMOVE) $(SFILES:.s=.lst)
	$(REMOVE) $(wildcard $(CM4_DEVICE)/*.d)
	$(REMOVE) $(wildcard $(CM4_DEVICE)/*.lst)
	$(REMOVE) fontdata.h
	$(REMOVE) log
	$(REMOVEDIR) .dep
	@$(MSGECHO)

