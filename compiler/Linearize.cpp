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



antlrcpp::Any Linearize::visitPar(ifccParser::ParContext *ctx){
    
    this->visit(ctx->expr());

    if(ctx->opU()->MINUS()){
        cfg->current_bb->add_IRInstr(new IRInstrNeg(cfg->current_bb, "!reg"));
    }

    return 0;
}

antlrcpp::Any Linearize::visitExprVar(ifccParser::ExprVarContext *ctx){

    string varName = ctx->VAR()->getText();
    
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));
    if(ctx->opU()->MINUS()){
        cfg->current_bb->add_IRInstr(new IRInstrNeg(cfg->current_bb, "!reg"));
    }

    return 0;
}

antlrcpp::Any Linearize::visitExprConst(ifccParser::ExprConstContext *ctx){

    int retval = stoi(ctx->CONST()->getText());
    if(ctx->opU()->MINUS()){
        retval = -retval;
    }
    cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", retval));
    
    return 0;
}


antlrcpp::Any Linearize::visitLvalue(ifccParser::LvalueContext *ctx){

    if(ctx->VAR()){
        string varName = ctx->VAR()->getText();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
        //cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));
    }
    return 0;   
}

antlrcpp::Any Linearize::visitMulDiv(ifccParser::MulDivContext *ctx){

    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    if(ctx->opM()->MULT()){
        this->visit(expr1);
        string tmp = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
        this->visit(expr2);
        cfg->current_bb->add_IRInstr(new IRInstrMul(cfg->current_bb, tmp, "!reg"));
    }
    else{
        this->visit(expr2);
        string tmp = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
        this->visit(expr1);
        cfg->current_bb->add_IRInstr(new IRInstrDiv(cfg->current_bb, tmp));
    }
    return 0;   
}