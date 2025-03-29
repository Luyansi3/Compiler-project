#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "antlr4-runtime.h"
#include "generated/ifccLexer.h"
#include "generated/ifccParser.h"
#include "generated/ifccBaseVisitor.h"

#include "IR.h"
#include "SymbolTableVisitor.h"

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
  //  for (auto it=table.symbolTable.begin(); it != table.symbolTable.end(); ++it){
  //    if (!it->second.used && it->second.nombreParams == -1)
  //    {
  //        cerr << "Var " << it->first << " n'est pas utilisé" <<endl;
  //    }
  //  }


  for (auto it=SymbolTableVisitor::symbolTableFonction.begin(); it != SymbolTableVisitor::symbolTableFonction.end(); it++) {
    if (!it->second.declared) {
      cerr << "Fonction" << it->first << " n'est pas déclarée" << endl;
      exit(1);
    }
    if (!it->second.used)
      cerr << "Fonction" << it->first << " n'est jamais utilisée" << endl;
      
  }

  for (CFG *cfg : SymbolTableVisitor::cfg_liste) {
    for (auto it=cfg->getSymbolIndex().begin(); it != cfg->getSymbolIndex().end(); ++it){
      if (!it->second.used)
      {
          cerr << "Var " << it->first << " n'est jamais utilisé" <<endl;
      }
    }
  }

  // Generate assembly code
  cout << ".globl main\n" ;

  //Create the control flow graph (CFG) using the symbol table
  for (CFG *cfg : SymbolTableVisitor::cfg_liste) {
    Linearize code(cfg);
    code.visit(cfg->getTree());
    cfg->gen_asm(cout);
    delete(cfg);
  }

  

  


  return 0;
}