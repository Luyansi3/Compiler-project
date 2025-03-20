#pragma once
using namespace std;

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include "IR.h"

// Class to linearize the parse tree into intermediate representation (IR)
class Linearize : public ifccBaseVisitor
{
private:
    CFG* cfg; // Control Flow Graph (CFG) pointer

public:
    // Constructor to initialize the CFG pointer
    Linearize(CFG* cfg): cfg(cfg) {}

    // Destructor
    ~Linearize() { }

    // Override methods to visit different parts of the parse tree
    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
    virtual antlrcpp::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
    virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
    virtual antlrcpp::Any visitExprConst(ifccParser::ExprConstContext *ctx) override;
    virtual antlrcpp::Any visitLvalue(ifccParser::LvalueContext *ctx) override;
    virtual antlrcpp::Any visitPar(ifccParser::ParContext *ctx) override;
    virtual antlrcpp::Any visitMulDiv(ifccParser::MulDivContext *ctx) override;
    virtual antlrcpp::Any visitAddSub(ifccParser::AddSubContext *ctx) override;
    virtual antlrcpp::Any visitCall(ifccParser::CallContext *ctx) override;
    virtual antlrcpp::Any visitExprCall(ifccParser::ExprCallContext *ctx) override;
};

