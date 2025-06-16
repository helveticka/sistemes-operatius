# Sistemes Operatius - 21708/21718

## Content
In this repository there are two directorys, each one storing the projects of the differents parts of the subject (SO I & SO II).

### SO_I

#### P1: my_lib functions
In this exercise, we coded string-related functions included in the C library "string.h". Furthermore, we also included stack-manager functions.

#### P2: mini shell
This is the main project, where we implemented a shell that can manage internal commands (jobs, fg, bg...) and external commands on a bash terminal. Each level includes the specified functions and their debug messages. The final version of my_shell works perfectly and properly manages different threads.

#### P3: threads and mutual exclusion
The final exercise's purpose is to access a stack with different threads and manipulate its elements while adhering to concurrency principles. In stack_counters we manage the stacks, and with reader.c we can print the stack information.

### SO_II

This project consists of implementing a file system simulation on top of a virtual disk represented by a regular file, following a structure inspired by `ext2`. The main objective is to develop a modular set of libraries organized into levels, each providing primitive operations to manage a hierarchical file system with support for both directories and files. These primitives (`mi_creat`, `mi_read`, `mi_write`, `mi_unlink`, etc.) replicate the basic functionalities of a real file system. Additionally, a tool called `mi_mkfs` is included to create the initial structure of the file system, alongside various command-line programs (`mi_ls`, `mi_rm`, `mi_stat`, `mi_cat`, `mi_chmod`, `mi_ln`, ...) that emulate standard GNU/Linux commands.

A simulation program is also provided to test concurrent access: it spawns 100 processes at regular intervals, each creating its own subdirectory and writing 50 structured records into a file (`prueba.dat`) at random positions. This stress test evaluates the correct behavior of direct and indirect inode pointers while ensuring data integrity under concurrent writes. Synchronization mechanisms are implemented using mutex semaphores to protect critical metadata (superblock, bitmap, inode array). After the writing phase, a verification stage analyzes each file to extract meaningful information (first/last write, min/max positions) and confirm consistency.

The final part involves scripting a sequence of commands to test the user-level utilities (`mi_cat`, `mi_chmod`, `mi_ln`, `mi_ls`, `mi_rm`, `mi_stat`) using the previously generated structure, ensuring correct integration between user commands and internal primitives. This practice integrates key operating system concepts such as file system architecture, indexing with inode structures, dynamic block allocation, metadata management, and inter-process synchronization, offering a comprehensive hands-on experience of OS-level storage handling.

## Authors
Developed by [Xavi Campos](https://github.com/XaviCampos2005), [Pedro Félix](https://github.com/PedroFelix8) & [Harpo Joan](https://github.com/helveticka)

## License
This repository is licensed under a Creative Commons Attribution-NonCommercial 4.0 International License.
Copyright (c) 2024 Xavi Campos, Pedro Félix & Harpo Joan
