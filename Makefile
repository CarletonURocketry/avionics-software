##### Configuration #####
# Output file name (without extension).
OUTPUT = main
# Directory that contains source files
SRCDIR = src
# Directory that contains target definitions
TARGETSDIR = $(SRCDIR)/targets
# Directory that contains board definitions
BOARDSDIR = $(SRCDIR)/boards
# Directory that contains variant definitions
VARIANTSDIR = $(SRCDIR)/variants
# Directory where products should be placed
OBJDIR = obj
# Include board makefile
BOARD ?= mcu/rev_a
include $(BOARDSDIR)/$(BOARD)/board.mk
# Include target make file (TARGET should be defined by board makefile)
include $(TARGETSDIR)/$(TARGET)/target.mk
# Include varient makefile
VARIANT ?= test
include $(VARIANTSDIR)/$(VARIANT)/variant.mk

##### Files #####
# List of C source files. (C dependencies are automatically generated.)
SRC = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.c)
SRC += $(BOARD_SOURCES) $(TARGET_SOURCES) $(VARIANT_SOURCES)
# List Assembler source files here (Make sure they always end in a capital .S.)
ASRC = $(wildcard $(SRCDIR)/*.S) $(BOARD_ASRC) $(TARGET_ASRC)
# List any extra directories to look for include files here.
INCLUDE_DIRS = . $(SRCDIR) $(BOARD_INC_DIRS) $(TARGET_INC_DIRS) $(VARIANT_INC_DIRS)
# Define all object files.
OBJ = $(patsubst $(SRCDIR)/%.S,$(OBJDIR)/%.o,$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))))
# Define all dependancy files.
DEP = $(OBJ:%.o=%.d)
# Define all list files.
LST = $(OBJ:%.o=%.lst)

### Compiler Options ###
CFLAGS += $(TARGET_CFLAGS) $(BOARD_CFLAGS)
# Optimization level, can be [0, 1, 2, 3, s].
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level.)
CFLAGS += -O2 -g3 -gstrict-dwarf -std=gnu11
CFLAGS += -funsigned-char -funsigned-bitfields -fno-strict-aliasing
CFLAGS += -ffunction-sections -fdata-sections -mlong-calls
CFLAGS += --param max-inline-insns-single=500

# Enable many usefull warnings
# (see https://gcc.gnu.org/onlinedocs/gcc-6.3.0/gcc/Warning-Options.html)
CFLAGS += -Wall -Wextra -Wshadow -Wundef -Wformat=2 -Wtrampolines -Wfloat-equal
CFLAGS += -Wbad-function-cast -Waggregate-return -Wstrict-prototypes -Wpacked
CFLAGS += -Wno-aggressive-loop-optimizations -Wmissing-prototypes -Winit-self
CFLAGS += -Wmissing-declarations -Wmissing-format-attribute -Wunreachable-code
CFLAGS += -Wshift-overflow=2 -Wduplicated-cond -Wpointer-arith -Wwrite-strings
CFLAGS += -Wnested-externs -Wcast-align -Wredundant-decls -Wlong-long
CFLAGS += -Werror=implicit-function-declaration -Wlogical-not-parentheses
CFLAGS += -Wlogical-op -Wold-style-definition -Wcast-qual -Wdouble-promotion
CFLAGS += -Wunsuffixed-float-constants -Wmissing-include-dirs -Wnormalized
CLFAGS += -Wdisabled-optimization -Wsuggest-attribute=const

# These warning may be usefull in some cases, but cause too many false positives
# to be enabled all of the time: -Winline -Wpadded -Wvla -Wpedantic -Wconversion
# -Wnull-dereference -Wsuggest-attribute=noreturn -Wsuggest-attribute=pure
# -Wstack-usage=256

# Disable some anoying warnings
CFLAGS += -Wno-unused-parameter

CFLAGS += -Wa,-adhlns=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.lst,$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.lst,$<))
CFLAGS += $(patsubst %,-I%,$(INCLUDE_DIRS))
CFLAGS += $(CSTANDARD)


##### Assembler Options #####
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
#  -listing-cont-lines: Sets the maximum number of continuation lines of hex 
#       dump that will be displayed for a given single line of source input.
ASFLAGS = -x assembler-with-cpp -Wa,-adhlns=$(patsubst $(SRCDIR)/%.S,$(OBJDIR)/%.lst,$<),-gstabs,--listing-cont-lines=100


##### Linker Options #####
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,--gc-sections $(TARGET_LDFLAGS)


##### Debugging Options #####

# Script used to launch OpenOCD and GDB
DEBUG_CMD = PYTHONDONTWRITEBYTECODE=1 ./debug.py

# Debugging port used to communicate between GDB / openocd.
OPENOCD_PORT = 4444
GDB_PORT = 2331

# Debugging host used to communicate between GDB / openocd, normally
#     just set to localhost unless doing some sort of crazy debugging when 
#     openocd is running on a different computer.
DEBUG_HOST = localhost

DEBUG_ARGS = --host $(DEBUG_HOST) --gdb-port $(GDB_PORT)
DEBUG_ARGS += --openocd-port $(OPENOCD_PORT)

# Use user provided OpenOCD if specified
ifdef OPENOCD_PATH
DEBUG_ARGS += --openocd "$(OPENOCD_PATH)"
endif

# Use GDB from toolchain if one is specified
ifdef CORTEX_TOOLCHAIN_BIN
DEBUG_ARGS += --gdb "$(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-gdb"
endif

# Select OpenOCD interface
ifdef OPENOCD_INTERFACE
DEBUG_ARGS += --interface "$(OPENOCD_INTERFACE)"
endif

# If we are in WSL we need to let debug.py know becuase Windows is annoying
ifdef WSL
DEBUG_ARGS += --wsl
endif

# Pass debugging options from board.mk to debug script
DEBUG_ARGS += --chip-name "$(DEBUG_CHIP_NAME)"
DEBUG_ARGS += --chip-config "$(DEBUG_CHIP_CONFIG)"

# Use a device file as the GDB server if one is proved, useful for Black Magic
# Probe. Note that the debug script will not attach to the target automatically,
# the user needs to perform the scan and attach manually after GDB opens.
ifdef GDB_FILE
DEBUG_ARGS += --gdb-file "$(GDB_FILE)"
endif

################################################################################

# Define compilers
COMPILE.c = $(CC) $(CFLAGS) -c
COMPILE.cpp = $(CC) $(CFLAGS) -c

# Default target.
build : $(OBJDIR) elf

all : gccversion clean build program

elf : $(OBJDIR)/$(OUTPUT).elf

hex : $(OBJDIR)/$(OUTPUT).hex

$(OBJDIR) :
	@mkdir -p $@

# Display compiler version information.
gccversion : 
	@$(CC) --version

# Display information about build.
info :
	@echo CFLAGS=$(CFLAGS)
	@echo SRC=$(SRC)
	@echo OBJ=$(OBJ)

# Program the device.  
program : $(OBJDIR)/$(OUTPUT).elf
	@echo
	@echo Programming Target
	$(DEBUG_CMD) $(DEBUG_ARGS) --upload --reset --no-debug $^

# Upload binary to target.
upload : $(OBJDIR)/$(OUTPUT).elf
	@echo
	@echo Programming Target
	$(DEBUG_CMD) $(DEBUG_ARGS) --upload --no-debug $^

# Reset the device.
reset :
	@echo
	@echo Resetting Target
	$(DEBUG_CMD) $(DEBUG_ARGS) --reset --no-debug
	
# Reset the device and leave halted.
halt :
	@echo
	@echo Resetting Target
	$(DEBUG_CMD) $(DEBUG_ARGS) --reset halt --no-debug

# Launch a debugging session.
debug : $(OBJDIR)/$(OUTPUT).elf
	$(DEBUG_CMD) $(DEBUG_ARGS) $^

# Link: create ELF output file from object files.
.SECONDARY : $(OBJDIR)/$(OUTPUT).elf
.PRECIOUS : $(OBJ)
$(OBJDIR)/%.elf : $(OBJ)
	$(shell mkdir -p $(@D) >/dev/null)
	@echo
	@echo Linking: $@
	$(LD) $^ --output $@ $(LDFLAGS)

# Create a hex file from the elf
%.hex : %.elf
	$(OBJCOPY) -O ihex $< $@

# Compile: create object files from C source files.
$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(shell mkdir -p $(@D) >/dev/null)
	@echo
	@echo Compiling: $<
	$(COMPILE.c) "$(abspath $<)" -o $@

# Compile: create object files from C++ source files.
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(shell mkdir -p $(@D) >/dev/null)
	@echo
	@echo Compiling: $<
	$(COMPILE.cpp) "$(abspath $<)" -o $@

# Assemble: create object files from assembler source files.
$(OBJDIR)/%.o : $(SRCDIR)/%.s
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/.depend : $(SRC) | $(OBJDIR)
	$(COMPILE.c) -MM $(SRC)  | \
	sed -E 's#^(.*\.o: *)$(SRCDIR)/(.*/)?(.*\.(c|cpp|S))#$(OBJDIR)/\2\1$(SRCDIR)/\2\3#' > $@

# Create an OpenOCD configuration
openocd.cfg:
	$(DEBUG_CMD) $(DEBUG_ARGS) --make-openocd-config=$@

include $(OBJDIR)/.depend

# Target: clean project.
clean :
	@echo $(MSG_CLEANING)
	$(REMOVE) -rf $(OBJDIR)/*

# Listing of phony targets.
.PHONY : all gccversion build elf hex clean program debug upload reset openocd.cfg
