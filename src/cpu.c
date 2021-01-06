/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file simulates the function of a central processing unit (CPU) when executing a process
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "interpreter.h"
#include "shell.h"
#include "pcb.h"

// Initialize cpu
struct CPU cpu = {.IP = 0, .offset = 0, .IR = { '\0' }, .quanta = QUANTA};

// Runs quanta instructions from RAM
int run(int quanta) {
	int i; // For loop counter
	int done = 0; // When this is 1, the function terminates
	
	for (i = 0; i < quanta; i++) {
		if (cpu.IP == -1 || cpu.IP >= RAM_SIZE / PAGE_SIZE || cpu.offset > PAGE_SIZE) {
			return -1; // Error
		}

		if (cpu.offset == PAGE_SIZE || ram[PAGE_SIZE * cpu.IP + cpu.offset] == NULL) {
			return 1; // Generate pseudo-interrput
			// Execution stops because the CPU is at the end of the frame
		}
		
		strcpy(cpu.IR, ram[PAGE_SIZE * cpu.IP + cpu.offset]); // Copy an instruction from ram into the IR
		int len = strlen(cpu.IR); // Compute the length of the instruction
		int endOfFile = len == 0 || cpu.IR[len - 1] != '\n' || cpu.IR[0] == EOF; // The line does not end with a new line character or is the EOF character, which means the end of the file has been reached

		// Execute the instruction
		parse(cpu.IR);

		if (endOfFile) { // Stop executing the script if the end of the file has been reached
			done = 1;
		}

		if (quitExecutingScript == 1) { // Stop executing the script if the quit command was executed
			done = 1;
		}

		// Increment offset
		cpu.offset++;

		if (done) {
			break;
		}
	}

	return 0;
}

// Clears the ready queue
void clearReadyQueue() {
	while (head != NULL) {
		if (head == tail) {
			free(head->pcb);
			free(head);
			break;
		}

		struct ReadyQueue *rq = head;
		head = head->next;
		free(rq->pcb);
		free(rq);
	}

	head = NULL;
	tail = NULL;
}
