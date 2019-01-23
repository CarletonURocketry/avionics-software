#----------------------------------------------------------------------------
# SAMD21 Makefile
# By Samuel Dewan.
#
# Adapted from the WinAVR Makefile Template written by Eric B. Weddington, Jörg Wunsch, et al.
#
# Released to the Public Domain
#
#----------------------------------------------------------------------------
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make program = Download the hex file to the device, using openocd and GDB.
#
# make debug = Start openocd with a gdb debuging frontend
#
# make filename.s = Just compile filename.c into the assembler code only.
#
# make filename.i = Create a preprocessed source file for use in submitting
#                   bug reports to the GCC project.
#
# To rebuild project do "make clean" then "make all".
#----------------------------------------------------------------------------

# MCU name
PTYPE = __SAMD21J18A__
MCU_NAME = samd21

# Processor frequency.
#     This will define a symbol, F_CPU, in all source code files equal to the 
#     processor frequency. You can then use this symbol in your source code to 
#     calculate timings. Do NOT tack on a 'UL' at the end, this will be done
#     automatically to create a 32-bit value in your source code.
F_CPU = 48000000

OPENOCD_CONFIG = openocd.cfg

# Target file name (without extension).
TARGET = main

# List C source files here. (C dependencies are automatically generated.)
SRC = $(wildcard *.c) $(MCU_NAME)/startup_$(MCU_NAME).c
SRC += $(wildcard usb/*.c) $(wildcard usb/samd/*.c)

OBJDIR = obj
# List Assembler source files here.
#     Make sure they always end in a capital .S.  Files ending in a lowercase .s
#     will not be considered source files but generated files (assembler
#     output from the compiler), and will be deleted upon "make clean"!
#     Even though the DOS/Win* filesystem matches both .s and .S the same,
#     it will preserve the spelling of the filenames, and gcc itself does
#     care about how the name is spelled on its command-line.
ASRC = $(wildcard *.S)


# Optimization level, can be [0, 1, 2, 3, s]. 
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level.)
OPT = 2


# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRAINCDIRS = $(MCU_NAME)/include $(MCU_NAME)/source
EXTRAINCDIRS += usb

# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensions
CSTANDARD = -std=gnu99


# Place -D or -U options here
CDEFS = -DF_CPU=$(F_CPU)UL


# Place -I options here
CINCS = 

#---------------- Architecture Options ----------------
ARCHFLAGS += -mcpu=cortex-m0plus -mthumb 

#---------------- Compiler Options ----------------
#  -g*: 			generate debugging information
#  -O*:          optimization level
#  -f...:        	tuning, see GCC manual
#  -Wall...:    warning level
#  -Wa,...:     tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT) -g3
CFLAGS += $(ARCHFLAGS)
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -fno-strict-aliasing -ffunction-sections -fdata-sections -mlong-calls
CFLAGS += --param max-inline-insns-single=500
CFLAGS += -gstrict-dwarf

CFLAGS += -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror-implicit-function-declaration -Wpointer-arith
CFLAGS += -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int -Wmain -Wparentheses -Wsequence-point -Wreturn-type
CFLAGS += -Wswitch -Wtrigraphs -Wunused -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wbad-function-cast
CFLAGS += -Wwrite-strings -Wsign-compare -Waggregate-return -Wmissing-declarations -Wformat -Wmissing-format-attribute
CFLAGS += -Wno-deprecated-declarations -Wpacked -Wredundant-decls -Wnested-externs -Wlong-long -Wunreachable-code -Wcast-align

CFLAGS += -Wa,-adhlns=$(addprefix $(OBJDIR)/,$(<:.c=.lst))
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)

#---------------- Assembler Options ----------------
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
#  -listing-cont-lines: Sets the maximum number of continuation lines of hex 
#       dump that will be displayed for a given single line of source input.
ASFLAGS = -Wa,-adhlns=$(addprefix $(OBJDIR)/,$(<:.S=.lst)),-gstabs,--listing-cont-lines=100


#---------------- Library Options ----------------
MATH_LIB = -Lsamd21/lib/libarm_cortexM0l_math.a


#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDSCRIPT = samd21/samd21j18a_flash.ld
SPECS = samd21/nosys.specs

LDFLAGS += -T$(LDSCRIPT) $(ARCHFLAGS) -Wl,--gc-sections --entry=Reset_Handler
LDFLAGS += --specs=$(SPECS) $(MATH_LIB)


#---------------- Programming Options (openocd/dgb) ----------------



#---------------- Debugging Options ----------------

# Debugging port used to communicate between GDB / openocd.
OPENOCD_PORT = 4444
GDB_PORT = 3333
#GDB_PORT = 2331

# Debugging host used to communicate between GDB / openocd, normally
#     just set to localhost unless doing some sort of crazy debugging when 
#     openocd is running on a different computer.
DEBUG_HOST = localhost



#============================================================================


# Define programs and commands.

# CORTEX_TOOLCHAIN_BIN must be specified as an environment variable or argument
# to make

SHELL ?= sh
CC ?= $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-gcc
LD ?= $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-gcc
AS ?= $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-as
OBJCOPY ?= $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-objcopy
OBJDUMP ?= $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-objdump
SIZE ?= $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-size
NM ?= $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-nm
GDB ?= $(CORTEX_TOOLCHAIN_BIN)/arm-none-eabi-gdb
OPENOCD ?= /usr/local/bin/openocd
TELNET ?= /usr/bin/nc
REMOVE ?= rm -f
COPY ?= cp


# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:
MSG_PROGRAMMING = Uploading to Target:
MSG_RESET = Resetting Target:
MSG_DEBUGGING = Starting Debugger:



# Define all object files.
#OBJ = $(addprefix $(OBJDIR)/,$(SRC:.c=.o)) $(addprefix $(OBJDIR)/,$(ASRC:.S=.o))
OBJ = $(addprefix $(OBJDIR)/,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SRC)))) $(addprefix $(OBJDIR)/,$(ASRC:.S=.o))


# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -D$(PTYPE) -I. $(CFLAGS)
ALL_ASFLAGS = -D$(PTYPE) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: begin gccversion clean build program end

build: clean $(OBJDIR) elf

elf: $(OBJDIR)/$(TARGET).elf

$(OBJDIR):
	@mkdir -p $@

# Display compiler version information.
gccversion : 
	@$(CC) --version

# Display information about build.
info:
	@echo CFLAGS=$(CFLAGS)
	@echo OBJS=$(OBJS)


# Program the device.  
program: | upload reset

# Upload to target withh GDB
upload: $(OBJDIR)/$(TARGET).elf
	@echo
	@echo $(MSG_PROGRAMMING)
	echo load | $(GDB) -q -iex "target extended-remote $(DEBUG_HOST):$(GDB_PORT)" $(OBJDIR)/$(TARGET).elf

# Reset the device.
reset:
	@echo
	@echo $(MSG_RESET)
	(echo reset run; sleep 0.5) | $(TELNET) $(DEBUG_HOST) $(OPENOCD_PORT)
	@echo

# Launch a debugging session.
debug: $(OBJDIR)/$(TARGET).elf
	@echo
	@echo $(MSG_DEBUGGING)
	$(GDB) -iex "target extended-remote $(DEBUG_HOST):$(GDB_PORT)" $(OBJDIR)/$(TARGET).elf


# Link: create ELF output file from object files.
.SECONDARY : $(OBJDIR)/$(TARGET).elf
.PRECIOUS : $(OBJ)
$(OBJDIR)/%.elf: $(OBJ)
	@echo
	@echo $(MSG_LINKING) $@
	$(LD) $^ --output $@ $(LDFLAGS)


# Compile: create object files from C source files.
$(OBJDIR)/%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) "$(abspath $<)" -o $@
	$(CC) -MM $(ALL_CFLAGS) $< > $(OBJDIR)/$*.d


# Compile: create assembler files from C source files.
$(OBJDIR)/%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
$(OBJDIR)/%.o:    %.s
	$(AS) $< -o $@


# Compile: create object files from C++ source files.
$(OBJDIR)/%.o : %.cpp
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) "$(abspath $<)" -o $@


# Compile: create assembler files from C++ source files.
$(OBJDIR)/%.s : %.cpp
	$(CC) -S $(ALL_CFLAGS) $< -o $@

# Create preprocessed source for use in sending a bug report.
$(OBJDIR)/%.i : %.cpp
	$(CC) -E -mmcu=$(MCU) -I. $(CFLAGS) $< -o $@

# Target: clean project.
clean: begin clean_list end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(OBJDIR)/$(TARGET).elf
	$(REMOVE) $(OBJDIR)/$(TARGET).map
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
#$(REMOVE) $(OBJDIR)/$(SRC:.c=.s)
	$(REMOVE) $(OBJDIR)/$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SRC)))
	$(REMOVE) $(OBJDIR)/$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SRC)))


# Listing of phony targets.
.PHONY : all begin finish end gccversion \
build elf clean clean_list program debug upload reset
