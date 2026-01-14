#include "../../inc/linker.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <set>
#include <iostream>



#include <cstdint>



void Linker::link()
{
  std::unordered_map<std::string, Section> tempSectionTable = {};
  std::unordered_map<std::string, Symbol> tempSymbolTable = {};

  for(std::string file : inputFiles){
    rw.read(file, tempSymbolTable, tempSectionTable);

    fillSectionTable(tempSectionTable);

    fillSymbolTable(tempSymbolTable);
  
    tempSectionTable.clear();
    tempSymbolTable.clear();
  }

  layoutSections();
  applyRelocations();

  writeHexFile();
}

void Linker::fillSectionTable(std::unordered_map<std::string, Section>& tempSectionTable){
  lastBaseOffset.clear();

  for(auto& [sectionName, section]: tempSectionTable){
    if (sectionTable.find(sectionName) == sectionTable.end()) {
      lastBaseOffset[sectionName] = 0;
      sectionTable[sectionName] = section;
      continue;
    }

    uint32_t base = sectionTable[sectionName].data.size();
    lastBaseOffset[sectionName] = base;
    
    for (const auto& r : section.relocs) {
      Reloc rr = r;
      rr.offset += base;
      //std::cout << rr.addend << '\t';
      sectionTable[sectionName].relocs.push_back(std::move(rr));
    }

    sectionTable[sectionName].data.insert(sectionTable[sectionName].data.end(), section.data.begin(), section.data.end());
  }
}


void Linker::fillSymbolTable(std::unordered_map<std::string, Symbol>& tempSymbolTable)
{
  for (const auto& [name, s] : tempSymbolTable) {

    // 1) Ako je simbol DEFINISAN u ovom ulaznom fajlu, pomeri ga za base offset sekcije.
    if (s.is_defined && s.section != "UND") {
      auto itBase = lastBaseOffset.find(s.section);
      if (itBase == lastBaseOffset.end()) {
        throw std::runtime_error("fillSymbolTable: section base not found for symbol '" + name + "' in section '" + s.section + "'");
      }
      uint32_t newOff = itBase->second + s.offset;

      auto itOut = symbolTable.find(name);
      if (itOut != symbolTable.end()) {
        // Već postoji globalni zapis — ako je već definisan, to je dupla definicija.
        if (itOut->first[0] != '.' && itOut->second.is_defined) {
          throw std::runtime_error("duplicate definition of symbol: " + name);
        }
        // Inače dopuni postojeći (bio je npr. extern/UND)
        itOut->second.is_defined = true;
        itOut->second.section = s.section;
        itOut->second.offset = newOff;
        itOut->second.is_global |= s.is_global;
      } else {
        Symbol out = s;
        out.offset = newOff;
        out.is_defined = true;
        symbolTable.emplace(name, std::move(out));
      }
      continue;
    }

    // 2) Ako je simbol NEDERFINISAN (UND) ili extern u ovom fajlu, samo merge u globalnu tabelu.
    auto [itOut, inserted] = symbolTable.emplace(name, s);
    if (!inserted) {
      itOut->second.is_global |= s.is_global;
    }
  }
}

void Linker::layoutSections() {
  // prvo proveri preklapanja među fiksnim -place opisima 
  // napravi pomoćnu listu (sekName, size, *ref) da možemo da računamo end-adrese
  struct Span { 
    std::string name; 
    uint32_t base=0, size=0; 
    Section* sec=nullptr; 
  };
  std::vector<Span> spans;

  
  for (auto& kv : sectionTable) {
    auto& sec = kv.second;
    Span s; 
    s.name = sec.name; 
    s.size = static_cast<uint32_t>(sec.data.size()); 
    s.sec = &sec;
    auto it = placeFixed.find(sec.name);
    if (it != placeFixed.end()) s.base = it->second;
    else s.base = 0xFFFFFFFF;
    spans.push_back(std::move(s));
  }

  // najpre postavi sve fiksirane
  for (auto& s : spans) {
    if (s.base != 0xFFFFFFFF) {
      s.sec->addr = s.base;
    }
  }

  // proveri preklapanja među fiksiranima
  for (size_t i = 0; i < spans.size(); ++i) {
    if (spans[i].base == 0xFFFFFFFF) continue;
    uint32_t iBeg = spans[i].base;
    uint32_t iEnd = iBeg + spans[i].size;
    for (size_t j = i + 1; j < spans.size(); j++) {
      if (spans[j].base == 0xFFFFFFFF) continue;
      uint32_t jBeg = spans[j].base;
      uint32_t jEnd = jBeg + spans[j].size;
      if (!(iEnd <= jBeg || jEnd <= iBeg)) {
        throw std::runtime_error("overlap due to -place between sections '" + spans[i].name + "' and '" + spans[j].name + "'");
      }
    }
  }

  // nađi trenutni maksimum kraja (poslednja zauzeta adresa)
  uint32_t cur = 0;
  for (auto& s : spans) {
    if (s.base == 0xFFFFFFFF) continue;
    uint32_t end = s.base + s.size;
    if (end > cur) cur = end;
  }

  // rasporedi preostale 
  for (auto& s : spans) {
    if (s.base != 0xFFFFFFFF) continue; // već fiksiran
    s.sec->addr = cur;
    cur = cur + s.size;
  }
}

