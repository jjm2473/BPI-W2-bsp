

# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

# ********************************************************************
# u-boot's makefile will pass the PRJ setting to here
# ********************************************************************
#PRJ = 6227_c0000_b0000
#PRJ = 6227_c0000_b0000_fpga_emmc_DCAS_secure_enc
#PRJ = 6227_c0000_b0000_fpga_emmc
#PRJ = 6227_c0000_b0000_fpga_emmc_tv005_l4300
#PRJ = 6227_c0000_b0000_fpga_emmc_RTK_secure_enc
#PRJ = 6227_c0000_b0000_fpga_nand_DCAS_secure_enc
#PRJ = 6227_c0000_b0000_fpga_nand
#PRJ = 6227_c0000_b0000_fpga_nand_random
#PRJ = 6227_c0000_b0000_fpga_nand_RTK_secure_enc
#PRJ = 6227_c0000_b0000_fpga_spi_DCAS_secure_enc
#PRJ = 6227_c0000_b0000_fpga_spi
#PRJ = 6227_c0000_b0000_fpga_spi_RTK_secure_enc
# load to fsbl addr
#LOAD_ADDR = 0x10100000
LOAD_ADDR = 0x01500000

# ********************************************************************
# Detect compiling environment.
# ********************************************************************
UNAME = $(shell uname)
LINUX_ENV = "LINUX"
CYGWIN_ENV = "CYGWIN"
ENV = $(CYGWIN_ENV)	# default

ifneq "$(filter Linux%,$(UNAME))" ""
	ENV = $(LINUX_ENV)
endif

ifneq "$(filter CYGWIN%,$(UNAME))" ""
	ENV = $(CYGWIN_ENV)
endif

# ********************************************************************
# Directories specifics.
# ********************************************************************
PERL    = perl
RM      = rm -rf
CD      = cd
CAT     = cat
CP      = cp
SED     = sed

TOP_DIR              = $(shell cd .; pwd)
ROOT_DIR	     = ../../include
ASM_DIR	    	 = ../../arch/arm/include
INCLUDE_DIR          = $(TOP_DIR)/include
OBJ_DIR 			= .build

#-------------------------------------------------------------------------------
TARGET_EXE             = $(TOP_DIR)/usbflash.exe

include $(TOP_DIR)/config.mk

ELF-$(ARCH)  :=
ELF-$(BOARD) :=
ELF-$(CPU)   :=
ELF-y        := usbflash.exe

#ELF-$(CONFIG_SMC91111)        += smc91111_eeprom
#ELF-$(CONFIG_SMC911X)         += smc911x_eeprom
#ELF-$(CONFIG_SPI_FLASH_ATMEL) += atmel_df_pow2
#ELF-i386                      += 82559_eeprom
#ELF-mpc5xxx                   += interrupt
#ELF-mpc8xx                    += test_burst timer
#ELF-mpc8260                   += mem_to_mem_idma2intr
#ELF-ppc                       += sched
#ELF-oxc                       += eepro100_eeprom

#
# Some versions of make do not handle trailing white spaces properly;
# leading to build failures. The problem was found with GNU Make 3.80.
# Using 'strip' as a workaround for the problem.
#
ELF := $(strip $(ELF-y) $(ELF-$(ARCH)) $(ELF-$(BOARD)) $(ELF-$(CPU)))

SREC := $(addsuffix .srec,$(ELF))
BIN  := $(addsuffix .bin,$(ELF))
DIS  := $(addsuffix .dis,$(ELF))

SOBJS	:= start64.o cache64.o
COBJS	:= usbflash.o crt.o cache.o util.o
COBJS 	+= string.o ctype.o vsprintf.o div64.o
COBJS 	+= crc32.o uuid.o
include uart/module.mk
include printf/module.mk
include emmc/module.mk
include mem/module.mk
include timer/module.mk
include gpio/module.mk
include usb/module.mk
include disk/module.mk
include fs/module.mk
#LIB	= $(obj)libstubs.o
#LIB	= $(obj)

LIBAOBJS-$(ARCH)     := 
LIBAOBJS-$(CPU)      :=
LIBAOBJS := $(LIBAOBJS-$(ARCH)) $(LIBAOBJS-$(CPU))

#LIBCOBJS = stubs.o 
LIBCOBJS = 

