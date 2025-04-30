# g++ Make Windows Template

A C++ project template.

## Project Structure

```
.
├── src/
│   └── main.cpp
├── shared/
├── out/
└── Makefile
```

- `src/`: Contains the source code
- `shared/`: Directory for shared resources
- `out/`: Build output directory
- `Makefile`: Build configuration

## Prerequisites

- g++ compiler
- Make
- Windows operating system (due to Windows-specific commands in Makefile)

## Building the Project

The project uses a Makefile for building. Available commands:

- `make`: Builds the project and copies shared resources
- `make clean`: Cleans the build directory
- `make run`: Runs the compiled executable
- `make measure`: Runs the executable and measures execution time

## Build Process

1. The Makefile will:
   - Create the output directory if it doesn't exist
   - Compile the source code with optimization flags (-O3)
   - Enable multi-threading support (-pthread)
   - Copy shared resources to the output directory

## Compiler Flags

- `-Wall`: Enable all warnings
- `-Wextra`: Enable extra warnings
- `-O3`: Maximum optimization level
- `-pthread`: Enable POSIX threads support

## Usage

1. Build the project:
   ```bash
   make
   ```

2. Run the program:
   ```bash
   make run
   ```

3. Measure execution time:
   ```bash
   make measure
   ```

## Cleaning

To clean the build directory:
```bash
make clean
```