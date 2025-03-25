#pragma once

using namespace std;
#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <unordered_map>
#include <string>

// Structure to hold information about a variable
typedef struct Flag
{
    int index;
    bool used;
    bool affected;
    int nombreParams;
}Flag;

// Class to visit and build the symbol table
class SymbolTableVisitor : public ifccBaseVisitor {
    public:
        unordered_map<string, Flag> symbolTable; // Symbol table to store variable information

        // Constructor to initialize the base visitor and index
        SymbolTableVisitor(): ifccBaseVisitor(), index(-4) { }

        // Override methods to visit different parts of the parse tree
        virtual antlrcpp::Any visitDecl_element(ifccParser::Decl_elementContext *ctx) override;
        virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
        virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
        virtual antlrcpp::Any visitCall(ifccParser::CallContext * ctx) override;
    private:
        int index; // Index to keep track of variable positions
};