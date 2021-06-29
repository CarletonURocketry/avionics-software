#
#
# BIG MCU Board Revision A Board Makefile
#
#

##### Specify Target #####
TARGET = sam/same54/same54p20a

##### Source Files #####
BOARD_SOURCES = $(wildcard $(BOARDSDIR)/$(BOARD)/../board.c)

##### Include Directories #####
BOARD_INC_DIRS = $(BOARDSDIR)/$(BOARD)

##### cflags #####
BOARD_CFLAGS =
