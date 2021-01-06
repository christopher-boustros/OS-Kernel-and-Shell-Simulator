/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file simulates an operating system shell
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "interpreter.h"

int shellRunning = 1; // When this is equal to 0, the shell will stop running

// Removes spaces at the beginning of a string, if any
int removeLeadingSpaces(char *line, int len) {
	// Check for spaces at the beginning of the line
	int spaceCounter;
	for (spaceCounter = 0; spaceCounter < len; spaceCounter++) {
		if (line[spaceCounter] != ' ') {
			break;
		}
	}

	// If there are spaces at the beginning of the line, remove them
	if (spaceCounter > 0) {
		memmove(line, line + spaceCounter, len - spaceCounter + 1);
		len = len - spaceCounter;
	}

	return len; // Return the length of the new line
}

// Parses a line into an array of words delimited by spaces and passes them to the interpreter to
// be interpreted and executed
int parse(char* line) {
	int len = strlen(line);

	if (len == 0) {
		return -1; // Ignore it
	}

	len = removeLeadingSpaces(line, len);

	if (line[0] == ' ' || line[0] == '\n') { // If the line is just a space or a new line character
		return -1; // Ignore it
	}

	if (line != NULL && len > 0 && line[len - 1] == '\n' ) { // If the line is non-empty and ends with a new line character
		line[len - 1] = '\0'; // Replace the new line character with a null character
	}

	char* words[INSTRUCTION_SIZE]; // The array of words in the line (words are delimited by spaces)
	
	// Clear words array
	int j;
	for (j = 0; j < INSTRUCTION_SIZE; j++) {
		words[j] = NULL;
	}
	
	// Initialize words array
	int i = 0;
	words[i] = strtok(line, " ");
	while (words[i] != NULL) {
		words[++i] = strtok(NULL, " ");
	}

	int errorCode = interpreter(words);

	if (errorCode != 0) {
		errorCode = -1;
	}

	return errorCode;
}

// Implements the shell UI that promts the user for input
int shellUI() {
	char line[INSTRUCTION_SIZE]; // The string that stores a single instruction from the user
	int len; // The length of line
	char *prompt = "$"; // The shell prompt

	printf("Shell version 1.0 loaded!\n");
	printf("Enter 'help' to display all available commands\n");

	while(shellRunning) {
		printf("%s ", prompt);

		fflush(stdout);
		
		strcpy(line, "\0"); // Clear the line
		fgets(line, INSTRUCTION_SIZE - 1, stdin); // Read the user input, up to a maximum of INSTRUCTION_SIZE - 1 characters
		len = strlen(line); // Compute the length of the user input
		int endOfRedirection = len == 0 || line[len - 1] != '\n'; // The line does not end with a new line character. This means it was not entered directly by the user, so it is the last line of redirection.
		
		// Parse and interpret the line (this executes the line)
		parse(line);

		// Try to reopen stdin if end of redirection
		// Redirection is when the contents of a file is redirected to stdin (for example, if the '<' operator like this: mykernel < file.txt)
		if (endOfRedirection) { 
			printf("\nRedirection finished!\n");
			
			if (!freopen("/dev/tty", "r", stdin)) { // Try to reopen stdin to read from the command line after redirection (after ./mykernel < testfile.txt)
    			// If could not repoen stdin to read from terminal
    			break;
			}
		}
	}

	printf("Exiting shell...\n");
	return 0;
}

// Indicates to the shell that it needs to exit
void exitShell() {
	shellRunning = 0;
}
