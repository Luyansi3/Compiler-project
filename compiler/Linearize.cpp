#include "Linearize.h"

// Visit the program
antlrcpp::Any Linearize::visitProg(ifccParser::ProgContext *ctx)
{
    // Visit the block of the program
    this->visit(ctx->block());

    return 0;
}

// Visit a return statement
antlrcpp::Any Linearize::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    // Visit the expression in the return statement
    this->visit(ctx->expr());

    cfg->current_bb->exit_true = cfg->bb_epi;
    return 0;
}

// Visit an assignment
antlrcpp::Any Linearize::visitAffectation(ifccParser::AffectationContext *ctx)
{
    // Visit the expression and the left-hand side of the assignment
    this->visit(ctx->expr());
    this->visit(ctx->lvalue());

    return 0;
}



// Visit a variable expression
antlrcpp::Any Linearize::visitExprVar(ifccParser::ExprVarContext *ctx)
{
    string varName = ctx->VAR()->getText();
    
    // Add a copy instruction to load the variable into a register
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));

    return 0;
}

antlrcpp::Any Linearize::visitExprUnary(ifccParser::ExprUnaryContext *ctx)
{
    this->visit(ctx->expr());
    // If the expression has a unary minus operator, add a negation instruction
    if(ctx->opU()->MINUS()){
        cfg->current_bb->add_IRInstr(new IRInstrNeg(cfg->current_bb, "!reg"));
    }

    return 0;
}

// Visit a constant expression
antlrcpp::Any Linearize::visitExprConst(ifccParser::ExprConstContext *ctx)
{
    int retval;
    if (ctx->constante()->CONSTINT())
        retval = stoi(ctx->constante()->CONSTINT()->getText());
    else
        retval = (int) ctx->constante()->CONSTCHAR()->getText()[1];

    // Add a load constant instruction
    cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", retval));

    return 0;
}

// Visit a left-hand side value
antlrcpp::Any Linearize::visitLvalue(ifccParser::LvalueContext *ctx)
{
    if (ctx->VAR())
    {
        string varName = ctx->VAR()->getText();
        // Add a copy instruction to store the register value into the variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
    }
    return 0;   
}

// Visit a multiplication or division expression
antlrcpp::Any Linearize::visitMulDiv(ifccParser::MulDivContext *ctx)
{
    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    if(ctx->opM()->MULT()){
        // Visit the first expression
        this->visit(expr1);
        string tmp = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
        // Visit the second expression
        this->visit(expr2);
        // Add a multiplication instruction
        cfg->current_bb->add_IRInstr(new IRInstrMul(cfg->current_bb, tmp, "!reg"));
    }
    else{
        // Visit the second expression
        this->visit(expr2);
        string tmp = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
        // Visit the first expression
        this->visit(expr1);
        // Add a division instruction
        cfg->current_bb->add_IRInstr(new IRInstrDiv(cfg->current_bb, tmp));
    }
    return 0;
}

// Visit an addition or subtraction expression
antlrcpp::Any Linearize::visitAddSub(ifccParser::AddSubContext *ctx)
{
    if (ctx->opA()->PLUS())
    {
        // Visit the first expression
        this->visit(ctx->expr()[0]);
        string tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));

        // Visit the second expression
        this->visit(ctx->expr()[1]);

        // Add an addition instruction
        cfg->current_bb->add_IRInstr(new IRInstrAdd(cfg->current_bb, tmp1, "!reg"));
    }
    else if (ctx->opA()->MINUS())
    {
        // Visit the second expression
        this->visit(ctx->expr()[1]);
        string tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));

        // Visit the first expression
        this->visit(ctx->expr()[0]);

        // Add a subtraction instruction
        cfg->current_bb->add_IRInstr(new IRInstrSub(cfg->current_bb, tmp1, "!reg"));
    }
    return 0;
}




antlrcpp::Any Linearize::visitCall(ifccParser::CallContext *ctx) {
    //Obtentino du label de la fonction
    string label = ctx->VAR()->getText();
    vector<string> params;

    for (auto expression : ctx->liste_param()->expr()) {
        string tmp = cfg->create_new_tempvar();
        this->visit(expression);
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
        params.push_back(tmp);
    }



    

    cfg->current_bb->add_IRInstr(new IrInstrCall(cfg->current_bb, label, params));
    
    


    return 0;
}

antlrcpp::Any Linearize::visitExprComp(ifccParser::ExprCompContext *ctx)
{
    ifccParser::CompContext *ctxComp = ctx->comp();
    ifccParser::ExprContext *ctxExpr1 = ctx->expr(0);
    ifccParser::ExprContext *ctxExpr2 = ctx->expr(1);

    // Visit the second expression
    this->visit(ctxExpr2);
    string tmp1 = cfg->create_new_tempvar();
    // Add a copy instruction to store the result in a temporary variable
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    // Visit the first expression
    this->visit(ctxExpr1);

    if (ctxComp->EQ()) {
        // Add a comparison instruction
        cfg->current_bb->add_IRInstr(new IRInstrCmpEQ(cfg->current_bb, tmp1, "!reg"));
    }
    else if (ctxComp->NEQ()) {
        // Add a comparison instruction
        cfg->current_bb->add_IRInstr(new IRInstrCmpNEQ(cfg->current_bb, tmp1, "!reg"));
    }
    else if (ctxComp->INF()) {
        // Add a comparison instruction
        cfg->current_bb->add_IRInstr(new IRInstrCmpINF(cfg->current_bb, tmp1, "!reg"));
    }
    else if (ctxComp->SUP()) {
        // Add a comparison instruction
        cfg->current_bb->add_IRInstr(new IRInstrCmpSUP(cfg->current_bb, tmp1, "!reg"));
    }
    return 0;
}

