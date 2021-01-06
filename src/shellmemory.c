/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file implements the shell memory where shell variables are stored
#include <stdlib.h>
#include <string.h>

enum {
	SHELL_MEMORY_SIZE = 1000 // The number of variables that can be stored in the shell memory by the user with the set VAR STRING shell command.
};

// A structure that represents a variable and its value
struct MEM {
	char var[SHELL_MEMORY_SIZE]; // The name of a variable stored in shell memory
	char value[SHELL_MEMORY_SIZE]; // The value of the variable
};

// The shell memory (an array of struct MEM)
struct MEM mem[SHELL_MEMORY_SIZE];

int positions[SHELL_MEMORY_SIZE] = { 0 }; // To keep track of which elements of the mem array are null (0 represents null)

// Sets the value of a variable with name var
// If the variable already exists in shell memory, then the value is overwritten
// Otherwise, a new variable is created
void setVar(char *var, char *value) {
	int i;

	// Traverse the shell memory to find the position where the value should be set
	for (i = 0; i < SHELL_MEMORY_SIZE; i++) {
		// If position i in shell memory contains no variable, then no variable has been
		// found with the name var, so a new variable must be created at position i
		if (positions[i] == 0) {
			break;
		}

		// Check if the variable at position i has the name var
		if (strcmp(var, mem[i].var) == 0) {
			// If so, then update its value
			strcpy(mem[i].value, value);
			return;
		}
	}

	// If there is no more space in shell memory, do nothing
	if (i == SHELL_MEMORY_SIZE) return;
	
	// Since no variable was found with the name var, create a new variable
	strcpy(mem[i].var, var);
	strcpy(mem[i].value, value);
	positions[i] = 1; // Indicates that position i in shell memory contains a variable
}

// Returns the value of the variable with name var
char* ValueOfVar(char *var) {
	int i;
	for (i = 0; i < SHELL_MEMORY_SIZE; i++) {

		if (strcmp(var, mem[i].var) == 0) {
			return mem[i].value;
		}
	}

	return "\0"; // Error: variable not found
}

// Clears all variables in shell memory
void clearShellMemory() { 
	int i;
	for (i = 0; i < SHELL_MEMORY_SIZE; i++) {
		strcpy(mem[i].value, "\0");
		strcpy(mem[i].var, "\0");
		positions[i] = 0;
	}
}
