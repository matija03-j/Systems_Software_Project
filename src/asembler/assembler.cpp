#include "../../inc/assembler.hpp"
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <iomanip>

void Assembler::declareDirective(const std::string& directive, const std::string& name, uint32_t imm) {
  if(directive == "global"){
    if(symbols.find(name) == symbols.end()){
      auto& s = symbols[name];
      s.is_global = true;
      s.is_defined = false;
      s.section = "UND";
      s.offset = 0;
    }
  }else if(directive == "extern"){
    if(symbols.find(name) == symbols.end()){
      auto& s = symbols[name];
      s.is_global = true;
      s.is_defined = false;
      s.section = "UND";
      s.offset = 0;
    }
  }else if(directive == "section"){
    if(sections.find(name) == sections.end()){
      auto& s = sections[name];
      s.name = name;
      s.addr = 0;

      auto& sym = symbols["." + name];
      sym.is_defined = true;
      sym.is_global = false;
      sym.section = name;
      sym.offset = 0;
    }
    currSection = name;
  }else if(directive == "word"){
    if(name.empty()) { // .word imm
      sections[currSection].data.push_back(imm & 0xff);
      sections[currSection].data.push_back((imm >> 8) & 0xff);
      sections[currSection].data.push_back((imm >> 16) & 0xff);
      sections[currSection].data.push_back((imm >> 24) & 0xff);
    } else { // do reloc, it's easier
      // check if the symbol exists in symbolTable
      if(symbols.find(name) == symbols.end()) {
        auto& s = symbols[name];
        s.section = "UND";
        s.offset  = 0;
      }
      uint32_t offset = sections[currSection].data.size();
      sections[currSection].relocs.push_back(Reloc(name, offset));
      sections[currSection].data.push_back(0x00);
      sections[currSection].data.push_back(0x00);
      sections[currSection].data.push_back(0x00);
      sections[currSection].data.push_back(0x00);
    }
  }else if(directive == "skip"){
    for(uint32_t i = 0; i < imm; ++i) {
      sections[currSection].data.push_back(0x00);
    }
  }
}

void Assembler::defineLabel(const std::string& name){
  auto& sec = sections[currSection];
  auto& sym = symbols[name];
  if (sym.is_defined) { throw std::runtime_error("redefined label: " + name); }
  sym.is_defined = true;
  sym.section = currSection;
  sym.offset = static_cast<uint32_t>(sec.data.size());
}

void Assembler::end(){
  for(auto& sec: sections){
    for(uint32_t i = 0; i < sec.second.relocs.size(); i++){
      Symbol s = symbols[sec.second.relocs[i].symbol];
      if(s.is_defined && !s.is_global){
        sec.second.relocs[i].symbol = sec.second.name;
        sec.second.relocs[i].addend = s.offset;
      }
    }
  }
}

void Assembler::emit32(uint32_t w) {
  auto& sec = sections[currSection];
  sec.data.push_back(uint8_t( w & 0xFF));
  sec.data.push_back(uint8_t((w >> 8)  & 0xFF));
  sec.data.push_back(uint8_t((w >> 16) & 0xFF));
  sec.data.push_back(uint8_t((w >> 24) & 0xFF));
}

void Assembler::instHalt()
{
  uint32_t word = HALT_OC << 28;
  emit32(word);
}

void Assembler::instInt(){
  const uint32_t word = (uint32_t(INT_OC) << 28);
  emit32(word);
}

void Assembler::instIret(){
  uint32_t word1 =
      (LOAD_OC << 28) |
      (LOAD_MOD6 << 24) |
      ((STATUS & 0xF)<< 20) |
      ((SP_REG & 0xF) << 16) |
      (0x0 << 12) |
      4u;
  emit32(word1);

  uint32_t word2 =
      (LOAD_OC << 28) |
      (LOAD_MOD3 << 24) |
      ((PC_REG & 0xF)<< 20) |
      ((SP_REG & 0xF) << 16) |
      (0x0 << 12) |
      8u;
  emit32(word2);
}