antlrcpp::Any Linearize::visitIf_stmt(ifccParser::If_stmtContext *ctx) {
    // Visit the condition expression
    this->visit(ctx->expr());
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    // Create a new basic block for the then part
    BasicBlock *bb_then = new BasicBlock(cfg, cfg->current_bb->label + "_then");
    cfg->add_bb(bb_then);

    if (ctx->block().size() > 1) {
        // Create a new basic block for the else part
        BasicBlock *bb_else = new BasicBlock(cfg, cfg->current_bb->label + "_else");
        cfg->add_bb(bb_else);
        // Create a new basic block for the merge
        BasicBlock *bb_endif = new BasicBlock(cfg, cfg->new_BB_name());
        cfg->add_bb(bb_endif);
        bb_endif->exit_true = cfg->current_bb->exit_true;
        bb_endif->exit_false = cfg->current_bb->exit_false;
        bb_then->exit_true = bb_endif;
        bb_else->exit_true = bb_endif;
        cfg->current_bb->exit_true = bb_then;
        cfg->current_bb->exit_false = bb_else;
        // Set the current basic block to the then part
        cfg->current_bb = bb_then;
        this->visit(ctx->block(0));
        // Set the current basic block to the else part
        cfg->current_bb = bb_else;
        // Visit the else part
        this->visit(ctx->block(1));
        // Set the current basic block to the merge
        cfg->current_bb = bb_endif;
    } else {
        // Create a new basic block for the merge
        BasicBlock *bb_endif = new BasicBlock(cfg, cfg->new_BB_name());
        cfg->add_bb(bb_endif);
        bb_endif->exit_true = cfg->current_bb->exit_true;
        bb_endif->exit_false = cfg->current_bb->exit_false;
        bb_then->exit_true = bb_endif;
        cfg->current_bb->exit_true = bb_then;
        cfg->current_bb->exit_false = bb_endif;
        // Set the current basic block to the then part
        cfg->current_bb = bb_then;
        // Visit the then part
        this->visit(ctx->block(0));
        // Set the current basic block to the endif
        cfg->current_bb = bb_endif;
    }
    return 0;
}

antlrcpp::Any Linearize::visitExprAnd(ifccParser::ExprAndContext *ctx) {
    // Visit the condition expression
    this->visit(ctx->expr(0));
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    BasicBlock *bb_and = new BasicBlock(cfg, cfg->current_bb->label + "_and");
    cfg->add_bb(bb_and);
    BasicBlock *bb_false = new BasicBlock(cfg, cfg->current_bb->label + "_false");
    cfg->add_bb(bb_false);
    BasicBlock *bb_true = new BasicBlock(cfg, cfg->current_bb->label + "_true");
    cfg->add_bb(bb_true);
    BasicBlock *bb_end = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(bb_end);
    bb_and->exit_true = cfg->current_bb->exit_true;
    bb_and->exit_false = cfg->current_bb->exit_false;
    cfg->current_bb->exit_true = bb_and;
    cfg->current_bb->exit_false = bb_false;
    cfg->current_bb = bb_and;

    // Visit the condition expression
    this->visit(ctx->expr(1));
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    bb_end->exit_true = cfg->current_bb->exit_true;
    bb_end->exit_false = cfg->current_bb->exit_false;
    cfg->current_bb->exit_true = bb_true;
    cfg->current_bb->exit_false = bb_false;
    bb_false->exit_true = bb_end;
    bb_true->exit_true = bb_end;
    bb_true->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", 1));
    bb_false->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", 0));
    cfg->current_bb = bb_end;
    return 0;
}

antlrcpp::Any Linearize::visitExprOr(ifccParser::ExprOrContext *ctx) {
    // Visit the condition expression
    this->visit(ctx->expr(0));
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    BasicBlock *bb_or = new BasicBlock(cfg, cfg->current_bb->label + "_or");
    cfg->add_bb(bb_or);
    BasicBlock *bb_true = new BasicBlock(cfg, cfg->current_bb->label + "_true");
    cfg->add_bb(bb_true);
    BasicBlock *bb_false = new BasicBlock(cfg, cfg->current_bb->label + "_false");
    cfg->add_bb(bb_false);
    BasicBlock *bb_end = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(bb_end);
    bb_or->exit_true = cfg->current_bb->exit_true;
    bb_or->exit_false = cfg->current_bb->exit_false;
    cfg->current_bb->exit_true = bb_true;
    cfg->current_bb->exit_false = bb_or;
    cfg->current_bb = bb_or;

    // Visit the condition expression
    this->visit(ctx->expr(1));
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    bb_end->exit_true = cfg->current_bb->exit_true;
    bb_end->exit_false = cfg->current_bb->exit_false;
    cfg->current_bb->exit_true = bb_true;
    cfg->current_bb->exit_false = bb_false;
    bb_false->exit_true = bb_end;
    bb_true->exit_true = bb_end;
    bb_true->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", 1));
    bb_false->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", 0));
    cfg->current_bb = bb_end;
    return 0;
}