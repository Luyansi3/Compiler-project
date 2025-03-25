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
  
  // Check if the correct number of arguments is provided
  if (argn==2)
  {
     ifstream lecture(argv[1]);
     
     // Check if the file can be read
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
  
  // Create an ANTLR input stream from the file content
  ANTLRInputStream input(in.str());

  // Initialize the lexer with the input stream
  ifccLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  // Fill the token stream
  tokens.fill();

  // Initialize the parser with the token stream
  ifccParser parser(&tokens);
  tree::ParseTree* tree = parser.axiom();

  // Check for syntax errors
  if(parser.getNumberOfSyntaxErrors() != 0)
  {
      cerr << "error: syntax error during parsing" << endl;
      exit(1);
  }

  // Visit the parse tree to build the symbol table
  SymbolTableVisitor table;
  table.visit(tree);
  
  // Check for unused variables
  for (auto it=table.symbolTable.begin(); it != table.symbolTable.end(); ++it){
    if (!it->second.used && it->second.nombreParams == -1)
    {
        cerr << "Var " << it->first << " n'est pas utilisé" <<endl;
    }
  }
  
  // Create the control flow graph (CFG) using the symbol table
  CFG cfg(table.symbolTable, "main");
  
  // Linearize the code using IR instructions
  Linearize code(&cfg);
  code.visit(tree);

  // Generate assembly code
  cout << ".globl main\n" ;
  cfg.gen_asm(cout);

  return 0;
}