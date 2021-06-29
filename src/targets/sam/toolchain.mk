#
#
#	ARM Cortex-M Toolchain
#
#

SHELL = sh
ifdef CORTEX_TOOLCHAIN_BIN
# If a toolchain path is provided, use it
CC = $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-gcc
LD = $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-gcc
AS = $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-as
OBJCOPY = $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-objcopy
OBJDUMP = $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-objdump
SIZE = $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-size
NM = $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-nm
else
# If no toolchain path was provided, try to find a toolchain on the PATH
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
NM = arm-none-eabi-nm
endif
REMOVE = rm -f
COPY = cp