void Assembler::instCall(uint32_t literal){
  if (literal <= 2047) {
    const uint32_t disp12 = (uint32_t(literal) & 0x0FFF);
    uint32_t w = ((CALL_OC << 28)
             | (CALL_MOD0 << 24)
             | (0x0 << 20)
             | (0x0 << 16)
             | (0x0 << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = (CALL_OC << 28)
             | (CALL_MOD1 << 24)
             | ((PC_REG & 0xF)  << 20)
             | (0x0 << 16)
             | (0x0 << 12)
             | 0x004;
    emit32(w);
    instJmp(4);
    emit32(literal);
  }
}

void Assembler::instCall(std::string sym){
  int d = symbols[sym].offset - sections[currSection].data.size() - 4;
  if(currSection == symbols[sym].section && d <= 2047 && d >= -2048){
    const uint32_t disp12 = (uint32_t(d) & 0x0FFF);
    uint32_t w = ((CALL_OC << 28)
             | (CALL_MOD0 << 24)
             | (PC_REG << 20)
             | (0x0 << 16)
             | (0x0 << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = (CALL_OC << 28)
             | (CALL_MOD1 << 24)
             | ((PC_REG & 0xF)  << 20)
             | (0x0 << 16)
             | (0x0 << 12)
             | 0x004;
    emit32(w);
    instJmp(4);
    sections[currSection].relocs.push_back(Reloc{sym, static_cast<int>(sections[currSection].data.size())});
    emit32(0u);
  }
}

void Assembler::instRet(){
  uint32_t word =
      (LOAD_OC << 28) |
      (LOAD_MOD3 << 24) |
      ((PC_REG & 0xF)<< 20) |
      ((14 & 0xF) << 16) |
      (0x0 << 12) |
      4u;
  emit32(word);
}

void Assembler::instJmp(uint32_t literal){
  if (literal <= 2047) {
    //branch(JUMP_OC, JMP_MOD0, PC_REG, 0, 0, literal);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD0 << 24)
             | (PC_REG << 20)
             | (0x0 << 16)
             | (0x0 << 12)
             | (literal & 0xFFF));
    emit32(w);
  }else{
    //branchPool(JUMP_OC, JMP_MOD4, PC_REG, 0, 0, 0, nullptr, (uint32_t)literal);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD4 << 24)
             | (PC_REG << 20)
             | (0x0 << 16)
             | (0x0 << 12)
             | 0u);
    emit32(w);
    emit32(literal);
  }
}

void Assembler::instJmp(std::string sym){
  //branchPool(JUMP_OC, JMP_MOD4, PC_REG, 0, 0, 0, &sym, 0);
  int d = (int)symbols[sym].offset - static_cast<int>(sections[currSection].data.size()) - 4;
  if(currSection == symbols[sym].section && d <= 2047 && d >= -2048){
    const uint32_t disp12 = (uint32_t(d) & 0x0FFF);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD0 << 24)
             | (PC_REG << 20)
             | (0x0 << 16)
             | (0x0 << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD4 << 24)
             | (PC_REG << 20)
             | (0x0 << 16)
             | (0x0 << 12)
             | 0u);
    emit32(w);
    sections[currSection].relocs.push_back(Reloc{sym, static_cast<int>(sections[currSection].data.size())});
    emit32(0u);
  }
}

void Assembler::instBeq(int gpr1, int gpr2, uint32_t literal){
  if (literal <= 2047) {
    //branch(JUMP_OC, JMP_MOD1, PC_REG, gpr1, gpr2, literal);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD1 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | (literal & 0xFFF));
    emit32(w);
  }else{
    //branchPool(JUMP_OC, JMP_MOD5, PC_REG, gpr1, gpr2, 0, nullptr, (uint32_t)literal);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD5 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2<< 12)
             | 0x004);
    emit32(w);
    instJmp(4);
    emit32(literal);
  }
}

void Assembler::instBeq(int gpr1, int gpr2, std::string sym){
  //branchPool(JUMP_OC, JMP_MOD5, PC_REG, gpr1, gpr2, 0, &sym, 0);
  int d = symbols[sym].offset - static_cast<int>(sections[currSection].data.size()) - 4;
  if(currSection == symbols[sym].section && d <= 2047 && d >= -2048){
    const uint32_t disp12 = (uint32_t(d) & 0x0FFF);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD1 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD5 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | 0x004);
    emit32(w);
    instJmp(4);
    sections[currSection].relocs.push_back(Reloc{sym, static_cast<int>(sections[currSection].data.size())});
    emit32(0u);
  }
}

void Assembler::instBne(int gpr1, int gpr2, uint32_t literal){
  if (literal <= 2047) {
    //branch(JUMP_OC, JMP_MOD2, PC_REG, gpr1, gpr2, literal);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD2 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | (literal & 0xFFF));
    emit32(w);
  }else{
    //branchPool(JUMP_OC, JMP_MOD6, PC_REG, gpr1, gpr2, 0, nullptr, (uint32_t)literal);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD6 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2<< 12)
             | 0x004);
    emit32(w);
    instJmp(4);
    emit32(literal);
  }
}

