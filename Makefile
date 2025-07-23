# Project Name
TARGET = reverb_pedal

# Sources
CPP_SOURCES = reverb_pedal.cpp

USE_DAISYSP_LGPL = 1

# Library Locations
LIBDAISY_DIR = DaisyExamples/libDaisy/
DAISYSP_DIR = DaisyExamples/DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
