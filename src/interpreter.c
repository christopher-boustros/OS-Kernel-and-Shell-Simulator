/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file implements an interpreter that interprets an instruction to determine and executes its corresponding command
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shellmemory.h"
#include "shell.h"
#include "cpu.h"
#include "memorymanager.h"
#include "kernel.h"

// Define constants for the script stack
enum {
	SCRIPT_STACK_SIZE = 200, // The size of the script stack
	EMPTY = 0, // The value of an empty element in the script stack
	EXEC = -1, // This value at the head of the script stack indicates that the last script was executed with the 'exec' command
	RUN = 1 // This value at the head of the script stack indicates that the last script was executed with the 'run' command
};

int runningScript = 0; // The number of nested 'run' commands being executed: to know if a line being interpreted comes from a script (from the 'run' command) or was typed by the user (in order to interpret the quit command correctly)
int executingScript = 0; // To know if a line being interpreted comes from a script (from the 'exec' command) or was typed by the user (in order to interpret the quit command correctly)
int quitRunningScript = 0; // Indicates whether the currently running script (from the 'run' command) needs to quit
int quitExecutingScript = 0; // Indicates whether the currently running script (from the 'exec' command) needs to quit
int scriptStack[SCRIPT_STACK_SIZE] = { EMPTY }; // The script stack keeps track of how the last script was executed: -1 means from 'exec' and 1 means from 'run'
int scriptStackIndex = -1; // The index of the last element of scriptStack
int mustResetInterpreterVariables = 0; // Indicates whether all of the interpreter variables above need to be reset (which is the case after running the stopAllScripts() method)

// Pushes integer i to the script stack
int pushToScriptStack(int i) {
	if (scriptStackIndex >= SCRIPT_STACK_SIZE - 1) {
		scriptStackIndex = SCRIPT_STACK_SIZE - 1;
		return -1; // Stack full
	}

	scriptStack[++scriptStackIndex] = i;
	return 0;
}

// Pops an integer from the script stack
int popFromScriptStack() {
	if (scriptStackIndex < 0) {
		scriptStackIndex = -1;
		return EMPTY;
	}

	int i = scriptStack[scriptStackIndex];
	scriptStack[scriptStackIndex--] = EMPTY;
	return i;
}

// Peeks the integer from the script stack
int peekFromScriptStack() {
	return scriptStack[scriptStackIndex];
}

// Clears the script stack
void clearScriptStack() {
	scriptStackIndex = SCRIPT_STACK_SIZE - 1;

	while (scriptStackIndex > -1) {
		scriptStack[scriptStackIndex] = EMPTY;
		scriptStackIndex--;
	}
}

// Resets the interpreter variables, which is necessary after running stopAllScripts())
void resetIntepreterVariables() {
	runningScript = 0;
	executingScript = 0;
	quitRunningScript = 0;
	quitExecutingScript = 0;
	clearScriptStack();
	mustResetInterpreterVariables = 0;
}

// Performs the 'help' command
void help() {
	printf(
			"help\t\t\t\tDisplays all available commands\n"
			"quit\t\t\t\tExits the shell or the script with \"Bye!\"\n"
			"clearmem\t\t\tClears the shell memory\n"
			"set VAR STRING\t\t\tAssigns STRING to variable VAR in shell memory\n"
			"print VAR\t\t\tDisplays the value assigned to variable VAR\n"
			"run SCRIPT.TXT\t\t\tExecutes the file SCRIPT.TXT\n"
			"exec S1.TXT S2.TXT S3.TXT\tExecutes up to three files concurrently\n"
			);
}

// Performs the 'quit' command
void quit() {
	printf("Bye!\n");
	if (!runningScript && !executingScript) {
		exitShell(); // Exit the shell
	} else {
		// Exit the script
		int last = peekFromScriptStack();
		if (last == EXEC) { // Last script was from 'exec' command
			quitExecutingScript = 1;
		} else { // Last script was from 'run' command
			quitRunningScript = 1;
		}
	}
}

// Performs the 'clearmem' command
void clearmem() {
	clearShellMemory();
	printf("Shell memory cleared!\n");
}

// Performs the 'set' command
void set(char* var, char* string) {
	setVar(var, string);
}

// Performs the 'print' command
void print(char* var) {
	char *str = (char *) malloc(INSTRUCTION_SIZE * sizeof(char));
	strcpy(str, ValueOfVar(var));

	if (*str != '\0') {
		printf("%s\n", str);
	} else {
		printf("Error: Variable '%s' not found\n", var);
	}
}

// Performs the 'exec' command.
// The 'exec' command will execute up to three files concurrently.
// Unlike the 'run' command, 'exec' will use the paging memeory management scheme,
// which will create page files in the backing store.
void exec(char *names[], int size) {
	int i;
	FILE* files[3];

	for (i = 0; i < size; i++) {
		if (! (files[i] = fopen(names[i], "r"))) {
			printf("Error: Script '%s' not found\n", names[i]);
			clearRam();
			return;
		}

		fclose(files[i]);
	}
	
	for (int i = 0; i < size; i++) {
		// Load file into ram, create PCB for that program, and add PCB to ready queue
		int error = launcher(names[i]);
		if (error != 0) { // There is a load error
			if (error == -1) {
				printf("Error: Script '%s' could not be loaded since it has more than %d instructions!\n", names[i], RAM_SIZE);
			} else {
				printf("Error: Script '%s' could not be loaded because a victim frame could not be found!\n", names[i]);
			}

			clearRam();
			clearReadyQueue();
			return;
		}
	}

	// Start execution of all the loaded programs
	scheduler();

	// Clear the ram once all loaded programs have finished execution
	clearRam();

	// Clear the ready queue
	clearReadyQueue();
}

