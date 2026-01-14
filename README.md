# Systems Software Toolchain – Assembler, Linker & Emulator


## Project Summary

This project implements a complete systems software toolchain for a custom 32-bit Von Neumann architecture. It includes a one-pass assembler, an architecture-independent linker, and a CPU emulator, enabling programs to be assembled, linked, and executed end-to-end. The project demonstrates strong fundamentals in low-level systems programming, instruction set design, relocation handling, and CPU/memory interaction.

---

## Overview

The goal of this project is to replicate the core components of a real-world compilation and execution pipeline. Starting from assembly source code, the toolchain generates relocatable object files, resolves symbols across multiple modules, produces executable images, and finally emulates program execution on a custom CPU model.

The project was developed as part of a university-level **Systems Software** course and emphasizes correctness, clarity of design, and close-to-hardware reasoning.

---

## Features

### 🧩 Assembler
- One-pass assembler for a custom assembly language
- Generates relocatable object files
- Supports:
  - Sections
  - Symbol tables
  - Relocation entries
- Produces an ELF-like object format

### 🔗 Linker
- Architecture-independent design
- Resolves symbols across multiple object files
- Performs relocation and section placement
- Supports:
  - `-relocatable` output
  - Final executable output in hex format

### 🖥 Emulator
- Emulates a 32-bit Von Neumann CPU
- Implements fetch–decode–execute cycle
- Supports memory-mapped I/O:
  - Terminal
  - Timer
- Prints final CPU register state after program execution

---

## Project Structure

```
.
├── src/
  ├── assembler.cpp
  ├── linker.cpp
  └── emulator.cpp
├── inc/
  ├── assembler.hpp
  ├── linker.hpp
  └── emulator.hpp
├── misc/
  ├── flex
  └── bison
├── tests/
├── my_tests
├── makefile
└── README.md
```

---

## Build Instructions

### Requirements
- Linux (amd64)
- GCC / Clang
- Make

### Build
```bash
make
```

This builds:
- the assembler
- the linker
- the emulator

---

## Usage

### Assemble
```bash
./assembler input.s -o input.o
```

### Link
Generate final executable:
```bash
./linker input.o -hex -o program.hex
```

Generate relocatable output:
```bash
./linker input.o -relocatable -o output.o
```

### Emulate
```bash
./emulator program.hex
```

After execution, the emulator prints the final CPU register values.

---

## Example Workflow

```bash
./assembler tests/example.s -o example.o
./linker example.o -hex -o example.hex
./emulator example.hex
```

---

## Key Concepts Demonstrated

- Instruction Set Architecture (ISA) fundamentals
- Instruction encoding and decoding
- Symbol tables and relocation processing
- Multi-file linking
- CPU and memory emulation
- Memory-mapped I/O
- End-to-end systems toolchain design

---

## Motivation

This project was built to gain a deeper understanding of how low-level software interacts with hardware, and how assemblers, linkers, loaders, and processors work together internally. It reflects a strong interest in systems programming, computer architecture, and hardware-adjacent software development.

---

## Technologies Used

- **C / C++**
- **Linux (amd64)**
- **Makefile**
- Custom binary and object file formats
