#
#
# SAMD21 XPlained Pro Board Makefile
#
#

##### Specify Target #####
TARGET = sam/samd21/samd21j18a

##### Source Files #####
#BOARD_SOURCES = $(wildcard $(BOARDSDIR)/$(BOARD)/board.c)
# Use board file from samd21 MCU board
BOARD_SOURCES = src/boards/mcu/board.c

##### Include Directories #####
BOARD_INC_DIRS = $(BOARDSDIR)/$(BOARD)

##### cflags #####
BOARD_CFLAGS =