LIBOBJS	= $(addprefix $(obj),$(LIBAOBJS) $(LIBCOBJS))

SRCS	:= $(COBJS:.o=.c) $(LIBCOBJS:.o=.c) $(LIBAOBJS:.o=.S) $(SOBJS:.o=.S)
OBJS	:= $(addprefix $(OBJ_DIR)/,$(COBJS))
SSOBJS	:= $(addprefix $(OBJ_DIR)/,$(SOBJS))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

CPPFLAGS += -I$(INCLUDE_DIR)
INCDIRS = -I$(INCLUDE_DIR) -I$(ROOT_DIR) -I$(ASM_DIR)

# For PowerPC there's no need to compile standalone applications as a
# relocatable executable.  The relocation data is not needed, and
# also causes the entry point of the standalone application to be
# inconsistent.
ifeq ($(ARCH),powerpc)
AFLAGS := $(filter-out $(RELFLAGS),$(AFLAGS))
CFLAGS := $(filter-out $(RELFLAGS),$(CFLAGS))
CPPFLAGS := $(filter-out $(RELFLAGS),$(CPPFLAGS))
endif

# We don't want gcc reordering functions if possible.  This ensures that an
# application's entry point will be the first function in the application's
# source file.
#CFLAGS_NTR := $(call cc-option,-fno-toplevel-reorder)
#CFLAGS += $(CFLAGS_NTR) -I$(INCLUDE_DIR)
#CFLAGS = -g -Os -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -isystem $(gcclibdir)/../include -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv7-a  -Wall -Wstrict-prototypes -fno-stack-protector -Wno-format-nonliteral -Wno-format-security  -fno-toplevel-reorder 
CFLAGS = -g -Os -fno-common -ffixed-r8 -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -isystem $(gcclibdir)/include -include ../../include/linux/kconfig.h -include debug.h -pipe  -DCONFIG_ARM -DCONFIG_PANIC_HANG -D__ARM__  -march=armv8-a -D__EL__ -D__ARM_ARCH_8A__
#CFLAGS += -msoft-float -marm -mabi=aapcs-linux -mno-thumb-interwork
CFLAGS += -DDO_DEPS_ONLY

CFLAGS += $(INCDIRS)
all: $(ELF) $(SREC) $(BIN)

clean:
	-rm -rf $(OBJ_DIR)
	-rm -f $(TARGET_EXE) *.o *.reverse *.bin *.map *.dis *.srec
	echo $(TOP_DIR)
#########################################################################
$(LIB):	$(obj).depend $(LIBOBJS)
	$(call cmd_link_o_target, $(LIBOBJS))

#$(ELF):
#$(obj)%:	$(obj)%.o $(LIB) 
#		$(LD) $(LDFLAGS) -g -Ttext $(CONFIG_STANDALONE_LOAD_ADDR) \
#			-o $@ -e $(SYM_PREFIX)$(notdir $(<:.o=)) $< $(LIB) \
#			-L$(gcclibdir) -lgcc

$(ELF): 	$(LIB) $(SSOBJS) $(OBJS) Makefile
		$(LD) $(LDFLAGS) -g -z common-page-size=0x400 -z max-page-size=0x400 -Ttext $(LOAD_ADDR) \
			-o $@ -e start $(SSOBJS) $(OBJS) $(LIB) \
			-L$(gcclibdir) -lgcc -Map $@.map

$(SREC):
$(obj)%.srec:	$(obj)%
		$(OBJCOPY) -O srec $< $@

$(BIN):
$(obj)%.bin:	$(obj)%
		$(OBJCOPY) -O binary $< $@
		$(OBJDUMP) -DS $<  >$(DIS)

#########################################################################


# defines $(obj).depend target
#include $(SRCTREE)/rules.mk

#sinclude $(obj).depend

ASM=$(CC)
$(OBJ_DIR)/%.o: %.c
	mkdir -p `dirname $@`
	$(CC) -Wp,-MD,$@.d $(CFLAGS) $(DFLAGS) $(IFLAGS) -c  $< -o $@

$(OBJ_DIR)/%.o: %.S
	mkdir -p `dirname $@`
	$(ASM) $(ASFLAGS) $(DFLAGS) $(IFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s
	mkdir -p `dirname $@`
	$(ASM) $(ASFLAGS) $(DFLAGS) $(IFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $(OBJ_DIR)
