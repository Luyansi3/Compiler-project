#include "Linearize.h"



Linearize::Linearize(CFG* cfg): cfg(cfg){
    // Initialize the scopeString and nameCurrentFunction
    scopeString = cfg->getNameFunction();
    nameCurrentFunction = cfg->getNameFunction();
}


antlrcpp::Any Linearize::visitBlock(ifccParser::BlockContext *ctx) {

    // Same work as with symbolTable, we want to know in which scope we are
    if(scope.find(scopeString) == scope.end()){
        scope.insert({scopeString, 0});
    }

    scope[scopeString]++;
    scopeString += "_"+to_string(scope[scopeString]);

    this->visit(ctx->instructions());


    size_t pos = scopeString.find_last_of('_');
    
    scope.erase(scopeString);

    if (pos != string::npos) {
        scopeString.erase(pos);
    }


    return 0;

}



// Visit a return statement
antlrcpp::Any Linearize::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    // Visit the expression in the return statement
    this->visit(ctx->expr());
    // Put the result in the return value register
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!returnVal","!reg"));
    // Unconditional jump to the epilogue
    cfg->current_bb->add_IRInstr(new IRInstrJump(cfg->current_bb,cfg->bb_epi->label));

    return 0;
}

// Visit an assignment
antlrcpp::Any Linearize::visitAffectation(ifccParser::AffectationContext *ctx)
{
    if (ctx->op_compose()->EQUAL())
    {
        this->visit(ctx->expr());
        this->visit(ctx->lvalue());
    }
    else
    {
        string varName;
        varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
        if (ctx->lvalue()->expr())
        {
            this->visit(ctx->lvalue()->expr());
            string index = cfg->create_new_tempvar();
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, index, "!reg"));
            varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
            cfg->current_bb->add_IRInstr(new IRInstrCopyMem(cfg->current_bb, index, varName));
            varName = cfg->create_new_tempvar(); // Reajust varName to store the table memory address to avoid duplicates if
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
        }

        // Visit the expression and the left-hand side of the assignment
        this->visit(ctx->expr());

        // Create the appropriate IR instruction based on the operator
        if (ctx->op_compose()->PLUSEQUAL())
        {
            cfg->current_bb->add_IRInstr(new IRInstrAdd(cfg->current_bb, varName, "!reg"));
        }
        else if (ctx->op_compose()->MOINSEQUAL()) {
            string tmp = cfg->create_new_tempvar();
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));
            cfg->current_bb->add_IRInstr(new IRInstrSub(cfg->current_bb, tmp, "!reg"));
        }
        else if (ctx->op_compose()->MULTEQUAL()) {
            cfg->current_bb->add_IRInstr(new IRInstrMul(cfg->current_bb, varName, "!reg"));
        }
        else if (ctx->op_compose()->DIVEQUAL()) {
            string tmp = cfg->create_new_tempvar();
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));
            cfg->current_bb->add_IRInstr(new IRInstrDiv(cfg->current_bb, tmp));
        }
        else if (ctx->op_compose()->SHLEQUAL()) {
            cfg->current_bb->add_IRInstr(new IRInstrSHL(cfg->current_bb, "!reg", varName));
        }
        else if (ctx->op_compose()->SHREQUAL()) {
            cfg->current_bb->add_IRInstr(new IRInstrSHR(cfg->current_bb, "!reg", varName));
        }
        else if (ctx->op_compose()->XOREQUAL()) {
            cfg->current_bb->add_IRInstr(new IRInstrXorBit(cfg->current_bb, varName, "!reg"));
        }
        else if (ctx->op_compose()->OREQUAL()) {
            cfg->current_bb->add_IRInstr(new IRInstrOrBit(cfg->current_bb, varName, "!reg"));
        }
        else if (ctx->op_compose()->ANDEQUAL()) {
            cfg->current_bb->add_IRInstr(new IRInstrAndBit(cfg->current_bb, varName, "!reg"));
        }
        else if (ctx->op_compose()->MODEQUAL()) {
            string tmp = cfg->create_new_tempvar();
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));
            cfg->current_bb->add_IRInstr(new IRInstrMod(cfg->current_bb, tmp));
        }

        // Visit the left-hand side of the assignment
        this->visit(ctx->lvalue());
    }

    return 0;
}
antlrcpp::Any Linearize::visitTableAffectation(ifccParser::TableAffectationContext *ctx)
{
    int i = 0;
    string baseVarName = cfg->getVarName(ctx->VAR()->getText(), scopeString); // Array base

    for (auto &expr : ctx->array_litteral()->expr())
    {
        this->visit(expr);
        string tmpValue = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmpValue, "!reg")); // store the expr evaluation into tmpValue

        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", i)); // store the index into !reg

        // Write the value at the appropriate index in the array
        cfg->current_bb->add_IRInstr(new IRInstrMem(cfg->current_bb, tmpValue, "!reg", baseVarName));
        i++;
    }

    int size =0;
    if (ctx->constante())
    {
        if (ctx->constante()->CONSTINT())
            size = stoi(ctx->constante()->CONSTINT()->getText());
        else
        {
            string text = ctx->constante()->CONSTCHAR()->getText();
            if (text.size() <= 2 )
            {
                cerr << "Error not a valid char" << endl;
                exit(1);
            }
            
            if (text.find("'\\n'") != string::npos) {
                size =  (int) '\n';  // Convert the escape sequence to the actual newline character
            } 
            else if (text.find("'\\t'")!= string::npos) {
                size =  (int) '\t';  // Convert to tab character
            } 
            else if (text.find("'\\r'")!= string::npos) {
                size =  (int) '\r';  // Convert to carriage return
            }
            else{
                size = (int) text[text.size()-2]; // Take the last char before the quote
            }
        }
    }
    
    
    // Load 0 in the array if the size is bigger than the number of elements
    while (i < size)
    {
        string tmpValue = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmpValue, 0)); // store 0 into tmpValue

        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", i)); // store the index into !reg

        // Write the value at the appropriate index in the array
        cfg->current_bb->add_IRInstr(new IRInstrMem(cfg->current_bb, tmpValue, "!reg", baseVarName));
        i++;
    }

    return 0;
}