void Assembler::instBne(int gpr1, int gpr2, std::string sym){
  //branchPool(JUMP_OC, JMP_MOD6, PC_REG, gpr1, gpr2, 0, &sym, 0);
  int d = symbols[sym].offset - static_cast<int>(sections[currSection].data.size()) - 4;
  if(currSection == symbols[sym].section && d <= 2047 && d >= -2048){
    const uint32_t disp12 = (uint32_t(d) & 0x0FFF);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD2 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD6 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | 0x004);
    emit32(w);
    instJmp(4);
    sections[currSection].relocs.push_back(Reloc{sym, static_cast<int>(sections[currSection].data.size())});
    emit32(0u);
  }
}

void Assembler::instBgt(int gpr1, int gpr2, uint32_t literal){
  if (literal <= 2047) {
    //branch(JUMP_OC, JMP_MOD3, PC_REG, gpr1, gpr2, literal);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD3 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | (literal & 0xFFF));
    emit32(w);
  }else{
    //branchPool(JUMP_OC, JMP_MOD7, PC_REG, gpr1, gpr2, 0, nullptr, (uint32_t)literal);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD7 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2<< 12)
             | 0x004);
    emit32(w);
    instJmp(4);
    emit32(literal);
  }
}

void Assembler::instBgt(int gpr1, int gpr2, std::string sym){
  //branchPool(JUMP_OC, JMP_MOD7, PC_REG, gpr1, gpr2, 0, &sym, 0);
  int d = symbols[sym].offset - static_cast<int>(sections[currSection].data.size()) - 4;
  if(currSection == symbols[sym].section && d <= 2047 && d >= -2048){
    const uint32_t disp12 = (uint32_t(d) & 0x0FFF);
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD3 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = ((JUMP_OC << 28)
             | (JMP_MOD7 << 24)
             | (PC_REG << 20)
             | (gpr1 << 16)
             | (gpr2 << 12)
             | 0x004);
    emit32(w);
    instJmp(4);
    sections[currSection].relocs.push_back(Reloc{sym, static_cast<int>(sections[currSection].data.size())});
    emit32(0u);
  }
}

void Assembler::instPush(int gpr){
  uint32_t word =
      (STORE_OC<< 28) |
      (STORE_MOD1 << 24) |
      ((14 & 0xF)<< 20) |
      (0x0 << 16) |
      ((uint32_t(gpr) & 0xF)<< 12) |
      (-4 & 0xFFF);
  emit32(word);
}

void Assembler::instPop(int gpr){
  uint32_t word =
      (LOAD_OC << 28) |
      (LOAD_MOD3 << 24) |
      ((uint32_t(gpr) & 0xF)<< 20) |
      ((14 & 0xF) << 16) |
      (0x0 << 12) |
      4u;
  emit32(word);
}

void Assembler::instXchg(int gpr1, int gpr2){
  uint32_t word =
      (XCHG_OC << 28) |
      (0x0 << 24) |
      (0x0 << 20) |
      ((uint32_t(gpr1) & 0xF) << 16) |
      ((uint32_t(gpr2) & 0xF) << 12) |
      0u;
  emit32(word);
}

void Assembler::instAdd(int rs, int rd) {
  uint32_t word =
      (ARIT_OC << 28) |   // OC [31:28]
      (ADD_MOD << 24) |   // MOD [27:24]
      ((uint32_t(rd) & 0xF) << 20) |  // A [23:20]
      ((uint32_t(rd) & 0xF) << 16) |  // B [19:16]
      ((uint32_t(rs) & 0xF) << 12) |  // C [15:12]
      0u;                             
  emit32(word);
}

void Assembler::instSub(int rs, int rd){
  uint32_t word =
      (ARIT_OC << 28) |   // OC [31:28]
      (SUB_MOD << 24) |   // MOD [27:24]
      ((uint32_t(rd) & 0xF) << 20) |  // A [23:20]
      ((uint32_t(rd) & 0xF) << 16) |  // B [19:16]
      ((uint32_t(rs) & 0xF) << 12) |  // C [15:12]
      0u;                              // disp [11:0] = 0 za aritmetiku
  emit32(word);
}

