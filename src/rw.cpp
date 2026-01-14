#include "../inc/rw.hpp"
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <iomanip>

static inline void wrU32(std::ofstream& f, uint32_t v){ f.write(reinterpret_cast<const char*>(&v), 4); }
static inline void wrI32(std::ofstream& f, int32_t v){  f.write(reinterpret_cast<const char*>(&v), 4); }
static inline void wrU8 (std::ofstream& f, uint8_t v){  f.write(reinterpret_cast<const char*>(&v), 1); }
static inline void wrStr(std::ofstream& f, const std::string& s){
  uint32_t len = static_cast<uint32_t>(s.size());
  wrU32(f, len);
  if (len) f.write(s.data(), len);
}

void Rw::write(const std::string& path,
               const std::unordered_map<std::string, Symbol>& symbols,
               const std::unordered_map<std::string, Section>& sections) const
{
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out) throw std::runtime_error("cannot open binary output");

  // SYMBOLS
  wrU32(out, static_cast<uint32_t>(symbols.size()));
  for (const auto& kv : symbols) {
    const std::string& name = kv.first;
    const Symbol& s = kv.second;
    wrStr(out, name);
    wrStr(out, s.section);
    wrU32(out, s.offset);
    wrU8 (out, s.is_global ? 1 : 0);
    wrU8 (out, s.is_defined ? 1 : 0);
  }

  // SECTIONS
  wrU32(out, static_cast<uint32_t>(sections.size()));
  for (const auto& kv : sections) {
    const Section& sec = kv.second;
    wrStr(out, sec.name);
    wrU32(out, sec.addr);

    wrU32(out, static_cast<uint32_t>(sec.relocs.size()));
    for (const auto& r : sec.relocs) {
      wrU32(out, r.offset);
      wrStr(out, r.symbol);
      wrI32(out, r.addend);
    }

    wrU32(out, static_cast<uint32_t>(sec.data.size()));
    if (!sec.data.empty()) {
      out.write(reinterpret_cast<const char*>(sec.data.data()),
                static_cast<std::streamsize>(sec.data.size()));
    }
  }
}


static inline void rdU32(std::ifstream& f, uint32_t& out) {
  if (!f.read(reinterpret_cast<char*>(&out), 4)) throw std::runtime_error("readFile: u32 read failed");
}
static inline void rdI32(std::ifstream& f, int32_t& out) {
  if (!f.read(reinterpret_cast<char*>(&out), 4)) throw std::runtime_error("readFile: i32 read failed");
}
static inline void rdU8 (std::ifstream& f, uint8_t&  out) {
  if (!f.read(reinterpret_cast<char*>(&out), 1)) throw std::runtime_error("readFile: u8 read failed");
}
static inline void rdStr(std::ifstream& f, std::string& s) {
  uint32_t len=0; rdU32(f,len);
  s.resize(len);
  if (len && !f.read(&s[0], len)) throw std::runtime_error("readFile: str read failed");
}

void Rw::read(const std::string& fileName, std::unordered_map<std::string, Symbol>& symbolTable, std::unordered_map<std::string, Section>& sectionTable)
{
  std::ifstream in(fileName, std::ios::binary);
  if (!in) throw std::runtime_error("readFile: cannot open " + fileName);

  // SYMBOLS
  uint32_t nsym=0; rdU32(in, nsym);
  for (uint32_t i=0;i<nsym;++i) {
    std::string name, secname;
    uint32_t off=0; 
    uint8_t g=0,d=0;

    rdStr(in, name);
    rdStr(in, secname);
    rdU32(in, off);
    rdU8 (in, g);      // is_global
    rdU8 (in, d);      // is_defined

    Symbol s;
    s.section    = secname;   // "UND" ako nedef.
    s.offset     = off;
    s.is_global  = (g!=0);
    s.is_defined = (d!=0);

    auto [it,ok] = symbolTable.emplace(name, std::move(s));
    if (!ok) throw std::runtime_error("readFile: duplicate symbol in object: " + name);
  }

  // SECTIONS (svaka: name, addr, nreloc, [relocs], size, data)
  uint32_t nsec=0; rdU32(in, nsec);
  for (uint32_t i=0;i<nsec;++i) {
    Section sec;
    rdStr(in, sec.name);
    rdU32(in, sec.addr);

    // relokacije: (offset, symbol, addend)
    uint32_t nrel=0; rdU32(in, nrel);
    sec.relocs.clear();
    sec.relocs.reserve(nrel);
    for (uint32_t r=0;r<nrel;++r) {
      uint32_t off=0; std::string sym; int32_t add=0;
      rdU32(in, off);
      rdStr(in, sym);
      rdI32(in, add);
      sec.relocs.emplace_back(sym, off);
      sec.relocs.back().addend = add;
    }

    // data
    uint32_t size=0; rdU32(in, size);
    sec.data.resize(size);
    if (size && !in.read(reinterpret_cast<char*>(sec.data.data()), size))
      throw std::runtime_error("readFile: section data read failed for " + sec.name);

    auto [it,ok] = sectionTable.emplace(sec.name, std::move(sec));
    if (!ok) throw std::runtime_error("readFile: duplicate section in object: " + it->first);
  }
}
