#pragma once

using namespace std;
#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <unordered_map>
#include <string>
#include <algorithm>

// Structure to hold information about a variable
typedef struct Flag
{
    int index;
    bool used;
    bool affected;
    int nombreParams;
} Flag;

// Class to visit and build the symbol table
class SymbolTableVisitor : public ifccBaseVisitor
{
public:
    unordered_map<string, Flag> symbolTable; // Symbol table to store variable information

    // Constructor to initialize the base visitor and index
    SymbolTableVisitor() : ifccBaseVisitor(), index(-4) {}

    // Override methods to visit different parts of the parse tree
    virtual antlrcpp::Any visitDecl_element(ifccParser::Decl_elementContext *ctx) override;
    // virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
    virtual antlrcpp::Any visitLvalueAffectation(ifccParser::LvalueAffectationContext *ctx) override;
    virtual antlrcpp::Any visitTableAffectation(ifccParser::TableAffectationContext *ctx) override;
    virtual antlrcpp::Any visitArray_litteral(ifccParser::Array_litteralContext *ctx) override;



    virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
    virtual antlrcpp::Any visitExprConst(ifccParser::ExprConstContext *ctx) override;
    virtual antlrcpp::Any visitMulDiv(ifccParser::MulDivContext *ctx) override;
    virtual antlrcpp::Any visitAddSub(ifccParser::AddSubContext *ctx) override;
    virtual antlrcpp::Any visitExprUnary(ifccParser::ExprUnaryContext *ctx) override;
    virtual antlrcpp::Any visitExprCompRelationnal(ifccParser::ExprCompRelationnalContext *ctx) override;
    virtual antlrcpp::Any visitExprCompEqual(ifccParser::ExprCompEqualContext *ctx) override;
    virtual antlrcpp::Any visitExprAnd(ifccParser::ExprAndContext *ctx) override;
    virtual antlrcpp::Any visitExprOr(ifccParser::ExprOrContext *ctx) override;



    
    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitCall(ifccParser::CallContext *ctx) override;

private:
    int index; // Index to keep track of variable positions
};