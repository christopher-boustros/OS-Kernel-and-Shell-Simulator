/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
#ifndef CPU_H
#define CPU_H

enum {
	INSTRUCTION_SIZE = 1000, // The maximum number of characters in a single instruction
	QUANTA = 2 // The number of instructions to execute before a task-switch
};

// This structure represents a CPU
struct CPU {
	int IP; // Instruction pointer: index of the next frame. This is an integer between 0 and RAM_SIZE / PAGE_SIZE - 1.
	int offset; // The index of the current element in the frame. This is an integer between 0 and PAGE_SIZE - 1.
	char IR[INSTRUCTION_SIZE]; // Instruction register: the the instruction that will be sent to the interpreter for execution
	int quanta; // Quanta field
};

// This structure implements a node of the ready queue in a singly-linked list
// The ready queue is a queue of process control blocks to be executed one by one by the CPU
struct ReadyQueue {
	struct PCB *pcb;
	struct ReadyQueue *next;
};

struct CPU cpu; // The CPU
struct ReadyQueue *head, *tail; // The head and tail of the ready queue

int run(int quanta);
void clearReadyQueue();

#endif
