/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file implements a process control block (PCB)
#include <stdlib.h>

#include "pcb.h"

// Creates a PCB
struct PCB *makePCB(int PID, int pages_max) {
	struct PCB *pcb = (struct PCB *) malloc(sizeof(struct PCB));
	pcb->PID = PID;
	pcb->PC_page = 0;
	pcb->PC_offset = 0;
	pcb->pages_max = pages_max;

	int i;
	for (i = 0; i < RAM_SIZE / 4; i++) {
		pcb->pageTable[i] = -1;
	}

	return pcb;
}