void Assembler::instMul(int rs, int rd){
  uint32_t word =
      (ARIT_OC << 28) |   // OC [31:28]
      (MUL_MOD << 24) |   // MOD [27:24]
      ((uint32_t(rd) & 0xF) << 20) |  // A [23:20]
      ((uint32_t(rd) & 0xF) << 16) |  // B [19:16]
      ((uint32_t(rs) & 0xF) << 12) |  // C [15:12]
      0u;                              // disp [11:0] = 0 za aritmetiku
  emit32(word);
}

void Assembler::instDiv(int rs, int rd){
  uint32_t word =
      (ARIT_OC << 28) |   // OC [31:28]
      (DIV_MOD << 24) |   // MOD [27:24]
      ((uint32_t(rd) & 0xF) << 20) |  // A [23:20]
      ((uint32_t(rd) & 0xF) << 16) |  // B [19:16]
      ((uint32_t(rs) & 0xF) << 12) |  // C [15:12]
      0u;                              // disp [11:0] = 0 za aritmetiku
  emit32(word);
}

void Assembler::instNot(int r){
  uint32_t word =
      (LOGIC_OC << 28) |
      (NOT_MOD << 24) |
      ((uint32_t(r) & 0xF) << 20) |
      ((uint32_t(r) & 0xF) << 16) |
      ((uint32_t(r) & 0xF) << 12) |
      0u;
  emit32(word);
}

void Assembler::instAnd(int rs, int rd){
  uint32_t word =
      (LOGIC_OC << 28) |
      (AND_MOD << 24) |
      ((uint32_t(rd) & 0xF) << 20) |
      ((uint32_t(rd) & 0xF) << 16) |
      ((uint32_t(rs) & 0xF) << 12) |
      0u;
  emit32(word);
}

void Assembler::instOr(int rs, int rd){
  uint32_t word =
      (LOGIC_OC << 28) |
      (OR_MOD << 24) |
      ((uint32_t(rd) & 0xF) << 20) |
      ((uint32_t(rd) & 0xF) << 16) |
      ((uint32_t(rs) & 0xF) << 12) |
      0u;
  emit32(word);
}

void Assembler::instXor(int rs, int rd){
  uint32_t word =
      (LOGIC_OC << 28) |
      (XOR_MOD << 24) |
      ((uint32_t(rd) & 0xF) << 20) |
      ((uint32_t(rd) & 0xF) << 16) |
      ((uint32_t(rs) & 0xF) << 12) |
      0u;
  emit32(word);
}

void Assembler::instShl(int rs, int rd){
  uint32_t word =
      (SHIFT_OC << 28) |
      (SHL_MOD << 24) |
      ((uint32_t(rd) & 0xF) << 20) |
      ((uint32_t(rd) & 0xF) << 16) |
      ((uint32_t(rs) & 0xF) << 12) |
      0u;
  emit32(word);
}

void Assembler::instShr(int rs, int rd){
  uint32_t word =
      (SHIFT_OC << 28) |
      (SHR_MOD << 24) |
      ((uint32_t(rd) & 0xF) << 20) |
      ((uint32_t(rd) & 0xF) << 16) |
      ((uint32_t(rs) & 0xF) << 12) |
      0u;
  emit32(word);
}

void Assembler::instLdImm(int literal, int gpr){
  if (literal >= -2048 && literal <= 2047) {
    //branch(LOAD_OC, LOAD_MOD1, gpr, 0, 0, literal);
    uint32_t w = ((LOAD_OC << 28)
             | (LOAD_MOD1 << 24)
             | (gpr << 20)
             | (0 << 16)
             | (0 << 12)
             | (literal & 0xFFF));
    emit32(w);
  }else{
    //branchPool(LOAD_OC, LOAD_MOD3, gpr, PC_REG, 0, 4, nullptr, literal);
    uint32_t w = ((LOAD_OC << 28)
             | (LOAD_MOD3 << 24)
             | (gpr << 20)
             | (PC_REG << 16)
             | (0 << 12)
             | 0x004);
    emit32(w);
    emit32(literal);
  }
}

