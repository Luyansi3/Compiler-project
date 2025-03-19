#pragma once
using namespace std;
#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <unordered_map>
#include <string>

typedef struct Flag
{
    int index;
    bool used;
    bool affected;
}Flag;


class  SymbolTableVisitor : public ifccBaseVisitor {
	public:
        unordered_map<string, Flag> symbolTable;

        SymbolTableVisitor():  ifccBaseVisitor(), index(-4) { }
        virtual antlrcpp::Any visitDecl_element(ifccParser::Decl_elementContext *ctx) override ;
        virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
        virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
    private:
        int index;
};