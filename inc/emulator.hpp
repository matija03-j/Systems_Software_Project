#pragma once
#include <unordered_map>

struct Cpu {
  uint32_t r[16]{};
  uint32_t csr[3] = {0, 0, 0}; // status, handler, cause
  bool halted{false};
};

class Emulator{
public:

  void writeMemory(const std::string& input);

  void run();

  void write();

private:
  std::unordered_map<uint32_t, uint8_t> memory;
  Cpu cpu;

  uint32_t read4Bytes(uint32_t addr);
  uint32_t getInstruction32();
  uint32_t readFromAdress(uint32_t addr);
  void writeAtAddress(uint32_t addr, uint32_t value);

  void handleCall(uint32_t inst);
  void handleJump(uint32_t inst);
  void handleXchg(uint32_t inst);
  void handleArit(uint32_t inst);
  void handleLogic(uint32_t inst);
  void handleShift(uint32_t inst);
  void handleStore(uint32_t inst);
  void handleLoad(uint32_t inst);
};
