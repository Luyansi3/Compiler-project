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
    string scopeString; // Same use as SymbolTableVar
    string nameCurrentFunction;
    unordered_map<string, int> scope;

public:
    // Constructor to initialize the CFG pointer
    Linearize(CFG* cfg);

    // Destructor
    ~Linearize() {}

    // Override methods to visit different parts of the parse tree
    virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
    virtual antlrcpp ::Any visitAffectation(ifccParser::AffectationContext *ctx) override;
    virtual antlrcpp::Any visitTableAffectation(ifccParser::TableAffectationContext *ctx) override;
    virtual antlrcpp::Any visitExprPar(ifccParser::ExprParContext *ctx) override;
    virtual antlrcpp::Any visitExprVar(ifccParser::ExprVarContext *ctx) override;
    virtual antlrcpp::Any visitExprConst(ifccParser::ExprConstContext *ctx) override;
    virtual antlrcpp::Any visitLvalue(ifccParser::LvalueContext *ctx) override;
    virtual antlrcpp::Any visitExprMulDivMod(ifccParser::ExprMulDivModContext *ctx) override;
    virtual antlrcpp::Any visitExprAddSub(ifccParser::ExprAddSubContext *ctx) override;
    virtual antlrcpp::Any visitExprUnary(ifccParser::ExprUnaryContext *ctx) override;
    virtual antlrcpp::Any visitExprCompRelationnal(ifccParser::ExprCompRelationnalContext *ctx) override;
    virtual antlrcpp::Any visitExprCompEqual(ifccParser::ExprCompEqualContext *ctx) override;
    virtual antlrcpp::Any visitExprAnd(ifccParser::ExprAndContext *ctx) override;
    virtual antlrcpp::Any visitExprOr(ifccParser::ExprOrContext *ctx) override;
    virtual antlrcpp::Any visitIf_stmt(ifccParser::If_stmtContext *ctx) override;
    virtual antlrcpp::Any visitElse_stmt(ifccParser::Else_stmtContext *ctx) override;
    virtual antlrcpp::Any visitElif_stmt(ifccParser::Elif_stmtContext *ctx) override;
    virtual antlrcpp::Any visitWhile_stmt(ifccParser::While_stmtContext *ctx) override;
    virtual antlrcpp::Any visitExprSuffixe(ifccParser::ExprSuffixeContext *ctx) override;
    virtual antlrcpp::Any visitExprPrefixe(ifccParser::ExprPrefixeContext *ctx) override;
    virtual antlrcpp::Any visitCall(ifccParser::CallContext *ctx) override;
    virtual antlrcpp::Any visitExprTable(ifccParser::ExprTableContext *ctx)override;
    virtual antlrcpp::Any visitVarAffectation(ifccParser::VarAffectationContext *ctx)override;
    virtual antlrcpp::Any visitExprShift(ifccParser::ExprShiftContext *ctx) override;
    virtual antlrcpp::Any visitExprAndBit(ifccParser::ExprAndBitContext *ctx) override;
    virtual antlrcpp::Any visitExprOrBit(ifccParser::ExprOrBitContext *ctx) override;
    virtual antlrcpp::Any visitExprXorBit(ifccParser::ExprXorBitContext *ctx) override;
};
