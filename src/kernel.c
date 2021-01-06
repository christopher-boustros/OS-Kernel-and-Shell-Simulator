/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file simulates an operating system kernel
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"
#include "shell.h"
#include "cpu.h"
#include "memorymanager.h"

const char* BACKING_STORE = "BackingStore"; // The backing store directory
enum { BUFFER_SIZE = 100 };
char removeBackingStore[BUFFER_SIZE]; // The command to remove the backing store directory
char createBackingStore[BUFFER_SIZE]; // The command to create the backing store directory

// Enqueue a ready queue node (which contains a PCB)
void addRQToReady(struct ReadyQueue *rq) {
	if (head == NULL) {
		head = rq;
		tail = rq;
	} else {
		tail->next = rq;
		tail = rq;
	}

	rq->next = NULL;
}

// Creates a ready queue node from a PCB and enqueues it to the ready queue
void addPCBToReady(struct PCB *pcb) {
	struct ReadyQueue *rq = (struct ReadyQueue *) malloc(sizeof(struct ReadyQueue));
	rq->pcb = pcb;
	rq->next = NULL;

	addRQToReady(rq);
}

// Dequeue a ready queue node (which contains a PCB)
struct ReadyQueue *removeFromReady() {
	if (head == NULL) {
		return NULL;
	}

	struct ReadyQueue *rq = head;
	head = head->next;
	if (head == NULL) {
		tail = NULL;
	}

	return rq;
}

// Assigns PCB's to the CPU one at a time from the ready queue
void scheduler() {
	struct ReadyQueue *rq = removeFromReady();
	while (rq != NULL) { // While the ready queue is not empty
		int pcbTerminated = 0;
		// Copy the offset from the PCB into the offset of the CPU
		cpu.offset = rq->pcb->PC_offset;
		// Copy the frame number from the PCB into the IP of the CPU
		cpu.IP = rq->pcb->pageTable[rq->pcb->PC_page];

		// Run quanta instructions
		int tag = run(cpu.quanta);

		if (tag == -1) { // Error
			// Do something
		}
		else if (tag == 1) { // CPU offset reached PAGE_SIZE
			// Determine the next page and reset the offset
			rq->pcb->PC_page++;
			rq->pcb->PC_offset = 0;

			if (rq->pcb->PC_page > rq->pcb->pages_max - 1) { // If there are no more pages to execute
				// Terminate the PCB
				free(rq->pcb);
				free(rq);
				pcbTerminated = 1;
			} else {
				if (rq->pcb->pageTable[(rq->pcb->PC_page)] == -1) { // If the page is not stored inside a frame in ram 
					// Page fault
					findLoadUpdate(rq->pcb, rq->pcb->PC_page, rq->pcb->PID);
				}
			}
		} else {
			// Update PCB offset
			rq->pcb->PC_offset = cpu.offset;
		}
		
		if (quitExecutingScript || pcbTerminated ) { // If script needs to quit or the pcb has been terminated
			if (!pcbTerminated) {
				// Terminate the PCB
				free(rq->pcb);
				free(rq);
			}

			quitExecutingScript = 0; // Reset quitExecutingScript
		} else {
			// Add PCB to end of ready queue
			addRQToReady(rq); 
		}

		rq = removeFromReady();
	}
}

// Creates a PCB and adds it to the ready queue
struct PCB *initPCB(int PID, int pages_max) {
	struct PCB *pcb = makePCB(PID, pages_max);
	addPCBToReady(pcb);
	return pcb;
}

// The commands to execute before starting the kernel
int boot() {
	int error = 0;

	// Initialize every cell of ram to NULL
	for (int i = 0; i < RAM_SIZE; i++) {
		ram[i] = NULL;
	}

	// Prepare the Backing Store
	snprintf(removeBackingStore, BUFFER_SIZE, "if [ -d \"%s\" ]; then rm -r BackingStore; fi", BACKING_STORE);
	snprintf(createBackingStore, BUFFER_SIZE, "mkdir %s", BACKING_STORE);
	error += system(removeBackingStore); // Remove the BackingStore directory if it exists
	error += system(createBackingStore); // Create the BackingStore directory

	return error;
}

// The commands to execute after exiting the kernel
int shutDown() {
	// Remove the Backing Store if it exists
	int error = system(removeBackingStore);
	return error;
}

// Starts the kernel
int kernel() {
	int error = 0;

	printf("Kernel loaded!\n");
	error += shellUI();
	printf("Exiting kernel...\n");

	return error;
}
