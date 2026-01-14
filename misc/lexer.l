%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

%}

%option noyywrap
%option nounput
%option noinput

DIGIT  [0-9]
ALPHA  [a-zA-Z]
HEX    [0-9A-Fa-f]+
IDENT  [A-Za-z_][A-Za-z0-9_]*
COM    "#"[^\n\r]*

%%

"\.global"    { return GLOBAL; }
"\.extern"    { return EXTERN; }
"\.section"   { return SECTION; }
"\.word"      { return WORD; }
"\.skip"      { return SKIP; }
"\.end"       { return END; }

"halt"   { return HALT; }
"int"    { return INT; }
"iret"   { return IRET; }
"call"   { return CALL; }
"ret"    { return RET; }
"jmp"    { return JMP; }
"beq"    { return BEQ; }
"bne"    { return BNE; }
"bgt"    { return BGT; }
"push"   { return PUSH; }
"pop"    { return POP; }
"xchg"   { return XCHG; }
"add"    { return ADD; }
"sub"    { return SUB; }
"mul"    { return MUL; }
"div"    { return DIV; }
"not"    { return NOT; }
"and"    { return AND; }
"or"     { return OR; }
"xor"    { return XOR; }
"shl"    { return SHL; }
"shr"    { return SHR; }
"ld"     { return LD; }
"st"     { return ST; }
"csrrd"  { return CSRRD; }
"csrwr"  { return CSRWR; }

0x{HEX}  { yylval.num = (int)strtol(yytext + 2, NULL, 16); return NUMBER; }
{DIGIT}+ { yylval.num = atoi(yytext); return NUMBER; }


"%r0"  { yylval.num = 0;  return REGISTER; }
"%r1"  { yylval.num = 1;  return REGISTER; }
"%r2"  { yylval.num = 2;  return REGISTER; }
"%r3"  { yylval.num = 3;  return REGISTER; }
"%r4"  { yylval.num = 4;  return REGISTER; }
"%r5"  { yylval.num = 5;  return REGISTER; }
"%r6"  { yylval.num = 6;  return REGISTER; }
"%r7"  { yylval.num = 7;  return REGISTER; }
"%r8"  { yylval.num = 8;  return REGISTER; }
"%r9"  { yylval.num = 9;  return REGISTER; }
"%r10" { yylval.num = 10; return REGISTER; }
"%r11" { yylval.num = 11; return REGISTER; }
"%r12" { yylval.num = 12; return REGISTER; }
"%r13" { yylval.num = 13; return REGISTER; }
"%r14" { yylval.num = 14; return REGISTER; }
"%r15" { yylval.num = 15; return REGISTER; }
"%sp"  { yylval.num = 14; return REGISTER; }
"%pc"  { yylval.num = 15; return REGISTER; }

"%cause"   { yylval.num = 2; return CSR; }
"%handler" { yylval.num = 1; return CSR; }
"%status"  { yylval.num = 0; return CSR; }
{IDENT}    { yylval.ident = strdup(yytext); return IDENTIFIER; }

\"[^\"]*\" {
    char *s = strdup(yytext + 1);
    size_t len = strlen(s);
    if (len > 0) s[len - 1] = '\0';
    yylval.ident = s;
}


{COM}    ;


"["     { return OBRACKET; }
"]"     { return CBRACKET; }
"+"     { return PLUS; }
"-"     { return MINUS; }
"{"     { return '{'; }
"}"     { return '}'; }
","     { return COMMA; }
":"     { return COLON; }
"$"     { return DOLLAR; }

[ \t]+   { /* skip */ }
\n       { yylineno++; return ENDL; }

.        { printf("Unexpected character at line %d: %s\n", yylineno, yytext); }

%%