/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file performs a memory management scheme known as paging
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memorymanager.h"
#include "cpu.h"
#include "kernel.h"

int lastPID = 0; // Last process ID
enum { BUFFER_SIZE = 50 }; // The buffer size for a page name

// A helper function that rounds up a double to an int
int roundUp(double d) {
	int di = (int) d;
	int result;
	if ((double) di == d) {
		result = di;
	} else {
		result = di + 1;
	}

	return result;
}

// Counts the total number of pages that a file f must be split into
int countTotalPages(FILE *f) {
    char c = '\0';
    char beforeEOF = '\0';
    int count = 0;

    while (c != EOF) {
        beforeEOF = c;
        c = getc(f);
        if (c == '\n') {
            count++;
        }
    }
    
    if (beforeEOF != '\n') count++; // Because the last line did not end with a '\n'

    int total = (int) (roundUp((double) count / PAGE_SIZE));

    return total;
}

// Loads the page "[PID].[pageNumber].txt" into the frame [frameNumber] in RAM
void loadPage(int pageNumber, int PID, int frameNumber) {
    char pageName[BUFFER_SIZE];
    snprintf(pageName, BUFFER_SIZE, "BackingStore/%d.%d.txt", PID, pageNumber);
    FILE *pageToLoad = fopen(pageName, "r");

    char buffer[INSTRUCTION_SIZE];

    int k;
	for (k = 0; k < PAGE_SIZE; k++) {
		strcpy(buffer, "\0"); // Clear buffer
		fgets(buffer, INSTRUCTION_SIZE - 1, pageToLoad);
		ram[frameNumber * PAGE_SIZE + k] = strdup(buffer);
		
		if (strcmp(buffer, "\0") == 0) { // Nothing was written to the buffer, so end of file
			ram[frameNumber * PAGE_SIZE + k] = NULL;
			break;
		}
    }
}

// Looks for an available frame in RAM
// If a frame is found, the frame number is returned
// Otherwise, error code -1 is returned
int findFrame() {
    int i;
    for (i = 0; i < RAM_SIZE; i += PAGE_SIZE) { // Traverse the RAM frame by frame
        if (ram[i] == NULL) {
            return i / PAGE_SIZE;
        }
    }

    return -1;
}

// If there is no available frame in RAM, this function is called to find a victim frame to overwrite
int findVictim(struct PCB *p) {
    int victim = rand() % (RAM_SIZE / PAGE_SIZE);

    int i;
    int tryAgain = 1;
    int counter = 0;

    while (tryAgain) {
        tryAgain = 0;
        victim = victim % (RAM_SIZE / PAGE_SIZE);
        victim++;

        // Traverse the page table
        for (i = 0; i < RAM_SIZE / PAGE_SIZE; i++) {
            // If the victim fram selected is used by the current PCB (the one passed in as a parameter),
            // then another victim frame needs to be selected
            if ((p->pageTable)[i] == victim) {
                tryAgain = 1;
            }
        }

        if (counter > RAM_SIZE / PAGE_SIZE) {
            return -1; // Error: The program had more than RAM_SIZE / PAGE_SIZE pages, which is not supported
        }

        counter++;
    }

    return victim;
}

// This function has a different behavior depending on whether the frame corresponding to frameNumber is a victim frame
// If victimFrame is equal to 1, then it is a victim frame. Otherwise, it is not.
//
// If the frame is not a victim, this function updates the page table of PCB p so that pageNumber is associated with frameNumber
// If the frame is a victim, this function also updates the page table of the PCB that owns the victim frame to indicate that it no longer owns the frame
int updatePageTable(struct PCB *p, int pageNumber, int frameNumber, int victimFrame) {
    if (victimFrame) { // If the frame is a victim
        // Traverse the page table of all PCBs in the ready queue to find the PCB that is the owner of the victim frame
        struct PCB *victimPCB = NULL;
        int victimPageNumber = -1;
        struct ReadyQueue *node = head;
        while (node != NULL) {
            int i;
            for (i = 0; i < RAM_SIZE / PAGE_SIZE; i++) {
                if (node->pcb->pageTable[i] == frameNumber) {
                    victimPCB = node->pcb;
                    victimPageNumber = i;
                    break;
                }
            }
            node = node->next;
        }

        if (victimPageNumber == -1) {
            return -1; // Error
        }

        // Update the victim PCB's page table
        victimPCB->pageTable[victimPageNumber] = -1; // victimPageNumber is no longer associated with frameNumber since the frame was taken by another PCB
    }

    // Update the current PCB's page table
    p->pageTable[pageNumber] = frameNumber; // pageNumber is now associated with frameNumber
    
    return frameNumber;
}

// Finds an available or victim frame, loads the page that corresponds with pageNumber to the frame, and updates the page table
int findLoadUpdate(struct PCB *pcb, int pageNumber, int PID) {
    // Find a frame
    int frame = findFrame();
    int victim = 0;

    if (frame == -1) {
        // Find a victim frame
        frame = findVictim(pcb);
        victim = 1;
    }

    if (frame == -1) {
        return -1; // Error
    }

    // Load page to frame
    loadPage(pageNumber, PID, frame);

    // Update page table
    updatePageTable(pcb, pageNumber, frame, victim);

    return 0; // No error
}

// Opens the file filename, stores it in the backing store as multiple pages, creates a PCB for the file, and loads one or more pages into RAM
int launcher(char *filename) {
    FILE* originalFile = fopen(filename, "r");
    int pages_max = countTotalPages(originalFile);
    fclose(originalFile);

    if (pages_max > RAM_SIZE / PAGE_SIZE) {
        return -1; // Error: script has too many instructions
    }

    originalFile = fopen(filename, "r");
    
    char c = '\0';
    char newName[BUFFER_SIZE] = "";
    int pageCount = 0;
    lastPID++;

    while (pageCount < pages_max) {
        snprintf(newName, BUFFER_SIZE, "BackingStore/%d.%d.txt", lastPID, pageCount++);
        FILE *target = fopen(newName, "w");
        int i;
        for (i = 0; i < PAGE_SIZE; i++) {
            do {
                c = fgetc(originalFile);
                if (c != EOF) { // Do not add the EOF to the file
                    if (i != PAGE_SIZE - 1 || c != '\n') { // Do not add the '\n' for the last line
                        fputc(c, target);
                    }
		        }
            } while (c != '\n' && c != EOF);
            
            if (c == EOF) {
                break;
            }
        }
        fclose(target);
    }

    fclose(originalFile);

    int numberOfPagesToLoad;
    if (pages_max > 2) {
        numberOfPagesToLoad = 2;
    } else {
        numberOfPagesToLoad = pages_max;
    }

    struct PCB *pcb = initPCB(lastPID, pages_max);

    int i;
    for (i = 0; i < numberOfPagesToLoad; i++) { 
        int tag = findLoadUpdate(pcb, i, lastPID);
        if (tag == -1) {
            return -2; // Error: could not find victim
        }
    }

    return 0; // No error
}
