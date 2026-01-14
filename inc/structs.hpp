#pragma once
#include <string>
#include <unordered_map>
#include<vector>

struct Symbol {
  bool is_global = false;
  bool is_defined = false;
  std::string section = "UND";
  uint32_t offset = 0;          
};

struct Reloc {
  std::string symbol;
  int offset;
  int addend = 0;

  Reloc(std::string sym, int off): symbol(sym), offset(off){}
};

struct Section {
    std::string name;
    std::vector<Reloc> relocs;
    uint32_t addr = 0;
    std::vector<uint8_t> data;
};