void Linker::applyRelocations() {
  for (auto& kv : sectionTable) {
    Section& placeSec = kv.second;

    for (const Reloc& r : placeSec.relocs) {
      uint32_t targetAddr = 0;

      // 1) Pokušaj GLOBALNI simbol
      auto itSym = symbolTable.find(r.symbol);
      if (itSym != symbolTable.end()) {
        const Symbol& s = itSym->second;
        if (!s.is_defined || s.section == "UND")
          throw std::runtime_error("applyRelocations: unresolved symbol '" + r.symbol + "'");

        auto itSymSec = sectionTable.find(s.section);
        if (itSymSec == sectionTable.end())
          throw std::runtime_error("applyRelocations: symbol section missing '" + s.section + "'");

        targetAddr = itSymSec->second.addr + s.offset;
      } else {
        // 2) Nije u symbolTable tretiraj kao LOKALNO: ime = ime sekcije
        targetAddr = sectionTable[r.symbol].addr;
      }

      // cilj + addend
      uint32_t value = targetAddr + static_cast<uint32_t>(r.addend);
      placeSec.data[r.offset+0] = static_cast<uint8_t>(value & 0xFF);
      placeSec.data[r.offset+1] = static_cast<uint8_t>((value >> 8 ) & 0xFF);
      placeSec.data[r.offset+2] = static_cast<uint8_t>((value >> 16) & 0xFF);
      placeSec.data[r.offset+3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }
  }
}



void Linker::writeHexFile() {
  std::ofstream out(outputFile);
  if (!out) throw std::runtime_error("cannot open output file");

  // sastavi sve parove (addr, byte), pa ih sort po adresi i ispiši po 8 bajtova
  struct Cell { 
    uint32_t addr; 
    uint8_t byte; 
  };
  std::vector<Cell> mem;

  for (const auto& kv : sectionTable) {
    const Section& s = kv.second;
    for (uint32_t i=0;i<s.data.size();++i) {
      mem.push_back(Cell{ s.addr + i, s.data[i] });
    }
  }
  std::sort(mem.begin(), mem.end(), [](const Cell&a, const Cell&b){ return a.addr < b.addr; });

  // štampa po 8 bajtova
  size_t i = 0;
  while (i < mem.size()) {
    uint32_t lineAddr = mem[i].addr & ~0x7u; // poravnaj unazad na 8
    out << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << (lineAddr) << ": ";

    for (int j=0;j<8;++j) {
      uint32_t a = lineAddr + j;
      uint8_t b = 0x00;
      if (i < mem.size() && mem[i].addr == a) { b = mem[i].byte; ++i; }
      out << std::setw(2) << (unsigned)b;
      if (j!=7) out << " ";
    }
    out << "\n";
  }
}



void Linker::write_object(const std::string& path) const {
  std::ofstream out(path);
  if (!out) throw std::runtime_error("cannot open output file");

  out << "OBJV1\n";

  // 1) Sekcije
  out << "SECTIONS " << sectionTable.size() << "\n";
  for (const auto& kv : sectionTable) {
    const auto& sec = kv.second;
    out << ".section " << sec.name
        << " size=" << sec.data.size()
        << " addr=" << sec.addr << "\n";

    // hexdump jedne linije (dovoljno za sada)
    out << "data:";
    for (uint8_t b : sec.data) out << ' ' << std::hex << std::uppercase
                                   << std::setw(2) << std::setfill('0') << (int)b;
    out << std::dec << "\n"; // vrati u dekadski
  }
  out << "\n";

  // 2) Simboli 
  out << "SYMBOLS " << symbolTable.size() << "\n";
  for (const auto& kv : symbolTable) {
    const auto& s = kv.second;
    out << "name=" << kv.first
        << " bind=" << (s.is_global ? "GLOBAL" : "LOCAL")
        << " section=" << s.section
        << " value=" << s.offset << "\n";
  }

  // 3) Relokacije
  size_t R = 0;
  for (const auto& kv : sectionTable) R += kv.second.relocs.size();
  out << "RELOCS " << R << "\n";
  for (const auto& kv : sectionTable) {
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
