/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
#ifndef PCB_H
#define PCB_H

#include "ram.h" // For RAM_SIZE

enum {
    PAGE_SIZE = 4 // The number of instructions per page. This is equal to the number of instructions per frame, so page size = frame size.
};

// This is the structure for a process control block (PCB)
// A PCB is a data structure that stores the information about a process that
// the CPU is executing. 
//
// In this program, each file/script being executed by in the shell is represented 
// by one PCB, which stores information about which page of the file is currently being
// executed and in which frame in RAM the page is stored.
struct PCB {
	int PID; // Process ID
	int PC_page; // The index of the current page being executed. This is an integer between 0 and RAM_SIZE / PAGE_SIZE - 1.
	int PC_offset; // The index of the current line of the page being executed (also known as the page offset). This is an integer between 0 and PAGE_SIZE - 1.
	int pageTable[RAM_SIZE / PAGE_SIZE]; // pageTable[i] is the index of the frame where the page with index i is stored in RAM. pageTable[i] == -1 means that page i is not stored in a frame
	int pages_max; // The total number of pages that the file/script is made up of
};

struct PCB *makePCB(int PID, int pages_max);

#endif
