#include "../../inc/emulator.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iostream>

#include <vector>
#include <algorithm>
#include <iomanip>

#include "../../inc/asmInfo.hpp"

void Emulator::writeMemory(const std::string &input){
  std::ifstream in(input, std::ios::binary);
  if (!in) throw std::runtime_error("readFile: cannot open " + input);

  std::string line;
  while (std::getline(in, line)) {
    auto pos = line.find(':');

    std::string addrHex = line.substr(0, pos);
    
    while (!addrHex.empty() && std::isspace((unsigned char)addrHex.back())) addrHex.pop_back();
    if (addrHex.empty()) continue;

    uint32_t base = std::stoul(addrHex, nullptr, 16);

    // posle dvotačke idu bajtovi u hexu, odvojeni razmacima
    std::istringstream iss(line.substr(pos + 1));
    for (int i = 0; i < 8; ++i) {
      std::string b;
      if (!(iss >> b)) break;
      uint8_t val = static_cast<uint8_t>(std::stoul(b, nullptr, 16));
      memory[base + static_cast<uint32_t>(i)] = val;
    }
  }
  
}

uint32_t Emulator::read4Bytes(uint32_t addr)
{
  uint32_t b0 = memory[addr];
  uint32_t b1 = memory[addr + 1];
  uint32_t b2 = memory[addr + 2];
  uint32_t b3 = memory[addr + 3];

  return (b0) | (b1<<8) | (b2<<16) | (b3<<24);
}

uint32_t Emulator::getInstruction32()
{
  uint32_t inst = read4Bytes(cpu.r[15]);
  cpu.r[15] = cpu.r[15] + 4;
  return inst;
}

void Emulator::run() {
  cpu.r[15] = PC_start;
  cpu.r[14] = SP_start;

  cpu.halted = false;

  while (!cpu.halted) {
    uint32_t inst = getInstruction32();
    uint32_t OC   = (inst >> 28) & 0xF;
    // uint32_t temp = cpu.r[15] - 4;
    // std::cout << std::hex << temp << " : " << inst << '\n';

    switch (OC) {
      case HALT_OC:
        cpu.halted = true;
        break;

      case CALL_OC:
        handleCall(inst);
        break;

      case JUMP_OC:
        handleJump(inst);
        break;

      case XCHG_OC:
        handleXchg(inst);
        break;

      case ARIT_OC:
        handleArit(inst);
        break;

      case LOGIC_OC:
        handleLogic(inst);
        break;

      case SHIFT_OC:
        handleShift(inst);
        break;

      case STORE_OC:
        handleStore(inst);
        break;

      case LOAD_OC:
        handleLoad(inst);
        break;

      case INT_OC:
        cpu.r[SP_REG] =  cpu.r[SP_REG] - 4;
        writeAtAddress(cpu.r[SP_REG], cpu.csr[0]);
        cpu.r[SP_REG] = cpu.r[SP_REG] - 4;
        writeAtAddress(cpu.r[SP_REG], cpu.r[PC_REG]);

        cpu.csr[2] = 4;
        cpu.csr[0] = cpu.csr[0] & (~0x01);
        //std::cout << std::hex << cpu.csr[2] << '\n';
        cpu.r[PC_REG] = cpu.csr[1]; 
        break;

      default:
        throw std::runtime_error("run: unknown OC");
    }

    cpu.r[0] = 0;
  }
}

uint32_t Emulator::readFromAdress(uint32_t addr)
{
  uint32_t value;
  value = read4Bytes(addr);
  return value;
}

void Emulator::writeAtAddress(uint32_t addr, uint32_t value){
  uint8_t b0 = static_cast<uint8_t>(value & 0xFF);
  uint8_t b1 = static_cast<uint8_t>((value >> 8) & 0xFF);
  uint8_t b2 = static_cast<uint8_t>((value >> 16) & 0xFF);
  uint8_t b3 = static_cast<uint8_t>((value >> 24) & 0xFF);

  memory[addr] = b0;
  memory[addr + 1] = b1;
  memory[addr + 2] = b2;
  memory[addr + 3] = b3;
}

