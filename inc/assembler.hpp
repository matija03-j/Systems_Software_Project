#pragma once
#include "asmInfo.hpp"
#include "structs.hpp"
#include "rw.hpp"
#include <cstdint>

class Assembler {
public:
  void declareDirective(const std::string& directive, const std::string& name, uint32_t imm);

  void defineLabel(const std::string& name);

  void emit32(uint32_t w);
  void end();

  void instHalt();
  void instInt();
  void instIret();
  void instCall(uint32_t);
  void instCall(std::string);
  void instRet();
  void instJmp(uint32_t);
  void instJmp(std::string);
  void instBeq(int, int, uint32_t);
  void instBeq(int, int, std::string);
  void instBne(int, int, uint32_t);
  void instBne(int, int, std::string);
  void instBgt(int, int, uint32_t);
  void instBgt(int, int, std::string);
  void instPush(int);
  void instPop(int);
  void instXchg(int, int);
  void instAdd(int, int);
  void instSub(int, int);
  void instMul(int, int);
  void instDiv(int, int);
  void instNot(int);
  void instAnd(int, int);
  void instOr(int, int);
  void instXor(int, int);
  void instShl(int, int);
  void instShr(int, int);
  void instLdImm(int, int);
  void instLdImm(std::string, int);
  void instLdRegDir(int, int);
  void instLdRegInd(int, int);
  void instLdRegIndOff(int, int, int);
  void instLdMemDir(uint32_t, int);
  void instLdMemDir(std::string, int);
  void instStRegInd(int, int);
  void instStRegIndOff(int, int, int);
  void instStMemDir(int, uint32_t);
  void instStMemDir(int, std::string);
  void instCsrrd(int, int);
  void instCsrwr(int, int);

  void write_object(const std::string& path) const;

  void write_object_binary(const std::string& path) const;

private:
  Rw rw;

  std::unordered_map<std::string, Symbol> symbols;
  std::unordered_map<std::string, Section> sections;
  std::string currSection;
};