antlrcpp::Any Linearize::visitExprVar(ifccParser::ExprVarContext *ctx)
{
    string varName = cfg->getVarName(ctx->VAR()->getText(), scopeString);

    // Add a copy instruction to load the variable into !reg
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));

    return 0;
}

antlrcpp::Any Linearize::visitExprUnary(ifccParser::ExprUnaryContext *ctx)
{
    this->visit(ctx->expr());

    // If the expression has a unary minus operator, add a negation instruction
    if (ctx->opU()->MINUS())
    {
        cfg->current_bb->add_IRInstr(new IRInstrNeg(cfg->current_bb, "!reg"));
    }

    // If the expression has a unary not operator, add a not instruction
    else if(ctx->opU()->NOT()){
        cfg->current_bb->add_IRInstr(new IRInstrNot(cfg->current_bb, "!reg"));
    }

    return 0;
}

// Visit a constant expression
antlrcpp::Any Linearize::visitExprConst(ifccParser::ExprConstContext *ctx)
{
    int retval;
    if (ctx->constante()->CONSTINT())
        retval = stoi(ctx->constante()->CONSTINT()->getText());
    else{
        string text = ctx->constante()->CONSTCHAR()->getText();
        if (text.size() <= 2 )
        {
            cerr << "Error not a valid char" << endl;
            exit(1);
        }
        
        if (text.find("'\\n'") != string::npos) {
            retval =  (int) '\n';  // Convert the escape sequence to the actual newline character
        } 
        else if (text.find("'\\t'")!= string::npos) {
            retval =  (int) '\t';  // Convert to tab character
        } 
        else if (text.find("'\\r'")!= string::npos) {
            retval =  (int) '\r';  // Convert to carriage return
        }
        else{
            retval = (int) text[text.size()-2]; // Take the last char before the quote
        }
        
    }

    // Add a load constant instruction
    cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", retval));

    return 0;
}

// Visit a left-hand side value
antlrcpp::Any Linearize::visitLvalue(ifccParser::LvalueContext *ctx)
{
    // // Visit the expression and the left-hand side of the assignment
    if (ctx->VAR() && !ctx->expr())
    {
        string varName = cfg->getVarName(ctx->VAR()->getText(), scopeString);
        // Add a copy instruction to store the register value into the variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
    }
    else
    {
        string tmpValue = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmpValue, "!reg"));
        this->visit(ctx->expr());
        string baseVarName = cfg->getVarName(ctx->VAR()->getText(), scopeString);
        string tmpIndex = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmpIndex, "!reg"));
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", tmpValue));
        cfg->current_bb->add_IRInstr(new IRInstrMem(cfg->current_bb, tmpValue, tmpIndex, baseVarName));
    }
    return 0;
}

