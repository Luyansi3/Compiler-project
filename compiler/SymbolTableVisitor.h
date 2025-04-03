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
class SymbolTableVisitor : public ifccBaseVisitor {
    public:
        unordered_map<string, FlagVar> symbolTableVar; // Symbol table to store variable information. Just temporary
        static unordered_map<string, FlagFonction> symbolTableFonction; //Symbol Table to store the function. It is static because common to all Symbol Table.
        static vector<CFG*> cfg_liste; // List of the CFG built

        // Constructor to initialize the base visitor and index
        SymbolTableVisitor(): ifccBaseVisitor(), index(-4) { }

        // Override methods to visit different parts of the parse tree
        virtual antlrcpp::Any visitDecl_element(ifccParser::Decl_elementContext *ctx) override;
        virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
        virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
        virtual antlrcpp::Any visitCall(ifccParser::CallContext * ctx) override;
        virtual antlrcpp::Any visitPre_decl_fonction(ifccParser::Pre_decl_fonctionContext *ctx) override;
        virtual antlrcpp::Any visitPost_decl_fonction(ifccParser::Post_decl_fonctionContext *ctx) override;
        virtual antlrcpp::Any visitDecl_param(ifccParser::Decl_paramContext *ctx) override;
        virtual antlrcpp::Any visitExprCall(ifccParser::ExprCallContext *ctx) override;
        virtual antlrcpp::Any visitExprInstr(ifccParser::ExprInstrContext *ctx) override;    
        ifccParser::CallContext* isASimpleCall(ifccParser::ExprParContext *ctx);
    private:
        int index; // Index to keep track of variable positions
};