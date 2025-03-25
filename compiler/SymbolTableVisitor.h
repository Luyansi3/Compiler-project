#pragma once

using namespace std;
#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <unordered_map>
#include <string>

// Structure to hold information about a variable
typedef struct FlagVar
{
    int index;
    bool used;
    bool affected;
}Flag;

typedef struct FlagFonction
{
    bool used;
    bool declared;
    int nombreParams;
}

// Class to visit and build the symbol table
class SymbolTableVisitor : public ifccBaseVisitor {
    public:
        unordered_map<string, unordered_map<string, Flag>> symbolTableVar; // Symbol table to store variable information
        static unrodered_map<string, FlagFonction> symbolTableFonction; //Symbol Table to store the function. It is static because common to all Symbol Table.
        unordered_map<string, Flag> currentSymbolTable;

        // Constructor to initialize the base visitor and index
        SymbolTableVisitor(): ifccBaseVisitor(), index(-4) { }

        // Override methods to visit different parts of the parse tree
        virtual antlrcpp::Any visitDecl_element(ifccParser::Decl_elementContext *ctx) override;
        virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
        virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
        virtual antlrcpp::Any visitCall(ifccParser::CallContext * ctx) override;
        virtual antlrcpp::Any visitDecl_fonction(ifccParser::Decl_fonction *ctx) override;
    private:
        int index; // Index to keep track of variable positions
};