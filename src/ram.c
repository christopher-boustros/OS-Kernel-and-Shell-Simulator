/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file simulates random access memory (RAM)
#include <stdlib.h>
#include <string.h>

#include "ram.h"

// Clears the RAM
void clearRam() {
	int k;

	// Traverse the ram array
	for (k = 0; k < RAM_SIZE; k++) {
		ram[k] = NULL;
	}
}
