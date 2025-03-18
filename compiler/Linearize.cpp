#include "Linearize.h"

antlrcpp::Any Linearize::visitProg(ifccParser::ProgContext *ctx)
{
    
    this->visit(ctx->block());

    return 0;
}

antlrcpp::Any Linearize::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{

    // if(ctx->rvalue()->VAR()){
    //     int index = symbolTable[ctx->rvalue()->VAR()->getText()].index;
    //     std::cout << "    movl "<< index <<"(%rbp), %eax\n" ;
    // }
    // else{
    //     int retval = stoi(ctx->rvalue()->CONST()->getText());
    //     std::cout << "    movl $"<<retval<<", %eax\n" ;
    // }
    this->visit(ctx->rvalue());

    return 0;
}


antlrcpp::Any Linearize::visitAffectation(ifccParser::AffectationContext *ctx)
{
    // string varName = ctx->lvalue()->VAR()->getText();

    // if(ctx->rvalue()->CONST()){
    //     int retval = stoi(ctx->rvalue()->CONST()->getText());
    //     std::cout << "    movl $"<<retval <<", " << symbolTable[varName].index <<"(%rbp)\n" ;
    // }
    // else{
    //     string varName2 = ctx->rvalue()->VAR()->getText();
    //     std::cout << "    movl "<< symbolTable[varName2].index<<"(%rbp), " << symbolTable[varName].index <<"(%rbp)\n" ;
    // }

    this->visit(ctx->rvalue());
    this->visit(ctx->lvalue());


    return 0;
}

antlrcpp::Any Linearize::visitRvalue(ifccParser::RvalueContext *ctx){


    if(ctx->VAR()){
        string varName = ctx->VAR()->getText();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));
    }
    else if(ctx->CONST()){
        int retval = stoi(ctx->CONST()->getText());
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", retval));
    }
    else if (ctx->affectation())
    {
        this->visit(ctx->affectation());
    }
    
    return 0;
}

antlrcpp::Any Linearize::visitLvalue(ifccParser::LvalueContext *ctx){

    if(ctx->VAR()){
        string varName = ctx->VAR()->getText();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));
    }
    return 0;   
}