void Assembler::instLdImm(std::string sym, int gpr){
  //branchPool(LOAD_OC, LOAD_MOD3, gpr, PC_REG, 0, 4, &sym, 0);
  int d = symbols[sym].offset - sections[currSection].data.size() - 4;
  if(currSection == symbols[sym].section && d <= 2047 && d >= -2048){
    const uint32_t disp12 = (uint32_t(d) & 0x0FFF);
    uint32_t w = ((LOAD_OC << 28)
             | (LOAD_MOD1 << 24)
             | (gpr << 20)
             | (PC_REG << 16)
             | (0 << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = ((LOAD_OC << 28)
             | (LOAD_MOD3 << 24)
             | (gpr << 20)
             | (PC_REG << 16)
             | (0 << 12)
             | 0x004);
    emit32(w);
    sections[currSection].relocs.push_back(Reloc{sym, static_cast<int>(sections[currSection].data.size())});
    emit32(0u);
  }
}

void Assembler::instLdRegDir(int gpr2, int gpr1){
  uint32_t word =
      (LOAD_OC << 28) |
      (LOAD_MOD1 << 24) |
      ((uint32_t(gpr1) & 0xF) << 20) |
      ((uint32_t(gpr2) & 0xF) << 16) |
      (0 << 12) |
      0u;
  emit32(word);
}

void Assembler::instLdRegInd(int gpr2, int gpr1){
  uint32_t word =
      (LOAD_OC << 28) |
      (LOAD_MOD2 << 24) |
      ((uint32_t(gpr1) & 0xF) << 20) |
      ((uint32_t(gpr2) & 0xF) << 16) |
      (0 << 12) |
      0u;
  emit32(word);
}

void Assembler::instLdRegIndOff(int gpr2, int literal, int gpr1){
  if(literal >= -2048 && literal <= 2047){
    uint32_t word =
      (LOAD_OC << 28) |
      (LOAD_MOD2 << 24) |
      ((uint32_t(gpr1) & 0xF) << 20) |
      ((uint32_t(gpr2) & 0xF) << 16) |
      (0 << 12) |
      (literal & 0xFFF);
    emit32(word);
  }else{
    std::cout << "ERROR | Literal can't fit into 12b" << std::endl;
    exit(-1);
  }
}

void Assembler::instLdMemDir(uint32_t literal, int gpr){
  //branchPool(LOAD_OC, LOAD_MOD3, gpr, PC_REG, 0, 4, nullptr, literal);
  //instLdRegInd(gpr, gpr);

  if (literal <= 2047) {
    uint32_t w = ((LOAD_OC << 28)
             | (LOAD_MOD2 << 24)
             | (gpr << 20)
             | (0 << 16)
             | (0 << 12)
             | (literal & 0xFFF));
    emit32(w);
  }else{
    //branchPool(LOAD_OC, LOAD_MOD3, gpr, PC_REG, 0, 4, nullptr, literal);
    uint32_t w = ((LOAD_OC << 28)
             | (LOAD_MOD3 << 24)
             | (gpr << 20)
             | (PC_REG << 16)
             | (0 << 12)
             | 0x004);
    emit32(w);
    emit32(literal);
    instLdRegInd(gpr, gpr);
  }
}

void Assembler::instLdMemDir(std::string sym, int gpr){
  //branchPool(LOAD_OC, LOAD_MOD3, gpr, PC_REG, 0, 4, &sym, 0);
  //instLdRegInd(gpr, gpr);

  int d = symbols[sym].offset - sections[currSection].data.size() - 4;
  if(currSection == symbols[sym].section && d <= 2047 && d >= -2048){
    const uint32_t disp12 = (uint32_t(d) & 0x0FFF);
    uint32_t w = ((LOAD_OC << 28)
             | (LOAD_MOD2 << 24)
             | (gpr << 20)
             | (PC_REG << 16)
             | (0 << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = ((LOAD_OC << 28)
             | (LOAD_MOD3 << 24)
             | (gpr << 20)
             | (PC_REG << 16)
             | (0 << 12)
             | 0x004);
    emit32(w);
    sections[currSection].relocs.push_back(Reloc{sym, static_cast<int>(sections[currSection].data.size())});
    emit32(0u);
    instLdRegInd(gpr, gpr);
  }
}

void Assembler::instStRegInd(int gpr2, int gpr1){
  uint32_t word =
      (STORE_OC << 28) |
      (STORE_MOD0 << 24) |
      ((uint32_t(gpr1) & 0xF) << 20) |
      (0 << 16) |
      ((uint32_t(gpr2) & 0xF) << 12) |
      0u;
  emit32(word);
}

void Assembler::instStRegIndOff(int gpr2, int gpr1, int literal){
  if(literal >= -2048 && literal <= 2047){
    uint32_t word =
      (STORE_OC << 28) |
      (STORE_MOD0 << 24) |
      ((uint32_t(gpr1) & 0xF) << 20) |
      (0 << 16) |
      ((uint32_t(gpr2) & 0xF) << 12) |
      (literal & 0xFFF);
  emit32(word);
  }else{
    std::cout << "ERROR | Literal can't fit into 12b" << std::endl;
    exit(-1);
  }
}

void Assembler::instStMemDir(int gpr, uint32_t literal){
  if (literal <= 2047) {
    //branch(STORE_OC, STORE_MOD0, 0, 0, gpr, literal);
    uint32_t w = ((STORE_OC << 28)
             | (STORE_MOD0 << 24)
             | (0 << 20)
             | (0 << 16)
             | (gpr << 12)
             | (literal & 0xFFF));
    emit32(w);
  }else{
    //branch(STORE_OC, STORE_MOD2, PC_REG, 0, gpr, 4);
    uint32_t w = ((STORE_OC << 28)
             | (STORE_MOD2 << 24)
             | (PC_REG << 20)
             | (0 << 16)
             | (gpr << 12)
             | 0x004);
    emit32(w);
    instJmp(4);
    emit32(literal);
  }
}

void Assembler::instStMemDir(int gpr, std::string sym){
  int d = symbols[sym].offset - sections[currSection].data.size() - 4;
  if(currSection == symbols[sym].section && d <= 2047 && d >= -2048){
    const uint32_t disp12 = (uint32_t(d) & 0x0FFF);
    uint32_t w = ((STORE_OC << 28)
             | (STORE_MOD0 << 24)
             | (PC_REG << 20)
             | (0 << 16)
             | (gpr << 12)
             | disp12);
    emit32(w);
  }else{
    uint32_t w = ((STORE_OC << 28)
             | (STORE_MOD2 << 24)
             | (PC_REG << 20)
             | (0 << 16)
             | (gpr << 12)
             | 0x004);
    emit32(w);
    instJmp(4);
    sections[currSection].relocs.push_back(Reloc{sym, static_cast<int>(sections[currSection].data.size())});
    emit32(0u);
  }
}

void Assembler::instCsrrd(int csr, int gpr){
  uint32_t word =
      (LOAD_OC << 28) |
      (LOAD_MOD0 << 24) |
      ((uint32_t(gpr) & 0xF) << 20) |
      ((uint32_t(csr) & 0xF) << 16) |
      (0 << 12) |
      0u;
  emit32(word);
}

void Assembler::instCsrwr(int gpr, int csr){
  uint32_t word =
      (LOAD_OC << 28) |
      (LOAD_MOD4 << 24) |
      ((uint32_t(csr) & 0xF) << 20) |
      ((uint32_t(gpr) & 0xF) << 16) |
      (0 << 12) |
      0u;
  emit32(word);
}

void Assembler::write_object(const std::string& path) const {
  std::ofstream out(path);
  if (!out) throw std::runtime_error("cannot open output file");

  out << "OBJV1\n";

  // 1) Sekcije
  out << "SECTIONS " << sections.size() << "\n";
  for (const auto& kv : sections) {
    const auto& sec = kv.second;
    out << ".section " << sec.name
        << " size=" << sec.data.size()
        << " addr=" << sec.addr << "\n";

    out << "data:";
    for (uint8_t b : sec.data) out << ' ' << std::hex << std::uppercase
                                   << std::setw(2) << std::setfill('0') << (int)b;
    out << std::dec << "\n";
  }
  out << "\n";

  // 2) Simboli 
  out << "SYMBOLS " << symbols.size() << "\n";
  for (const auto& kv : symbols) {
    const auto& s = kv.second;
    out << "name=" << kv.first
        << " bind=" << (s.is_global ? "GLOBAL" : "LOCAL")
        << " section=" << s.section
        << " value=" << s.offset << "\n";
  }

  // 3) Relokacije 
  size_t R = 0;
  for (const auto& kv : sections) R += kv.second.relocs.size();
  out << "RELOCS " << R << "\n";
  for (const auto& kv : sections) {
    const auto& sec = kv.second;
    for (const auto& r : sec.relocs) {
      out << "section=" << sec.name
          << " offset="  << r.offset
          << " symbol="  << r.symbol
          << " addend=" << r.addend
          << "\n";
    }
  }
}

void Assembler::write_object_binary(const std::string& path) const {
  rw.write(path, symbols, sections);
}