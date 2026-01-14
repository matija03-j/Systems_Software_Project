#pragma once
#include "structs.hpp"
#include "rw.hpp"

class Linker{
public:

  Linker(std::vector<std::string> inputFiles, const std::string& outputFile):inputFiles(inputFiles), outputFile(outputFile){
    sectionTable = {};
    symbolTable = {};
  }
  ~Linker() = default;

  void link();

  void addPlacement(const std::string& sec, uint32_t addr) { placeFixed[sec] = addr; }
  const std::unordered_map<std::string, Section>& getSectionTable() const { return sectionTable; }
  const std::unordered_map<std::string, Symbol>&  getSymbolTable()  const { return symbolTable; }

  void layoutSections();
  void applyRelocations();
  void writeHexFile();

private:
  std::unordered_map<std::string, Symbol> symbolTable;
  std::unordered_map<std::string, Section> sectionTable;

  std::unordered_map<std::string, uint32_t> lastBaseOffset;

  std::vector<std::string> inputFiles;
  const std::string& outputFile;

  void write_object(const std::string& path) const;

  Rw rw;

  void fillSectionTable(std::unordered_map<std::string, Section>&);
  void fillSymbolTable(std::unordered_map<std::string, Symbol>&);

  std::unordered_map<std::string, uint32_t> placeFixed;
};