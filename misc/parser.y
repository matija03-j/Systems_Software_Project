%{
  #include <iostream>
  using namespace std;

  extern int yylex();
  extern FILE *yyin;
  extern int yylineno;          

  void yyerror(const char *s) {
    cerr << "Parse error (line " << yylineno << "): " << s << endl;
  }

  #include "inc/assembler.hpp"
  extern Assembler* gAsm;
%}

%union {
  int   num;
  char* ident;
}


%token <num> NUMBER
%token <ident> IDENTIFIER
%token <num> REGISTER
%token <num> CSR

%token GLOBAL EXTERN SECTION WORD SKIP END
%token HALT INT IRET CALL RET
%token JMP BEQ BNE BGT
%token PUSH POP
%token XCHG
%token ADD SUB MUL DIV NOT AND OR XOR SHL SHR
%token LD ST CSRRD CSRWR

%token OBRACKET CBRACKET PLUS MINUS
%token COMMA COLON ENDL DOLLAR


%left PLUS MINUS
%right UMINUS UPLUS   /* za %prec u unarnim pravilima */

%start program

%%

program:
    | lines
    ;


lines:
      /* prazno */
    | lines ENDL
    | lines statement ENDL
    ;

statement:
      directive
    | instruction
    | label_def
    ;

directive:
      GLOBAL global_list
    | EXTERN extern_list
    | SECTION IDENTIFIER{ 
        gAsm->declareDirective("section", $2, 0);
        free($2);
    }
    | WORD expr_list
    | SKIP NUMBER{ 
        gAsm->declareDirective("skip", "", $2);
    }
    | END
        { gAsm->end(); }
    ;

global_list:
      IDENTIFIER { 
        gAsm->declareDirective("global", $1, 0);
        free($1); 
    }
    | global_list COMMA IDENTIFIER { 
        gAsm->declareDirective("global", $3, 0);
        free($3); 
    }
    ;

extern_list:
      IDENTIFIER { 
        gAsm->declareDirective("extern", $1, 0);
        free($1);
    }
    | extern_list COMMA IDENTIFIER { 
        gAsm->declareDirective("extern", $3, 0);
        free($3);  
    }
    ;

expr_list:
      NUMBER{ 
        gAsm->declareDirective("word", "", $1); 
    }
    | IDENTIFIER{ 
        gAsm->declareDirective("word", $1, 0);
        free($1);
    }
    | expr_list COMMA NUMBER{ 
        gAsm->declareDirective("word", "", $3); 
    }
    | expr_list COMMA IDENTIFIER{ 
        gAsm->declareDirective("word", $3, 0);
        free($3); 
    }
    ;

label_def:
    IDENTIFIER COLON
    {
      gAsm->defineLabel($1);
    }
    ;

instruction:
      HALT
        { gAsm->instHalt(); }
    | INT
        { gAsm->instInt(); }
    | IRET
        { gAsm->instIret(); }
    | CALL NUMBER
        { gAsm->instCall($2); }
    | CALL IDENTIFIER
        { gAsm->instCall($2); free($2); }
    | RET
        { gAsm->instRet(); }
    | JMP NUMBER
        { gAsm->instJmp($2);}
    | JMP IDENTIFIER
        { gAsm->instJmp($2); free($2); }
    | BEQ REGISTER COMMA REGISTER COMMA NUMBER
        { gAsm->instBeq($2, $4, $6);}
    | BEQ REGISTER COMMA REGISTER COMMA IDENTIFIER
        { gAsm->instBeq($2, $4, $6); free($6); }
    | BNE REGISTER COMMA REGISTER COMMA NUMBER
        { gAsm->instBne($2, $4, $6);}
    | BNE REGISTER COMMA REGISTER COMMA IDENTIFIER
        { gAsm->instBne($2, $4, $6); free($6); }
    | BGT REGISTER COMMA REGISTER COMMA NUMBER
        { gAsm->instBne($2, $4, $6);}
    | BGT REGISTER COMMA REGISTER COMMA IDENTIFIER
        { gAsm->instBgt($2, $4, $6); free($6); }
    | PUSH REGISTER
        { gAsm->instPush($2); }
    | POP REGISTER
        { gAsm->instPop($2); }
    | XCHG REGISTER COMMA REGISTER 
        {gAsm->instXchg($2, $4); }
    | ADD REGISTER COMMA REGISTER
        { gAsm->instAdd($2, $4); }
    | SUB REGISTER COMMA REGISTER
        { gAsm->instSub($2, $4); }
    | MUL REGISTER COMMA REGISTER
        { gAsm->instMul($2, $4); }
    | DIV REGISTER COMMA REGISTER
        { gAsm->instDiv($2, $4); }
    | NOT REGISTER
        { gAsm->instNot($2); }
    | AND REGISTER COMMA REGISTER
        { gAsm->instAnd($2, $4); }
    | OR REGISTER COMMA REGISTER
        { gAsm->instOr($2, $4); }
    | XOR REGISTER COMMA REGISTER
        { gAsm->instXor($2, $4); }
    | SHL REGISTER COMMA REGISTER
        { gAsm->instShl($2, $4); }
    | SHR REGISTER COMMA REGISTER
        { gAsm->instShr($2, $4); }
    | LD DOLLAR NUMBER COMMA REGISTER 
        {gAsm->instLdImm($3, $5);}
    | LD DOLLAR IDENTIFIER COMMA REGISTER 
        {gAsm->instLdImm($3, $5);}
    | LD NUMBER COMMA REGISTER 
        {gAsm->instLdMemDir($2, $4);}
    | LD IDENTIFIER COMMA REGISTER 
        {gAsm->instLdMemDir($2, $4);}
    | LD REGISTER COMMA REGISTER 
        {gAsm->instLdRegDir($2, $4);}
    | LD OBRACKET REGISTER CBRACKET COMMA REGISTER
        {gAsm->instLdRegInd($3, $6);}
    | LD OBRACKET REGISTER PLUS NUMBER CBRACKET COMMA REGISTER
        {gAsm->instLdRegIndOff($3, $5, $8);}
    | LD OBRACKET REGISTER PLUS IDENTIFIER CBRACKET COMMA REGISTER
        {cout << "REG_IND_OFF can't work with SYMBOL" << endl; exit(-1);}
    | ST REGISTER COMMA NUMBER
        { gAsm->instStMemDir($2, $4); }
    | ST REGISTER COMMA IDENTIFIER
        {gAsm->instStMemDir($2, $4); }
    | ST REGISTER COMMA OBRACKET REGISTER CBRACKET
        { gAsm->instStRegInd($2, $5); }
    | ST REGISTER COMMA OBRACKET REGISTER PLUS NUMBER CBRACKET
        { gAsm->instStRegIndOff($2, $5, $7); }
    |CSRRD CSR COMMA REGISTER 
        {gAsm->instCsrrd($2, $4);}
    |CSRWR REGISTER COMMA CSR
        { gAsm->instCsrwr($2, $4); }
    ;

%%