// Visit a multiplication or division expression
antlrcpp::Any Linearize::visitExprMulDivMod(ifccParser::ExprMulDivModContext *ctx)
{
    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    if (ctx->opM()->MULT())
    {
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
    else if (ctx->opM()->MOD()) {
        auto expr1 = ctx->expr(0);
        auto expr2 = ctx->expr(1);

        this->visit(expr2);
        string tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));

        this->visit(expr1);

        
        cfg->current_bb->add_IRInstr(new IRInstrMod(cfg->current_bb, tmp1));
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
antlrcpp::Any Linearize::visitExprAddSub(ifccParser::ExprAddSubContext *ctx)
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
    // Get the label of the function
    string label = ctx->VAR()->getText();
    vector<string> params;

    // Get the params
    for (auto expression : ctx->liste_param()->expr()) {
        string tmp = cfg->create_new_tempvar();
        this->visit(expression);
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
        params.push_back(tmp);
    }

    // Add a call instruction
    cfg->current_bb->add_IRInstr(new IrInstrCall(cfg->current_bb, label, params));
    
   
    return 0;
}

antlrcpp::Any Linearize::visitExprCompRelationnal(ifccParser::ExprCompRelationnalContext *ctx)
{
    ifccParser::CompRelationnalContext *ctxComp = ctx->compRelationnal();
    ifccParser::ExprContext *ctxExpr1 = ctx->expr(0);
    ifccParser::ExprContext *ctxExpr2 = ctx->expr(1);

    // Visit the second expression
    this->visit(ctxExpr2);
    string tmp1 = cfg->create_new_tempvar();
    // Add a copy instruction to store the result in a temporary variable
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    // Visit the first expression
    this->visit(ctxExpr1);

    if (ctxComp->INF())
    {
        // Add a comparison instruction
        cfg->current_bb->add_IRInstr(new IRInstrCmpINF(cfg->current_bb, tmp1, "!reg"));
    }
    else if (ctxComp->SUP())
    {
        // Add a comparison instruction
        cfg->current_bb->add_IRInstr(new IRInstrCmpSUP(cfg->current_bb, tmp1, "!reg"));
    }
    return 0;
}

antlrcpp::Any Linearize::visitExprCompEqual(ifccParser::ExprCompEqualContext *ctx)
{
    ifccParser::CompEqualContext *ctxComp = ctx->compEqual();
    ifccParser::ExprContext *ctxExpr1 = ctx->expr(0);
    ifccParser::ExprContext *ctxExpr2 = ctx->expr(1);

    // Visit the second expression
    this->visit(ctxExpr2);
    string tmp1 = cfg->create_new_tempvar();
    // Add a copy instruction to store the result in a temporary variable
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    // Visit the first expression
    this->visit(ctxExpr1);

    if (ctxComp->EQ())
    {
        // Add a comparison instruction
        cfg->current_bb->add_IRInstr(new IRInstrCmpEQ(cfg->current_bb, tmp1, "!reg"));
    }
    else if (ctxComp->NEQ())
    {
        // Add a comparison instruction
        cfg->current_bb->add_IRInstr(new IRInstrCmpNEQ(cfg->current_bb, tmp1, "!reg"));
    }
    return 0;
}

antlrcpp::Any Linearize::visitIf_stmt(ifccParser::If_stmtContext *ctx)
{
    BasicBlock *bb_init = cfg->current_bb;
    // Visit the condition expression
    this->visit(ctx->expr());
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    // Create a new basic block for the then part
    BasicBlock *bb_then = new BasicBlock(cfg, cfg->current_bb->label + "_then");
    cfg->add_bb(bb_then);

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
    if (ctx->instruction()) {
        this->visit(ctx->instruction());
    }
    else {
        this->visit(ctx->block());
    }

    // Set the current basic block to the init bb
    cfg->current_bb = bb_init;

    ifccParser::Elif_stmtContext *elif = ctx->elif_stmt(0);
    int i = 0;
    while (elif != nullptr)
    {
        // Create a new basic block for the elif part
        BasicBlock *bb_elif = new BasicBlock(cfg, cfg->current_bb->label + "_elif");
        cfg->add_bb(bb_elif);
        bb_elif->exit_true = bb_endif;
        cfg->current_bb->exit_false = bb_elif;
        // Set the current basic block to the endif
        cfg->current_bb = bb_elif;

        this->visit(elif);
        i++;
        elif = ctx->elif_stmt(i);
    }

    if (ctx->else_stmt())
    {
        this->visit(ctx->else_stmt());
    }

    cfg->current_bb = bb_endif;

    return 0;
}

