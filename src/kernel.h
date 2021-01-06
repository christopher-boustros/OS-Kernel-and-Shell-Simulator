/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
#ifndef KERNEL_H
#define KERNEL_H

struct PCB *initPCB(int PID, int pages_max);
void scheduler();
int boot();
int kernel();
int shutDown();

#endif
