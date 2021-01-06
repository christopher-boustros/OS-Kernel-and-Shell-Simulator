# Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
# SPDX-License-Identifier: MIT

# Define the compiler
CC			:=	gcc

# Define the target directory and target program
TARGETDIR	:=	bin
_TARGET		:=	mykernel
TARGET		:=	$(TARGETDIR)/$(_TARGET)

# Define the source directory and source files
SOURCEDIR	:=	src
_SOURCES	:=	main.c kernel.c shell.c interpreter.c shellmemory.c cpu.c pcb.c ram.c memorymanager.c
SOURCES		:=	$(patsubst %,$(SOURCEDIR)/%,$(_SOURCES))
_HEADERS	:=	kernel.h shell.h interpreter.h shellmemory.h cpu.h pcb.h ram.h memorymanager.h
HEADERS		:=	$(patsubst %,$(SOURCEDIR)/%,$(_HEADERS))

# Define the object directory and object files
OBJECTDIR	:=	obj
_OBJECTS	:=	$(patsubst %.c, %.o, $(_SOURCES))
OBJECTS		:=	$(patsubst %,$(OBJECTDIR)/%,$(_OBJECTS))

# Define the test directory
# This is where the test files for the target program are located, and this will
# be the working directory of the target program when executing 'make run'
TESTDIR		:=	tests

# Make the target program in the target directory
$(TARGET): $(TARGETDIR) $(OBJECTDIR) $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)

# Make the object files in the object directory
# Uses a static pattern rule and automatic variables:
# each target $(OBJECTDIR)/%.o in $(OBJECTS) has prerequisite $(SOURCEDIR)/%.c and all header files $(HEADERS)
$(OBJECTS): $(OBJECTDIR)/%.o : $(SOURCEDIR)/%.c $(HEADERS)
	$(CC) -c $< -o $@

# Make the target directory
$(TARGETDIR):
	mkdir -p $(TARGETDIR)

# Make the object directory
$(OBJECTDIR):
	mkdir -p $(OBJECTDIR)

# Phony targets
.PHONY: run clean

# Run the target program
# The working directory of the target program will be the test directory
run: $(TARGET)
	cd $(TESTDIR) && \
	./../$(TARGET)

# Clean the object and target directories
clean:
	rm -r $(OBJECTDIR)/*; \
	rm -r $(TARGETDIR)/*;