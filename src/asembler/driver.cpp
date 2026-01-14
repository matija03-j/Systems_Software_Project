#include <cstdio>
#include <cstring>
#include <iostream>
#include "../../inc/assembler.hpp"

// definicija globalnog pokazivača koji parser koristi
Assembler* gAsm = nullptr;

// yyparser interfejs
extern int yyparse();
extern FILE* yyin;

int main(int argc, char** argv) {
  if (argc < 4) { std::cerr << "Usage: " << argv[0] << " -o out.o input.s\n"; return 1; }
  const char* out = nullptr; const char* in = nullptr;
  for (int i=1;i<argc;i++){
    if (strcmp(argv[i], "-o")==0 && i+1<argc) out = argv[++i];
    else in = argv[i];
  }
  if (!out || !in) { std::cerr << "Missing -o or input\n"; return 1; }

  FILE* f = fopen(in, "r");
  if (!f) { perror("fopen"); return 1; }
  yyin = f;

  Assembler core;
  gAsm = &core;

  int rc = yyparse();
  fclose(f);
  if (rc != 0) { std::cerr << "Parsing failed!\n"; return 1; }

  core.write_object_binary(out);

  std::string txtPath = out; txtPath += ".txt";
  core.write_object(txtPath);

  return 0;
}
