BINDIR=./bin

TEST_SOURCES = $(patsubst %, %.c, $(TESTS))
TEST_OBJECTS = $(patsubst %, %.o, $(TESTS))
TEST_BINARIES = $(patsubst %, ${BINDIR}/%, $(TESTS))
TEST_GCNO_FILES = $(patsubst %, %.gcno, $(TESTS))
TEST_GCDA_FILES = $(patsubst %, %.gcda, $(TESTS))
TEST_PREPROCS = $(patsubst %, %.i, $(TESTS))


# Find a debugger
ifeq (, $(shell which gdb))
ifeq (, $(shell which lldb))
# No debugger found
else
DEBUGGER = lldb
endif
else
DEBUGGER = gdb
endif

# Include directories
CFLAGS += -I"${SRCDIR}" -I"$(SRCDIR)/targets/sam/samd21/cmsis/include"
CFLAGS += -I"$(SRCDIR)/targets/sam/samd21/cmsis/source"
CFLAGS += -I"${SRCDIR}/targets/sam/cmsis_core/include"
CFLAGS += -I"${SRCDIR}/targets/sam/samd21"
CFLAGS += -I"${SRCDIR}/targets/sam/src"
CFLAGS += -I"${SRCDIR}/boards/mcu/rev_b"
CFLAGS += -I"${SRCDIR}/variants/test"
CFLAGS += -I"${SRCDIR}/../unittests"

# Required macros
CFLAGS += -D__SAMD21J18A__ -DF_CPU=48000000UL -DSAMD2x

# Source file definitions
CFLAGS += -DSOURCE_C="\"$(SOURCE).c\"" -DSOURCE_H="\"$(SOURCE).h\""

# Include debuging information
CFLAGS += -O0 -g3

# Other CFLAGS
CFLAGS += -funsigned-char -fno-strict-aliasing
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -std=gnu11
CFLAGS += -Wall -Wextra -Wshadow -Wundef -Wformat=2 -Wfloat-equal
CFLAGS += -Wbad-function-cast -Waggregate-return -Wstrict-prototypes -Wpacked
CFLAGS += -Wmissing-prototypes -Winit-self -Wmissing-declarations
CFLAGS += -Wmissing-format-attribute -Wunreachable-code -Wshift-overflow
CFLAGS += -Wpointer-arith -Wwrite-strings -Wdouble-promotion -Wnested-externs
CFLAGS += -Wcast-align -Wredundant-decls -Wlong-long -Wmissing-include-dirs
CFLAGS += -Werror=implicit-function-declaration -Wlogical-not-parentheses
CFLAGS += -Wold-style-definition -Wcast-qual -Wdisabled-optimization
CFLAGS += -Wno-unused-function -Wno-unused-parameter

# LCOV Options
LCOV_OPTS += --rc lcov_branch_coverage=1

# Determine the correct linker argument to remove unused symbols based on the
# available linker. trimming unused symbols is important because we don't want
# to get linker errors for missing stubs that don't need to be defined.
LINKER_VERSION = $(shell $(CC) -Wl,-v 2>&1)
ifneq (, $(findstring GNU ld,$(LINKER_VERSION)))
# GNU linker
SYMBOL_STRIP_ARG = -Wl,--gc-sections
else
ifneq (, $(findstring ld64,$(LINKER_VERSION)))
# macOS linker
SYMBOL_STRIP_ARG = -Wl,-dead_strip
else
# Could not identify linker
endif
endif 

build: $(TEST_BINARIES)

$(BINDIR)/% %.gcno: %.c | $(BINDIR)
	$(CC) $(CFLAGS) $(SYMBOL_STRIP_ARG) -fprofile-arcs -ftest-coverage "$(abspath $<)" -o "${BINDIR}/$(basename $(notdir $@))"

%.gcda: $(BINDIR)/%
	rm -f $@
	@printf "Running Test: "
	$<
	@printf "\n"

%.i: %.c
	$(CC) $(CFLAGS) -E "$(abspath $<)" -o "$@"

baseline_coverage.info: $(TEST_GCNO_FILES)
	lcov $(LCOV_OPTS) --capture --initial --directory . --output-file baseline_coverage.info

test_coverage.info: $(TEST_GCNO_FILES) $(TEST_GCDA_FILES)
	lcov $(LCOV_OPTS) --capture --directory . --output-file test_coverage.info

total_coverage.info: baseline_coverage.info test_coverage.info
	lcov $(LCOV_OPTS) --add-tracefile baseline_coverage.info --add-tracefile test_coverage.info --output-file total_coverage.info
	lcov $(LCOV_OPTS) --remove total_coverage.info -o total_coverage.info */unittests/*

lcov_report: total_coverage.info
	genhtml --branch-coverage "$<" --output-directory "$@"

report: lcov_report

debug: $(TEST_BINARIES)
	@if [ -z "$(DEBUGGER)" ] ; then\
		echo "Could not find gdb or lldb."; exit 1;\
	fi
	$(foreach t, $(TEST_BINARIES), $(DEBUGGER) $t; )

preproc: $(TEST_PREPROCS)

$(BINDIR):
	mkdir $(BINDIR)

clean:
	rm -f $(TEST_BINARIES) $(TEST_GCNO_FILES) $(TEST_GCDA_FILES) $(TEST_PREPROCS)
	rm -f baseline_coverage.info test_coverage.info total_coverage.info
	rm -rf lcov_report $(BINDIR)

.PHONY : build report debug preproc clean
