#pragma once
#include "structs.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>

class Rw {
public:
  Rw() = default;
  ~Rw() = default;

  void write(const std::string& path, const std::unordered_map<std::string, Symbol>& symbols, const std::unordered_map<std::string, Section>& sections) const;

  void read(const std::string& path, std::unordered_map<std::string, Symbol>& symbolTable, std::unordered_map<std::string, Section>& sectionTable);
};


