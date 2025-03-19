#pragma once
using namespace std;

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include "IR.h"

class Linearize : public ifccBaseVisitor
{
private:
    CFG* cfg;
public:
    Linearize(CFG* cfg): cfg(cfg) {}
    ~Linearize() { }
    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
    virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
    virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
    virtual antlrcpp::Any visitExprConst(ifccParser::ExprConstContext *ctx) override;
    virtual antlrcpp::Any visitLvalue(ifccParser::LvalueContext *ctx) override;
    virtual antlrcpp::Any visitMulDiv(ifccParser::MulDivContext *ctx) override;

};

