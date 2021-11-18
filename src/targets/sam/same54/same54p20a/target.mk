#
#
# SAME54 Target Makefile
#
#

### Part Details ###
TARGET_PART = same54p20a

##### Source Files #####
TARGET_SOURCES = $(wildcard $(TARGETSDIR)/$(TARGET)/../target.c)
# same54p20a source files
TARGET_SOURCES += $(wildcard $(TARGETSDIR)/$(TARGET)/src/*.c)
# same54 source files
TARGET_SOURCES += $(wildcard $(TARGETSDIR)/$(TARGET)/../src/*.c)
# sam source files
TARGET_SOURCES += $(wildcard $(TARGETSDIR)/$(TARGET)/../../src/*.c)
TARGET_SOURCES += $(TARGETSDIR)/$(TARGET)/../cmsis/startup_same54.c

##### Include Directories #####
TARGET_INC_DIRS = $(TARGETSDIR)/$(TARGET)/../
TARGET_INC_DIRS += $(TARGETSDIR)/$(TARGET)/../src
TARGET_INC_DIRS += $(TARGETSDIR)/$(TARGET)/../../src
TARGET_INC_DIRS += $(TARGETSDIR)/$(TARGET)/../cmsis/include
TARGET_INC_DIRS += $(TARGETSDIR)/$(TARGET)/../cmsis/source
TARGET_INC_DIRS += src/targets/sam/cmsis_core/include

##### Architecture Options #####
ARCH_FLAGS = -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16

##### Compiler Flags #####
# Preprocessor definitions
TARGET_CFLAGS = -D__SAME54P20A__ -DF_CPU=120000000UL -DSAMx5x
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
TARGET_LDFLAGS += -L$(TARGETSDIR)/$(TARGET)/../cmsis/lib/libarm_cortexM4lf_math.a -lm

### Toolchain ###
include src/targets/sam/toolchain.mk

##### Debugging Options #####
DEBUG_CHIP_NAME = atsame54p20a
DEBUG_CHIP_CONFIG = target/atsame5x.cfg