// Performs the 'run' command.
// The 'run' command will not use the paging memory management scheme,
// unlike the 'exec' command.
void runCommand(char* file) {
	FILE* f;

	if (! (f = fopen(file, "r"))) {
	       printf("Error: script '%s' not found\n", file);
	       return;
	}

	char line[INSTRUCTION_SIZE];

	while (1) {
		strcpy(line, "\0"); // Clear the line
		fgets(line, INSTRUCTION_SIZE - 1, f); // Read the next line from the file, up to a maximum of INSTRUCTION_SIZE - 1 characters
		int len = strlen(line); // Compute the length of the line
		int endOfFile = len == 0 || line[len - 1] != '\n' || line[0] == EOF; // The line does not end with a new line character or is the EOF character, which means the end of the file has been reached

		parse(line);

		if (endOfFile) { // Stop executing the script if the end of the file has been reached
			break;
		}

		if (quitRunningScript == 1) { // Stop running the script if the quit command was executed
			quitRunningScript = 0;
			break;
		}
	}
	
	fclose(f);
}

// Prints an error for an unknown command
void unknown(char* command, int error) {
	switch (error) {
		case -1: printf("Error: The 'help' command cannot take parameters!\n"); break;
		case -2: printf("Error: The 'quit' command cannot take parameters!\n"); break;
		case -3: printf("Error: The 'clearmem' command cannot take parameters!\n"); break;
		case -4: printf("Error: The 'set' command must take exactly two parameters!\n"); break;
		case -5: printf("Error: The 'print' command must take exactly one parameter!\n"); break;
		case -6: printf("Error: The 'run' command must take exactly one parameter!\n"); break;
		case -7: printf("Error: The 'exec' command must take at least one parameter!\n"); break;
		case -8: printf("Error: The 'exec' command cannot take more than three parameters!\n"); break;
		case -9: printf("Error: Recursive 'exec' calls are not supported!\n"); break;
		case -10: printf("Error: Unknown command '%s'\n", command); break;
	}
}

// Stops all scripts being executed by the 'run' and 'exec' commands
void stopAllScripts() {
	mustResetInterpreterVariables = 1;
	clearRam();
	clearReadyQueue();
}

// Handles the error of the script stack being full
void scriptStackIsFullError() {
	printf("Error: Maximum recursion depth (%d) reached\n", SCRIPT_STACK_SIZE);
	stopAllScripts();
}

// Interprets parsed input from the user and runs the appropritate command
int interpreter(char *words[]) {

	// The interpreter variables must be reset after stopAllScripts() is called
	if (mustResetInterpreterVariables) {
		// Before resetting  the interpreter variables, quit the script that is being executed or running, if any
		if (runningScript || executingScript) {
			if (runningScript) {
				quitRunningScript = 1;
			}
			
			if (executingScript) {
				quitExecutingScript = 1;
			}

			return 0;
		}
		else {
			resetIntepreterVariables();
		}
	}

	int errorCode = 0;

	if (strcmp(words[0], "help") == 0) {
		if (words[1] == NULL) {
			help();
		} else {
			errorCode = -1;
		}
	} else if (strcmp(words[0], "quit") == 0) {
		if (words[1] == NULL) {
			quit();
		} else {
			errorCode = -2;
		}
	} else if (strcmp(words[0], "clearmem") == 0) {
		if (words[1] == NULL) {
			clearmem();
		} else {
			errorCode = -3;
		}
	} else if (strcmp(words[0], "set") == 0) {
		if (words[2] != NULL && words[3] == NULL) {
			set(words[1], words[2]);
		} else {
			errorCode = -4;
		}
	} else if (strcmp(words[0], "print") == 0) {
		if (words[1] != NULL && words[2] == NULL) {
			print(words[1]);
		} else {
			errorCode = -5;
		}
	} else if (strcmp(words[0], "run") == 0) {
		if (words[1] != NULL && words[2] == NULL) {
			if(pushToScriptStack(RUN) == 0) { // Try to push 1 to the script stack to indicate that this script was executed with the 'run' command
				// If the stack is not full, proceed with the run command
				runningScript++; // Increment the number of nested run commands being executed
				runCommand(words[1]);
				runningScript--; // Decrement the number of nested run commands being executed
				popFromScriptStack(); // Pop the 1 from the script stack since the script is no longer being executed
			} else {
				// If the stack is full
				scriptStackIsFullError();
			}
		} else {
			errorCode = -6;
		}
	} else if (strcmp(words[0], "exec") == 0) {
		if (words[1] == NULL) {
			errorCode = -7;
		}
		else if (words[4] != NULL) {
			errorCode = -8;
		}
		else if (executingScript == 1) {
			errorCode = -9;
		}
		else { // exec has either 1, 2, or 3 parameters
			char* parameters[3] = { "\0" }; // An array of strings to hold the parameters
			int len = 0;

			int i;
			for (i = 0; i < 3; i++) {
				if (words[i+1] != NULL) {
					parameters[i] = words[i+1]; // Add parameters words[i+1] if it is not null (for up to 3 parameters)
					len++;
				}
			}

			if (pushToScriptStack(EXEC) == 0) { // Try to push -1 to the script stack to indicate that these parameters were executed with the 'exec' command
				// If the stack is not full, proceed with the exec command
				// Execute the parameters (which are file names)
				executingScript = 1; // Indicate that the 'exec' command is running
				exec(parameters, len); // Execute the parameter scripts
				executingScript = 0; // Indicate that the 'exec' command is not running
				popFromScriptStack(); // Pop the -1 from the script stack since the parameters are no longer being executed
			} else {
				// If the stack is full
				scriptStackIsFullError();
			}
		}
	} else {
		errorCode = -10;
	}

	if (errorCode) {
		unknown(words[0], errorCode);
	}

	return errorCode;
}
