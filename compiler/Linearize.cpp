#include "Linearize.h"

antlrcpp::Any Linearize::visitProg(ifccParser::ProgContext *ctx)
{

    this->visit(ctx->block());

    return 0;
}

antlrcpp::Any Linearize::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{

    this->visit(ctx->expr());
    return 0;
}

antlrcpp::Any Linearize::visitAffectation(ifccParser::AffectationContext *ctx)
{
    this->visit(ctx->expr());
    this->visit(ctx->lvalue());

    return 0;
}

antlrcpp::Any Linearize::visitExprVar(ifccParser::ExprVarContext *ctx)
{

    string varName = ctx->VAR()->getText();
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));

    return 0;
}

antlrcpp::Any Linearize::visitExprConst(ifccParser::ExprConstContext *ctx)
{

    int retval = stoi(ctx->CONST()->getText());
    cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", retval));

    return 0;
}

antlrcpp::Any Linearize::visitLvalue(ifccParser::LvalueContext *ctx)
{

    if (ctx->VAR())
    {
        string varName = ctx->VAR()->getText();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));
    }
    return 0;
}

antlrcpp::Any Linearize::visitAddSub(ifccParser::AddSubContext *ctx)
{
    // auto expr1=ctx->expr()[0];
    // auto expr2=ctx->expr()[1];
    if (ctx->opA()->PLUS())
    {
        this->visit(ctx->expr()[0]); // Évaluer la première expression
        string tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));

        this->visit(ctx->expr()[1]); // Évaluer la deuxième expression
        string tmp2 = cfg->create_new_tempvar();

        cfg->current_bb->add_IRInstr(new IRInstrAdd(cfg->current_bb, tmp1, "!reg"));
    }
    else if (ctx->opA()->MINUS())
    {
        this->visit(ctx->expr()[1]); // Évaluer la première expression
        string tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));

        this->visit(ctx->expr()[0]); // Évaluer la deuxième expression

        cfg->current_bb->add_IRInstr(new IRInstrSub(cfg->current_bb, tmp1, "!reg"));
    }
    return 0;
}