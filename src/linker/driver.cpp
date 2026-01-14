#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include "../../inc/linker.hpp"

static uint32_t parseUint(const std::string& s) {
  if (s.rfind("0x", 0) == 0 || s.rfind("0X",0)==0) {
    return static_cast<uint32_t>(std::stoul(s, nullptr, 16));
  }
  return static_cast<uint32_t>(std::stoul(s, nullptr, 10));
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: linker [options] file.o ...\n"
              << "  -o <out>              # output filename\n"
              << "  -place=<sec>@<addr>   # e.g. -place=text@0x40000000\n"
              << "  -hex                  # emit hex (nivo A)\n";
    return 1;
  }

  std::vector<std::string> inputs;
  std::string outFile = "a.hex";
  bool hexMode = false;
  std::unordered_map<std::string,uint32_t> placements;

  for (int i=1; i<argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-o") {
      if (i+1 >= argc) { std::cerr << "Missing argument after -o\n"; return 1; }
      outFile = argv[++i];
    } else if (arg == "-hex") {
      hexMode = true;
    } else if (arg.rfind("-place=",0)==0) {
      // format: -place=<sec>@<addr>
      auto payload = arg.substr(7);
      auto at = payload.find('@');
      if (at == std::string::npos) { std::cerr << "Bad -place format: " << arg << "\n"; return 1; }
      std::string sec  = payload.substr(0, at);
      std::string addr = payload.substr(at+1);
      try {
        placements[sec] = parseUint(addr);
      } catch (...) {
        std::cerr << "Bad address in -place: " << arg << "\n"; return 1;
      }
    } else if (!arg.empty() && arg[0] == '-') {
      std::cerr << "Unknown option: " << arg << "\n"; return 1;
    } else {
      inputs.push_back(arg);
    }
  }

  if (inputs.empty()) {
    std::cerr << "No input files.\n"; return 1;
  }
  // u nivou A tražimo tačno -hex
  if (!hexMode) {
    std::cerr << "You must pass exactly one of -hex or -relocatable (A nivo: -hex).\n";
    return 1;
  }

  try {
    Linker L(inputs, outFile);
    for (auto& kv : placements) L.addPlacement(kv.first, kv.second);


    L.link();
    std::cout << "OK: wrote " << outFile << "\n";
  } catch (const std::exception& e) {
    std::cerr << "Link error: " << e.what() << "\n";
    return 2;
  }

  return 0;
}
