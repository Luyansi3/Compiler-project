#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "antlr4-runtime.h"
#include "generated/ifccLexer.h"
#include "generated/ifccParser.h"
#include "generated/ifccBaseVisitor.h"

#include "SymbolTableVisitor.h"
#include "IR.h"
#include "Linearize.h"

using namespace antlr4;
using namespace std;

int main(int argn, const char **argv)
{
  stringstream in;
  if (argn==2)
  {
     ifstream lecture(argv[1]);
     if( !lecture.good() )
     {
         cerr<<"error: cannot read file: " << argv[1] << endl ;
         exit(1);
     }
     in << lecture.rdbuf();
  }
  else
  {
      cerr << "usage: ifcc path/to/file.c" << endl ;
      exit(1);
  }
  
  ANTLRInputStream input(in.str());

  ifccLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  tokens.fill();

  ifccParser parser(&tokens);
  tree::ParseTree* tree = parser.axiom();

  if(parser.getNumberOfSyntaxErrors() != 0)
  {
      cerr << "error: syntax error during parsing" << endl;
      exit(1);
  }

  SymbolTableVisitor table;
  table.visit(tree);
  

  for (auto it=table.symbolTable.begin(); it != table.symbolTable.end(); ++it){
    if (!it->second.used)
    {
        cerr << "Var " << it->first << " n'est pas utilisé" <<endl;
    }
  }
  

  CFG cfg(table.symbolTable, "main");
  

  Linearize code(&cfg);
  code.visit(tree);

  cout << ".globl main\n" ;
  cfg.gen_asm(cout);

  return 0;
}
