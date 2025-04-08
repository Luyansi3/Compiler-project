#pragma once

using namespace std;
#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <unordered_map>
#include <string>
#include "IR.h"
#include "Flag.h"

class CFG;

// Class to visit and build the symbol table
class SymbolTableVisitor : public ifccBaseVisitor
{
public:
    unordered_map<string, FlagVar> symbolTableVar;                  // Symbol table to store variable information. Just temporary
    static unordered_map<string, FlagFonction> symbolTableFonction; // Symbol Table to store the function. It is static because common to all Symbol Table.
    static vector<CFG *> cfg_liste;                                 // List of the CFG built
    string scopeString;                                             // Store the current scope of where we are
    string nameCurrentFunction;
    unordered_map<string, int> scope; // store a counter of blocks for a certain depth

    // Constructor to initialize the base visitor and index
    SymbolTableVisitor() : ifccBaseVisitor(), index(-4), scopeString("") {}

    // Override methods to visit different parts of the parse tree
    virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;
    virtual antlrcpp::Any visitDecl_element(ifccParser::Decl_elementContext *ctx) override;
    virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitCall(ifccParser::CallContext *ctx) override;
    virtual antlrcpp::Any visitDecl_fonction(ifccParser::Decl_fonctionContext *ctx) override;
    virtual antlrcpp::Any visitDecl_param(ifccParser::Decl_paramContext *ctx) override;
    virtual antlrcpp::Any visitLvalueAffectation(ifccParser::LvalueAffectationContext *ctx) override;
    virtual antlrcpp::Any visitTableAffectation(ifccParser::TableAffectationContext *ctx) override;
    virtual antlrcpp::Any visitArray_litteral(ifccParser::Array_litteralContext *ctx) override;
    int getIndex(){return index;}

private:
    int index; // Index to keep track of variable positions
};