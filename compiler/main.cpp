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

#include "Linearize_optimized.h"
#include "Linearize.h"

using namespace antlr4;
using namespace std;

int main(int argn, const char **argv)
{
  stringstream in;
  bool optimized = true;
  
  // Check if the correct number of arguments is provided
  if (argn>=2)
  {
     ifstream lecture(argv[1]);
     
     // Check if the file can be read
     if( !lecture.good() )
     {
         cerr<<"error: cannot read file: " << argv[1] << endl ;
         exit(1);
     }
     in << lecture.rdbuf();
     if (argn ==3)
     {
        string arg2 = argv[2];
        if (arg2 == "-O0")
        {
          cerr << "Compilation with no optimizations" << endl;
          optimized = false;
        }
        else{
          cerr << "error: not a right argument : (put -O0 to avoid opitmization)" << argv[2]  << endl;
          exit(1);
        }
        
     }
     
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


  //Vérif de declration et utilisation de toutes les fonctions
  for (auto it=SymbolTableVisitor::symbolTableFonction.begin(); it != SymbolTableVisitor::symbolTableFonction.end(); it++) {
    if (!it->second.declared) {
      cerr << "Fonction " << it->first << " n'est pas déclarée" << endl;
      exit(1);
    }
    if (!it->second.used)
      cerr << "Fonction " << it->first << " n'est jamais utilisée" << endl;
      
  }

  //Vérfi d'utilisation des variables
  for (CFG *cfg : SymbolTableVisitor::cfg_liste) {
    for (auto it=cfg->getSymbolIndex().begin(); it != cfg->getSymbolIndex().end(); ++it){
      if (!it->second.used && it->first != "!returnVal")
      {
        if (it->second.isTable)
        {
          cerr << "Tableau " << it->second.varName << " dans " << it->second.functionName << " n'est jamais utilisé" <<endl;
        }
        else{
          cerr << "Var " << it->second.varName << " dans " << it->second.functionName << " n'est jamais utilisé" <<endl;
        }
        
      }
    }
  }

  // Generate assembly code
  cout << ".globl main\n" ;

  //Create the control flow graph (CFG) using the symbol table
  for (CFG *cfg : SymbolTableVisitor::cfg_liste) {
    if (optimized)
    {
      Linearize_optimized code(cfg);
      code.visit(cfg->getTree());
    }
    else{
      Linearize code(cfg);
      code.visit(cfg->getTree());
    }
    cfg->gen_asm(cout);
    delete(cfg);
  }


  return 0;
}