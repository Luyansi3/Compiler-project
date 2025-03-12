#pragma once
using namespace std;

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <unordered_map>
#include "SymbolTableVisitor.h"


class  CodeGenVisitor : public ifccBaseVisitor {
	public:
        CodeGenVisitor(unordered_map<string, Flag> symbolTable): ifccBaseVisitor(), symbolTable(symbolTable) { }
        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override ;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
        virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
        virtual antlrcpp::Any visitRvalue(ifccParser::RvalueContext *ctx) override;
        virtual antlrcpp::Any visitLvalue(ifccParser::LvalueContext *ctx) override;
        private:
        unordered_map<string, Flag> symbolTable;
};

