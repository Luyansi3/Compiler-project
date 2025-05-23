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
    unordered_map<string, FlagVar> symbolTableVar;                  // Symbol table to store variable information
    unordered_map<string, FlagFonction> symbolTableFonction; // Symbol Table to store the functions.
    vector<CFG *> cfg_liste;                                 // List of the CFG built
    string scopeString;                                             // Store the current scope of where we are
    string nameCurrentFunction;
    unordered_map<string, int> scope; // store a counter of blocks for a certain depth

    // Constructor to initialize the base visitor and index
    SymbolTableVisitor() : ifccBaseVisitor(), index(-4), scopeString("") {}

        // Override methods to visit different parts of the parse tree
        virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
        virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
        virtual antlrcpp::Any visitCall(ifccParser::CallContext * ctx) override;
        virtual antlrcpp::Any visitPre_decl_fonction(ifccParser::Pre_decl_fonctionContext *ctx) override;
        virtual antlrcpp::Any visitPost_decl_fonction(ifccParser::Post_decl_fonctionContext *ctx) override;
        virtual antlrcpp::Any visitDecl_param(ifccParser::Decl_paramContext *ctx) override;
        virtual antlrcpp::Any visitExprCall(ifccParser::ExprCallContext *ctx) override;
        virtual antlrcpp::Any visitInstrExpr(ifccParser::InstrExprContext *ctx) override;
        ifccParser::CallContext* isASimpleCall(ifccParser::ExprParContext *ctx);
        virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;
        virtual antlrcpp::Any visitTableAffectation(ifccParser::TableAffectationContext *ctx) override;
        virtual antlrcpp::Any visitVarAffectation(ifccParser::VarAffectationContext *ctx) override;
        virtual antlrcpp::Any visitClassicDeclaration(ifccParser::ClassicDeclarationContext *ctx) override;
        virtual antlrcpp::Any visitArray_litteral(ifccParser::Array_litteralContext *ctx) override;
        virtual antlrcpp::Any visitExprTable(ifccParser::ExprTableContext *ctx) override;
    private:
        int index; // Index to keep track of variable positions
    
};