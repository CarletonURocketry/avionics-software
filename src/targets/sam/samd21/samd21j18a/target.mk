#
#
# SAMD21 Target Makefile
#
#

### Part Details ###
TARGET_PART = samd21j18a

##### Source Files #####
TARGET_SOURCES = $(wildcard $(TARGETSDIR)/$(TARGET)/../target.c)
# samd21j18a source files
TARGET_SOURCES += $(wildcard $(TARGETSDIR)/$(TARGET)/src/*.c)
# samd21 source files
TARGET_SOURCES += $(wildcard $(TARGETSDIR)/$(TARGET)/../src/*.c)
# sam source files
TARGET_SOURCES += $(wildcard $(TARGETSDIR)/$(TARGET)/../../src/*.c)
TARGET_SOURCES += $(TARGETSDIR)/$(TARGET)/../cmsis/startup_samd21.c

##### Include Directories #####
TARGET_INC_DIRS = $(TARGETSDIR)/$(TARGET)/../
TARGET_INC_DIRS += $(TARGETSDIR)/$(TARGET)/../src
TARGET_INC_DIRS += $(TARGETSDIR)/$(TARGET)/../../src
TARGET_INC_DIRS += $(TARGETSDIR)/$(TARGET)/../cmsis/include
TARGET_INC_DIRS += $(TARGETSDIR)/$(TARGET)/../cmsis/source
TARGET_INC_DIRS += src/targets/sam/cmsis_core/include

##### Architecture Options #####
ARCH_FLAGS = -mcpu=cortex-m0plus -mthumb

##### Compiler Flags #####
# Preprocessor definitions
TARGET_CFLAGS = -D__SAMD21J18A__ -DF_CPU=48000000UL -DSAMD2x
# Architecture options
TARGET_CFLAGS += $(ARCH_FLAGS)

##### Linker Flags #####
# Linker script
TARGET_LDFLAGS = -T$(TARGETSDIR)/$(TARGET)/../cmsis/$(TARGET_PART)_flash.ld
# Architecture options
TARGET_LDFLAGS += $(ARCH_FLAGS)
# Entry point
TARGET_LDFLAGS += --entry=Reset_Handler
# Specs
TARGET_LDFLAGS += --specs=$(TARGETSDIR)/$(TARGET)/../cmsis/nosys.specs
# Math library
TARGET_LDFLAGS += -L$(TARGETSDIR)/$(TARGET)/../cmsis/lib/libarm_cortexM0l_math.a -lm

### Toolchain ###
include src/targets/sam/toolchain.mk

##### Debugging Options #####
DEBUG_CHIP_NAME = at91samd21j18
DEBUG_CHIP_CONFIG = target/at91samdXX.cfg
