/*
 * Copyright (c) 2021 Christopher Boustros <github.com/christopher-boustros>
 * SPDX-License-Identifier: MIT
 */
// This file starts the kernel and shell
#include "kernel.h"

// Starts and exits the kernel
int main() {
	int error = 0;
	error += boot(); // Performs the commands necessary before starting the kernel
	error += kernel(); // Starts and eventually exits the kernel
	error += shutDown(); // Performs the commands necessary after exiting the kernel
	return error;
}
