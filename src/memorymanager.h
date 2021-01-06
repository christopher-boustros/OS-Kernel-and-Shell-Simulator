/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "pcb.h" // For struct PCB

int lastPID;

int findLoadUpdate(struct PCB *pcb, int pageNumber, int PID);
int launcher(char *filename);

#endif
