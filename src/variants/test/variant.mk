#
#
# Test Variant Makefile
#
#

##### Source Files #####
VARIANT_SOURCES = $(wildcard $(VARIANTSDIR)/$(VARIANT)/../variant.c)

##### Include Directories #####
VARIANT_INC_DIRS = $(VARIANTSDIR)/$(VARIANT)