void Emulator::handleCall(uint32_t inst)
{
  uint32_t MOD = (inst >> 24) & 0xF;
  uint32_t A   = (inst >> 20) & 0xF;
  uint32_t B   = (inst >> 16) & 0xF;
  //uint32_t C   = (inst >> 12) & 0xF;
  int32_t  D   = int32_t(inst & 0xFFF);

  cpu.r[SP_REG] = cpu.r[SP_REG] - 4;
  writeAtAddress(cpu.r[SP_REG], cpu.r[PC_REG]);
  
  if(MOD == CALL_MOD0){
    cpu.r[PC_REG] = cpu.r[A] + cpu.r[B] + D;
  }else if(MOD == CALL_MOD1){
    cpu.r[PC_REG] = readFromAdress(cpu.r[A] + cpu.r[B] + D);
  }
}

void Emulator::handleJump(uint32_t inst){
  uint32_t MOD = (inst >> 24) & 0xF;
  uint32_t A   = (inst >> 20) & 0xF;
  uint32_t B   = (inst >> 16) & 0xF;
  uint32_t C   = (inst >> 12) & 0xF;
  int32_t  D   = int32_t(inst & 0xFFF);

  int val = 0;
  val = (D & 0x800) ? (int)D - 0x1000 : (int)D;

  switch (MOD)
  {
  case JMP_MOD0:
    cpu.r[15] = cpu.r[A] + val;
    break;

  case JMP_MOD1:
    if(cpu.r[B] == cpu.r[C]){
      cpu.r[15] = cpu.r[A] + val;
    }
    break;
  
  case JMP_MOD2:
    if(cpu.r[B] != cpu.r[C]){
      cpu.r[15] = cpu.r[A] + val;
    }
    break;

  case JMP_MOD3:
    if((int)cpu.r[B] > (int)cpu.r[C]){
      cpu.r[15] = cpu.r[A] + val;
    }
    break;

  case JMP_MOD4:
    cpu.r[15] = readFromAdress(cpu.r[A] + val);
    break;
  
  case JMP_MOD5:
    if(cpu.r[B] == cpu.r[C]){
      cpu.r[15] = readFromAdress(cpu.r[A] + val);
    }
    break;

  case JMP_MOD6:
    if(cpu.r[B] != cpu.r[C]){
      cpu.r[15] = readFromAdress(cpu.r[A] + val);
    }
    break;
  
  case JMP_MOD7:
    if((int)cpu.r[B] > (int)cpu.r[C]){
      cpu.r[15] = readFromAdress(cpu.r[A] + val);
    }
    break;
  
  default:
    break;
  }
}

void Emulator::handleXchg(uint32_t inst){
  //uint32_t MOD = (inst >> 24) & 0xF;
  //uint32_t A   = (inst >> 20) & 0xF;
  uint32_t B   = (inst >> 16) & 0xF;
  uint32_t C   = (inst >> 12) & 0xF;
  //uint32_t  D   = int32_t(inst & 0xFFF);

  std::swap(cpu.r[B], cpu.r[C]);
}

void Emulator::handleArit(uint32_t inst){
  uint32_t MOD = (inst >> 24) & 0xF;
  uint32_t A   = (inst >> 20) & 0xF;
  uint32_t B   = (inst >> 16) & 0xF;
  uint32_t C   = (inst >> 12) & 0xF;
  //int32_t  D   = int32_t(inst & 0xFFF);

  switch (MOD)
  {
  case ADD_MOD:
    cpu.r[A] = cpu.r[B] + cpu.r[C];
    break;

  case SUB_MOD:
    cpu.r[A] = cpu.r[B] - cpu.r[C];
    break;
  
  case MUL_MOD:
    cpu.r[A] = cpu.r[B] * cpu.r[C];
    break;
  
  case DIV_MOD:
    cpu.r[A] = cpu.r[B] / cpu.r[C];
    break;
  
  default:
    break;
  }
}

