// emu/main.cpp
#include "../../inc/emulator.hpp"
#include <iostream>

int main(int argc, char** argv) {
  if (argc<2) {
    std::cerr << "Usage: emu <out.hex>\n";
    return 1;
  }
  std::string path = argv[1];

  Emulator emu;
  emu.writeMemory(path);
  emu.run();

  emu.write();
  return 0;
}
