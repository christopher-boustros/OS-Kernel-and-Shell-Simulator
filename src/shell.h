/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
#ifndef SHELL_H
#define SHELL_H

int shellRunning;

int shellUI();
int parse(char* line);
void exitShell();

#endif