void Emulator::handleLogic(uint32_t inst){
  uint32_t MOD = (inst >> 24) & 0xF;
  uint32_t A   = (inst >> 20) & 0xF;
  uint32_t B   = (inst >> 16) & 0xF;
  uint32_t C   = (inst >> 12) & 0xF;
  //int32_t  D   = int32_t(inst & 0xFFF);

  switch (MOD)
  {
  case AND_MOD:
    cpu.r[A] = cpu.r[B] & cpu.r[C];
    break;

  case NOT_MOD:
    cpu.r[A] = ~cpu.r[B];
    break;
  
  case OR_MOD:
    cpu.r[A] = cpu.r[B] | cpu.r[C];
    break;
  
  case XOR_MOD:
    cpu.r[A] = cpu.r[B] ^ cpu.r[C];
    break;
  
  default:
    break;
  }
}

void Emulator::handleShift(uint32_t inst){
  uint32_t MOD = (inst >> 24) & 0xF;
  uint32_t A   = (inst >> 20) & 0xF;
  uint32_t B   = (inst >> 16) & 0xF;
  uint32_t C   = (inst >> 12) & 0xF;
  //int32_t  D   = int32_t(inst & 0xFFF);

  if(MOD == SHL_MOD){
    cpu.r[A] = cpu.r[B] << cpu.r[C];
  }else if(MOD == SHR_MOD){
    cpu.r[A] = cpu.r[B] >> cpu.r[C];
  }
}

void Emulator::handleStore(uint32_t inst){
  uint32_t MOD = (inst >> 24) & 0xF;
  uint32_t A   = (inst >> 20) & 0xF;
  uint32_t B   = (inst >> 16) & 0xF;
  uint32_t C   = (inst >> 12) & 0xF;
  int32_t  D   = int32_t(inst & 0xFFF);

  int val = 0;
  val = (D & 0x800) ? (int)D - 0x1000 : (int)D;

  switch (MOD)
  {
  case STORE_MOD0:
    writeAtAddress(cpu.r[A] + cpu.r[B] + val, cpu.r[C]);
    break;

  case STORE_MOD1:
    cpu.r[A] = cpu.r[A] + val;
    writeAtAddress(cpu.r[A], cpu.r[C]);
    break;
  
  case STORE_MOD2:
    writeAtAddress(readFromAdress(cpu.r[A] + cpu.r[B] + D), cpu.r[C]);
    break;
  
  default:
    break;
  }
}

void Emulator::handleLoad(uint32_t inst){
  uint32_t MOD = (inst >> 24) & 0xF;
  uint32_t A   = (inst >> 20) & 0xF;
  uint32_t B   = (inst >> 16) & 0xF;
  uint32_t C   = (inst >> 12) & 0xF;
  int32_t  D   = int32_t(inst & 0xFFF);

  int val = 0;
  val = (D & 0x800) ? (int)D - 0x1000 : (int)D;

  switch (MOD)
  {
  case LOAD_MOD0:
    cpu.r[A] = cpu.csr[B];
    break;

  case LOAD_MOD1:
    cpu.r[A] = cpu.r[B] + val;
    break;
  
  case LOAD_MOD2:
    cpu.r[A] = readFromAdress(cpu.r[B] + cpu.r[C] + val);
    break;

  case LOAD_MOD3:
    cpu.r[A] = readFromAdress(cpu.r[B]);
    cpu.r[B] = cpu.r[B] + D;
    break;

  case LOAD_MOD4:
    cpu.csr[A] = cpu.r[B];
    break;
  
  case LOAD_MOD5:
    cpu.csr[A] = cpu.csr[B] | val;
    break;

  case LOAD_MOD6:
    cpu.csr[A] = readFromAdress(cpu.r[B] + cpu.r[C] + val);
    break;
  
  case LOAD_MOD7:
    cpu.csr[A] = readFromAdress(cpu.r[B]);
    cpu.r[B] = cpu.r[B] + val;
    break;
  
  default:
    break;
  }
}


void Emulator::write()
{
  std::cout << "-----------------------------------------------------------------" << std::endl;
  std::cout << "Emulated processor executed halt instruction" << std::endl;
  std::cout << "Emulated processor state:" << std::endl;

  int i = 0;
  for (const auto &reg : cpu.r)
  {
    std::cout << (i < 10 ? " " : "") << "r" << std::dec << i << "=0x" << std::setw(8) << std::setfill('0') << std::hex << reg << " ";
    if (++i % 4 == 0)
      std::cout << std::endl;
  }
}