antlrcpp::Any Linearize::visitElif_stmt(ifccParser::Elif_stmtContext *ctx)
{
    // Visit the condition expression
    this->visit(ctx->expr());
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    // Create a new basic block for the then part
    BasicBlock *bb_then = new BasicBlock(cfg, cfg->current_bb->label + "_then");
    cfg->add_bb(bb_then);

    // Create a new basic block for the merge
    BasicBlock *bb_endif = cfg->current_bb->exit_true;
    BasicBlock *bb_init = cfg->current_bb;
    // bb_endif->exit_true = cfg->current_bb->exit_true;
    bb_then->exit_true = bb_endif;
    cfg->current_bb->exit_true = bb_then;
    cfg->current_bb->exit_false = bb_endif;
    // Set the current basic block to the then part
    cfg->current_bb = bb_then;
    // Visit the then part
    if (ctx->instruction()) {
        this->visit(ctx->instruction());
    }
    else {
        this->visit(ctx->block());
    }

    // Set the current basic block to the initial block
    cfg->current_bb = bb_init;

    return 0;
}

antlrcpp::Any Linearize::visitElse_stmt(ifccParser::Else_stmtContext *ctx)
{
    BasicBlock *bb_init = cfg->current_bb;
    // Create a new basic block for the else part
    BasicBlock *bb_else = new BasicBlock(cfg, cfg->current_bb->label + "_else");
    cfg->add_bb(bb_else);
    // Retrieve endif bb
    BasicBlock *bb_endif = cfg->current_bb->exit_false;
    bb_else->exit_true = bb_endif;
    cfg->current_bb->exit_false = bb_else;
    // Set the current basic block to the else part
    cfg->current_bb = bb_else;
    // Visit the else part
    if (ctx->instruction()) {
        this->visit(ctx->instruction());
    }
    else {
        this->visit(ctx->block());
    }
    cfg->current_bb = bb_init;

    return 0;
}

antlrcpp::Any Linearize::visitExprAnd(ifccParser::ExprAndContext *ctx) {
    // Visit the condition expression
    this->visit(ctx->expr(0));
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
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
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
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
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
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
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
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

antlrcpp::Any Linearize::visitWhile_stmt(ifccParser::While_stmtContext *ctx)
{
    BasicBlock *bb_body = new BasicBlock(cfg, cfg->current_bb->label + "_while_body");
    cfg->add_bb(bb_body);
    BasicBlock *bb_test = new BasicBlock(cfg, cfg->current_bb->label + "_while_test");
    cfg->add_bb(bb_test);
    BasicBlock *bb_end = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(bb_end);

    bb_end->exit_true = cfg->current_bb->exit_true;
    bb_end->exit_false = cfg->current_bb->exit_false;
    cfg->current_bb->exit_true = bb_test;
    bb_test->exit_true = bb_body;
    bb_test->exit_false = bb_end;
    bb_body->exit_true = bb_test;

    cfg->current_bb = bb_test;

    // Visit the condition expression
    this->visit(ctx->expr());
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));

    cfg->current_bb = bb_body;
    // Visit the body of the while loop
    if (ctx->instruction()) {
        this->visit(ctx->instruction());
    }
    else {
        this->visit(ctx->block());
    }

    cfg->current_bb = bb_end;

    return 0;
}

antlrcpp::Any Linearize::visitExprSuffixe(ifccParser::ExprSuffixeContext *ctx)
{
    string varName;
    varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
    if (ctx->lvalue()->expr()) {
        this->visit(ctx->lvalue()->expr());
        string index = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, index, "!reg"));
        varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
        cfg->current_bb->add_IRInstr(new IRInstrCopyMem(cfg->current_bb, index, varName));
        varName = cfg->create_new_tempvar(); // Reajust varName to store the table memory address to avoid duplicates if
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
    }

    if (ctx->opD()->PLUSPLUS())
    {
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));

        string tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the variable initial value in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));

        string tmp2 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp2, 1));

        // Add an addition instruction
        cfg->current_bb->add_IRInstr(new IRInstrAdd(cfg->current_bb, tmp2, "!reg"));

        this->visit(ctx->lvalue());

        // Put back the initial value in !reg
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", tmp1));
    }
    else if (ctx->opD()->MOINSMOINS())
    {
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));

        string tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the variable initial value in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));

        string tmp2 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp2, 1));

        // Add a subtraction instruction
        cfg->current_bb->add_IRInstr(new IRInstrSub(cfg->current_bb, tmp2, "!reg"));

        this->visit(ctx->lvalue());

        // Put back the initial value in !reg
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", tmp1));
    }

    return 0;
}

