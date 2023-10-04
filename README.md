# File System Simulator

This is a simple file system simulator implemented in C++. The program allows you to perform various file system operations on a simulated disk, such as creating and deleting files, reading and writing to files, and managing file descriptors. Below, you'll find instructions on how to use the program and an overview of its components.

## Prerequisites

Before running the program, make sure you have the following:

- C++ compiler (e.g., g++)
- Standard C++ libraries

## Usage

1. Compile the code using your C++ compiler. For example:

    ```bash
    g++ -o filesystem main.cpp
    ```

2. Run the program:

    ```bash
    ./filesystem
    ```

3. Once the program is running, you can enter commands interactively. Here are the available commands and their descriptions:

   - `0`: Exit the program.
   - `1`: List all files in the file system.
   - `2`: Format the file system with specified block size and direct entries.
   - `3`: Create a new file.
   - `4`: Open an existing file.
   - `5`: Close a file.
   - `6`: Write data to a file.
   - `7`: Read data from a file.
   - `8`: Delete a file.
   - `9`: Copy a file.
   - `10`: Rename a file.

4. Follow the prompts to provide additional input as needed for each command.

## Components

### `main.cpp`

This is the main program that manages the user interface and handles file system commands. It interacts with the following classes:

### `fsInode`

This class represents an Inode in the file system. It contains information about the file's size, the number of blocks in use, pointers to direct blocks, and information about single and double indirect blocks.

### `FileDescriptor`

This class represents a file descriptor, which is used to manage open files. It contains a pair of the file name and its associated `fsInode`. It also keeps track of whether the file descriptor is in use.

### `fsDisk`

This class simulates the disk and manages the overall file system. It includes functions for formatting the disk, allocating and deallocating blocks, reading and writing data blocks, and managing the directory structure.

## Note

This file system simulator is a simplified educational tool and may not be suitable for production use. It provides a basic understanding of how file systems work by simulating their core concepts.

Please use this simulator responsibly and feel free to extend or modify it for your educational purposes or experimentation.
