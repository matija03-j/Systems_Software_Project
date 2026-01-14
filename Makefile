# ---- Toolchains & flags ----
CXX       := g++
CXXFLAGS  := -std=c++17 -O2 -Wall -Wextra
BISON     := bison
FLEX      := flex

# Ako treba dodatni include (kao u komandi): -Iassembler
ASM_INC   := -Iassembler

# ---- Targets ----
.PHONY: all clean

all: asembler linker emulator

# --- Assembler (zavisi od bison/flex izlaza) ---
asembler: parser.tab.c lex.yy.c src/asembler/driver.cpp src/asembler/assembler.cpp src/rw.cpp
	$(CXX) $(CXXFLAGS) $(ASM_INC) src/asembler/driver.cpp src/asembler/assembler.cpp src/rw.cpp parser.tab.c lex.yy.c -lfl -o $@

# Bison: generiše parser.tab.c i parser.tab.h iz grammar fajla
parser.tab.c parser.tab.h: misc/parser.y
	$(BISON) -d -o parser.tab.c $<

# Flex: generiše lex.yy.c; tipično je dobro da zavisi i od parser.tab.h
lex.yy.c: misc/lexer.l parser.tab.h
	$(FLEX) -o $@ $<

# --- Linker ---
linker: src/linker/linker.cpp src/linker/driver.cpp src/rw.cpp
	$(CXX) $(CXXFLAGS) src/linker/linker.cpp src/linker/driver.cpp src/rw.cpp -o $@

# --- Emulator ---
emulator: src/emulator/emulator.cpp src/emulator/mainEmu.cpp
	$(CXX) $(CXXFLAGS) src/emulator/emulator.cpp src/emulator/mainEmu.cpp -o $@

# --- Čišćenje ---
clean:
	rm -f asembler linker emulator parser.tab.c parser.tab.h lex.yy.c
