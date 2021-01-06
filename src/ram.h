/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
#ifndef RAM_H
#define RAM_H

enum {
    RAM_SIZE = 40 // The number of characters that can be stored in ram
};

// This the the RAM, an array of strings (each string is an instruction in a file/script)
char *ram[RAM_SIZE]; 

void clearRam();

#endif