antlrcpp::Any Linearize::visitExprPrefixe(ifccParser::ExprPrefixeContext *ctx) {
    string varName;
    varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
    if (ctx->lvalue()->expr()) {
        this->visit(ctx->lvalue()->expr());
        string index = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, index, "!reg"));
        varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
        cfg->current_bb->add_IRInstr(new IRInstrCopyMem(cfg->current_bb, index, varName));
        varName = cfg->create_new_tempvar(); // Reajust varName to store the table memory address to avoid duplicates if
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
    }

    if (ctx->opD()->PLUSPLUS()) {
        string tmp = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp, 1));

        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));

        // Add an addition instruction
        cfg->current_bb->add_IRInstr(new IRInstrAdd(cfg->current_bb, tmp, "!reg"));

        this->visit(ctx->lvalue());
    }

    else if (ctx->opD()->MOINSMOINS()) {
        string tmp = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp, 1));

        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));

        // Add a subtraction instruction
        cfg->current_bb->add_IRInstr(new IRInstrSub(cfg->current_bb, tmp, "!reg"));

        this->visit(ctx->lvalue());
    }

    return 0;
}
antlrcpp::Any Linearize::visitExprTable(ifccParser::ExprTableContext *ctx)
{
    this->visit(ctx->expr());
    string baseVarName =cfg->getVarName( ctx->VAR()->getText(),scopeString);
    string tmpIndex = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmpIndex, "!reg"));

    cfg->current_bb->add_IRInstr(new IRInstrCopyMem(cfg->current_bb, tmpIndex, baseVarName));
    return 0;
}

antlrcpp::Any Linearize::visitVarAffectation(ifccParser::VarAffectationContext *ctx) {
    string varName = cfg->getVarName(ctx->VAR()->getText(), scopeString);
    this->visit(ctx->expr());
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
    return 0;
}
antlrcpp::Any Linearize::visitExprShift(ifccParser::ExprShiftContext *ctx) {
    // Visit the first expression
    this->visit(ctx->expr(0));
    string tmp = cfg->create_new_tempvar();
    // Store the result in a temporary variable
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
    // Visit the second expression
    this->visit(ctx->expr(1));

    // Add a shift instruction depending on the operator
    if (ctx->opS()->SHL()) {
        cfg->current_bb->add_IRInstr(new IRInstrSHL(cfg->current_bb, "!reg", tmp));
    }
    else if (ctx->opS()->SHR()) {
        cfg->current_bb->add_IRInstr(new IRInstrSHR(cfg->current_bb, "!reg", tmp));
    }

    return nullptr;
}

antlrcpp::Any Linearize::visitExprAndBit(ifccParser::ExprAndBitContext *ctx) {
    // Visit the first expression
    this->visit(ctx->expr(0));
    string tmp = cfg->create_new_tempvar();
    // Store the result in a temporary variable
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
    // Visit the second expression
    this->visit(ctx->expr(1));
    cfg->current_bb->add_IRInstr(new IRInstrAndBit(cfg->current_bb, tmp, "!reg"));
    return nullptr;
}

antlrcpp::Any Linearize::visitExprOrBit(ifccParser::ExprOrBitContext *ctx) {
    // Visit the first expression
    this->visit(ctx->expr(0));
    string tmp = cfg->create_new_tempvar();
    // Store the result in a temporary variable
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
    // Visit the second expression
    this->visit(ctx->expr(1));
    cfg->current_bb->add_IRInstr(new IRInstrOrBit(cfg->current_bb, tmp, "!reg"));
    return nullptr;
}

antlrcpp::Any Linearize::visitExprXorBit(ifccParser::ExprXorBitContext *ctx) {
    // Visit the first expression
    this->visit(ctx->expr(0));
    string tmp = cfg->create_new_tempvar();
    // Store the result in a temporary variable
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
    // Visit the second expression
    this->visit(ctx->expr(1));
    cfg->current_bb->add_IRInstr(new IRInstrXorBit(cfg->current_bb, tmp, "!reg"));
    return nullptr;
}