#include "Linearize_optimized.h"



Linearize_optimized::Linearize_optimized(CFG* cfg): cfg(cfg){
    scopeString = cfg->getNameFunction();
    nameCurrentFunction = cfg->getNameFunction();
}


antlrcpp::Any Linearize_optimized::visitBlock(ifccParser::BlockContext *ctx) {

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
antlrcpp::Any Linearize_optimized::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    // Visit the expression in the return statement
    auto e = this->visit(ctx->expr());

    // If I get a constant I need to put it inside reg
    if (e.is<int>()) {
        int result = e.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!returnVal",result));
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!returnVal","!reg"));
    }
    cfg->current_bb->add_IRInstr(new IRInstrJump(cfg->current_bb,cfg->bb_epi->label));

    return 0;
}

// Visit an assignment
antlrcpp::Any Linearize_optimized::visitAffectation(ifccParser::AffectationContext *ctx)
{
    if (ctx->op_compose()->EQUAL())
    {
        // Visit the expression and the left-hand side of the assignment
        auto e = this->visit(ctx->expr());
        // If this is a constant that does not come from ConstContext we put it in reg
        // otherwise the constant is already inside reg

        if (e.is<int>()) {
            
            int result = e.as<int>();
            cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg",result));
        }
        this->visit(ctx->lvalue());
        
    }
    else
    {
        string varName;
        varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
        if (ctx->lvalue()->expr())
        {
            auto e = this->visit(ctx->lvalue()->expr());
            if (e.is<int>()) {
                int result = e.as<int>();
                cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", result));
            }

            varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
            cfg->current_bb->add_IRInstr(new IRInstrCopyMem(cfg->current_bb, "!reg", varName));
            varName = cfg->create_new_tempvar(); // Reajust varName to store the table memory address to avoid duplicates if
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
        }

        // Visit the expression and the left-hand side of the assignment
        auto e = this->visit(ctx->expr());
        if (e.is<int>()) {
            int result = e.as<int>();
            cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg",result));
        }
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
        this->visit(ctx->lvalue());
    }

    
    return nullptr;
}


antlrcpp::Any Linearize_optimized::visitTableAffectation(ifccParser::TableAffectationContext *ctx)
{
    int i = 0;
    string baseVarName = cfg->getVarName(ctx->VAR()->getText(), scopeString); // base du tableau

    for (auto &expr : ctx->array_litteral()->expr())
    {
        auto e = this->visit(expr);
        string tmpValue = cfg->create_new_tempvar();
        if (e.is<int>()) {
            int result = e.as<int>();
            cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmpValue, result));
        }
        else{
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmpValue, "!reg")); // stocker valeur dans tmpValue
        }
        

        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", i)); // charger i dans !reg

        // Utiliser tmpValue (la valeur) et !reg (l'index)
        cfg->current_bb->add_IRInstr(new IRInstrMem(cfg->current_bb, tmpValue, "!reg", baseVarName));
        i++;
    }

    while (ctx->constante() && i < stoi(ctx->constante()->getText()))
    {
        string tmpValue = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmpValue, 0)); // stocker valeur dans tmpValue

        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", i)); // charger i dans !reg

        // Utiliser tmpValue (la valeur) et !reg (l'index)
        cfg->current_bb->add_IRInstr(new IRInstrMem(cfg->current_bb, tmpValue, "!reg", baseVarName));
        i++;
    }

    return 0;
}

// Visit a variable expression
antlrcpp::Any Linearize_optimized::visitExprVar(ifccParser::ExprVarContext *ctx)
{
    string varName = cfg->getVarName(ctx->VAR()->getText(), scopeString);

    // Add a copy instruction to load the variable into a register
    cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", varName));

    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprUnary(ifccParser::ExprUnaryContext *ctx)
{
    auto e = this->visit(ctx->expr());
            
    if (ctx->opU()->MINUS())
    {
        if (e.is<int>()) {
            int result = e.as<int>();
            return (int) (-result);
        }
        cfg->current_bb->add_IRInstr(new IRInstrNeg(cfg->current_bb, "!reg"));
    }
    else if(ctx->opU()->NOT()){
        if (e.is<int>()) {
            int result = e.as<int>();
            return (int) (!result);
        }
        cfg->current_bb->add_IRInstr(new IRInstrNot(cfg->current_bb, "!reg"));
    }
    else if (ctx->opU()->PLUS())
    {
        if (e.is<int>()) {
            int result = e.as<int>();
            return (int) (result);
        }
    }

    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprPar(ifccParser::ExprParContext *ctx)
{
    auto e = this->visit(ctx->expr());
        
    if (e.is<int>()) {
        int result = e.as<int>();
        return (int) (result);
    }

    return nullptr;
}

// Visit a constant expression
antlrcpp::Any Linearize_optimized::visitExprConst(ifccParser::ExprConstContext *ctx)
{
    int retval;
    if (ctx->constante()->CONSTINT())
        retval = stoi(ctx->constante()->CONSTINT()->getText());
    else
        retval = (int)ctx->constante()->CONSTCHAR()->getText()[1];


    return (int) retval;
}

// Visit a left-hand side value
antlrcpp::Any Linearize_optimized::visitLvalue(ifccParser::LvalueContext *ctx)
{
    // // Visit the expression and the left-hand side of the assignment
    // // mov !reg , !tmp
    // this->visit(ctx->lvalue());
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

        auto e = this->visit(ctx->expr());
        string tmpIndex = cfg->create_new_tempvar();
        if (e.is<int>()) {
            int result = e.as<int>();
            cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", result));
        }

        string baseVarName = cfg->getVarName(ctx->VAR()->getText(), scopeString);
        
        // cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, "!reg", tmpValue));
        cfg->current_bb->add_IRInstr(new IRInstrMem(cfg->current_bb, tmpValue, "!reg", baseVarName));
    }
    return 0;
}

// Visit a multiplication or division expression
antlrcpp::Any Linearize_optimized::visitExprMulDivMod(ifccParser::ExprMulDivModContext *ctx)
{

    int resultLeft, resultRight;
    bool leftConst = false , rightConst= false;
    string tmp1;
    auto e2 = this->visit(ctx->expr()[1]);
    // If we get a constant value
    if (e2.is<int>()) {
        resultRight = e2.as<int>();
        rightConst= true;
    }
    else{
        tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    }
    
    auto e1 = this->visit(ctx->expr()[0]);        
    if (e1.is<int>()) {
        resultLeft = e1.as<int>();
        leftConst= true;
    }
        

    if (rightConst && resultRight==1 ) // If .. * or / 1
    {
        if (!leftConst) // if a * or /1 (value of a already in reg)
        {
            return nullptr;
        }
        
    }
    if (leftConst && resultLeft == 1) 
    {
        if (ctx->opM()->MULT() && !rightConst) // If  1*a // we just put a that is in tmp1 in reg 
        {
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb,"!reg" ,tmp1 ));
            return nullptr;
        }
        
    }



    if (leftConst && rightConst) // If we have both constant
    {
        if (ctx->opM()->MULT()) return (int) resultLeft * resultRight;
        else if (ctx->opM()->MOD())
        {
            if (resultRight == 0)
            {
                cerr << "Division par 0" << endl; 
                tmp1 = cfg->create_new_tempvar();
                cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, resultRight));
                cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", resultLeft));
                cfg->current_bb->add_IRInstr(new IRInstrMod(cfg->current_bb, tmp1));
                return nullptr;
                // exit(1);
            } 
            return (int) (resultLeft % resultRight);
        }
        
        else{
            if (resultRight == 0)
            {
                cerr << "Division par 0" << endl; 
                tmp1 = cfg->create_new_tempvar();
                cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, resultRight));
                cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", resultLeft));
                cfg->current_bb->add_IRInstr(new IRInstrDiv(cfg->current_bb, tmp1));
                return nullptr;
                // exit(1);
            } 
            return (int) (resultLeft / resultRight);
        }
    }
    else if (leftConst && !rightConst) // if only leftside is a constant  we put the result in reg
    {
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", resultLeft));
    }
    else if (!leftConst && rightConst) // if only rightside is a constant  we put the result in tmp1
    {
        tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, resultRight));
    }

    if (ctx->opM()->MULT()) cfg->current_bb->add_IRInstr(new IRInstrMul(cfg->current_bb, tmp1, "!reg")); // then we multiply tmp1 and reg
    else if (ctx->opM()->MOD())
    {
        cfg->current_bb->add_IRInstr(new IRInstrMod(cfg->current_bb, tmp1)); // we do reg % tmp1
    }
    
    else{
        cfg->current_bb->add_IRInstr(new IRInstrDiv(cfg->current_bb, tmp1)); // we divide reg by tmp1
    } 

    return nullptr;
}

// Visit an addition or subtraction expression
antlrcpp::Any Linearize_optimized::visitExprAddSub(ifccParser::ExprAddSubContext *ctx)
{
    // Visit the first expression
    int resultLeft, resultRight;
    bool leftConst = false , rightConst= false;
    string tmp1;
    
    auto e2 = this->visit(ctx->expr()[1]);
    // If we get a constant value
    if (e2.is<int>()) {
        resultRight = e2.as<int>();
        rightConst= true;
    }
    else{
        tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    }

    auto e1 = this->visit(ctx->expr()[0]);
    if (e1.is<int>()) {
        resultLeft = e1.as<int>();
        leftConst= true;
    }
    

    if (rightConst && resultRight==0 ) // If .. + or - 0
    {
        if (!leftConst) // if a + or - 0 (value of a already in reg)
        {
            return nullptr;
        }
        
    }
    if (leftConst && resultLeft == 0)
    {
        if (ctx->opA()->PLUS() && !rightConst) // If  0+a // we just put a that is in tmp1 in reg 
        {
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb,"!reg" ,tmp1 ));
            return nullptr;
        }
        
    }
    
    if (leftConst && rightConst) // If we have both constant
    {
        if (ctx->opA()->PLUS()) return (int) resultLeft + resultRight;
        else return (int) (resultLeft - resultRight); 
        
    }
    else if (leftConst && !rightConst) // if only leftside is a constant  we put the result in reg
    {
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", resultLeft)); 
   
    }
    else if (!leftConst && rightConst)  // if only rightside is a constant  we put the result in tmp1
    {
        tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, resultRight));
        
    }

    if (ctx->opA()->PLUS()) cfg->current_bb->add_IRInstr(new IRInstrAdd(cfg->current_bb, tmp1, "!reg")); // then we add tmp1 and reg
    else{
        cfg->current_bb->add_IRInstr(new IRInstrSub(cfg->current_bb, tmp1, "!reg")); // then we sub  reg - tmp1
    } 


    return nullptr;
}





antlrcpp::Any Linearize_optimized::visitCall(ifccParser::CallContext *ctx) {
    //Obtention du label de la fonction
    string label = ctx->VAR()->getText();
    vector<string> params;

    //Obtention des params
    for (auto expression : ctx->liste_param()->expr()) {
        string tmp = cfg->create_new_tempvar();
        auto e = this->visit(expression);
        if (e.is<int>()) {
            int result = e.as<int>();
            cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp, result));
        }
        else{
            cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp, "!reg"));
        }
        params.push_back(tmp);
    }

    //Ajouter l'instruction call
    cfg->current_bb->add_IRInstr(new IrInstrCall(cfg->current_bb, label, params));
    
   
    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprCompRelationnal(ifccParser::ExprCompRelationnalContext *ctx)
{
    ifccParser::CompRelationnalContext *ctxComp = ctx->compRelationnal();
    ifccParser::ExprContext *ctxExpr1 = ctx->expr(0);
    ifccParser::ExprContext *ctxExpr2 = ctx->expr(1);

    string tmp1 = cfg->create_new_tempvar();
    auto e2 = this->visit(ctxExpr2);
    if (e2.is<int>()) {
        int result = e2.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, result));
    }
    else{
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    }
    
    // Visit the first expression

    auto e1 = this->visit(ctxExpr1);
    if (e1.is<int>()) {
        int result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", result));
    }


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
    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprCompEqual(ifccParser::ExprCompEqualContext *ctx)
{
    ifccParser::CompEqualContext *ctxComp = ctx->compEqual();
    ifccParser::ExprContext *ctxExpr1 = ctx->expr(0);
    ifccParser::ExprContext *ctxExpr2 = ctx->expr(1);

    // Visit the second expression
    string tmp1 = cfg->create_new_tempvar();
    auto e2 = this->visit(ctxExpr2);
    if (e2.is<int>()) {
        int result = e2.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, result));
    }
    else{
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    }
    
    // Visit the first expression

    auto e1 = this->visit(ctxExpr1);
    if (e1.is<int>()) {
        int result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", result));
    }

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
    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitIf_stmt(ifccParser::If_stmtContext *ctx)
{
    BasicBlock *bb_init = cfg->current_bb;
    // Visit the condition expression
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    bool resultConst = false;
    int result;
    auto e1 = this->visit(ctx->expr());
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    if (e1.is<int>()) {
        result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, cfg->current_bb->test_var_name, result));
        resultConst = true;
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    }
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
    if(resultConst)
    {
        if (result)
        {
            // Visit the then part
            if (ctx->instruction()) {
                this->visit(ctx->instruction());
            }
            else {
                this->visit(ctx->block());
            }
            return nullptr; // we dont need to execute the rest because the first if is always true
        }   
    }
    else{
        // Visit the then part
        if (ctx->instruction()) {
            this->visit(ctx->instruction());
        }
        else {
            this->visit(ctx->block());
        }
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

antlrcpp::Any Linearize_optimized::visitElif_stmt(ifccParser::Elif_stmtContext *ctx)
{
    // Visit the condition expression

    auto e1 = this->visit(ctx->expr());
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    if (e1.is<int>()) {
        int result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, cfg->current_bb->test_var_name, result));
        if (!result) return nullptr;
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    }
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

antlrcpp::Any Linearize_optimized::visitElse_stmt(ifccParser::Else_stmtContext *ctx)
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

antlrcpp::Any Linearize_optimized::visitExprAnd(ifccParser::ExprAndContext *ctx) {
    // Visit the condition expression
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    auto e1 = this->visit(ctx->expr(0));
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    if (e1.is<int>()) {
        int result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, cfg->current_bb->test_var_name, result));
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    }

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
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    auto e2 = this->visit(ctx->expr(1));
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    if (e2.is<int>()) {
        int result = e2.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, cfg->current_bb->test_var_name, result));
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    }

    bb_end->exit_true = cfg->current_bb->exit_true;
    bb_end->exit_false = cfg->current_bb->exit_false;
    cfg->current_bb->exit_true = bb_true;
    cfg->current_bb->exit_false = bb_false;
    bb_false->exit_true = bb_end;
    bb_true->exit_true = bb_end;
    bb_true->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", 1));
    bb_false->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", 0));
    cfg->current_bb = bb_end;
    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprOr(ifccParser::ExprOrContext *ctx) {
    // Visit the condition expression
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    auto e1 = this->visit(ctx->expr(0));
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    if (e1.is<int>()) {
        int result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, cfg->current_bb->test_var_name, result));
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    }
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
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    auto e2 = this->visit(ctx->expr(1));
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    if (e2.is<int>()) {
        int result = e2.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, cfg->current_bb->test_var_name, result));
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    }
    bb_end->exit_true = cfg->current_bb->exit_true;
    bb_end->exit_false = cfg->current_bb->exit_false;
    cfg->current_bb->exit_true = bb_true;
    cfg->current_bb->exit_false = bb_false;
    bb_false->exit_true = bb_end;
    bb_true->exit_true = bb_end;
    bb_true->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", 1));
    bb_false->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", 0));
    cfg->current_bb = bb_end;
    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitWhile_stmt(ifccParser::While_stmtContext *ctx)
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
    auto e1 = this->visit(ctx->expr());
    cfg->current_bb->test_var_name = cfg->create_new_tempvar();
    if (e1.is<int>()) {
        int result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, cfg->current_bb->test_var_name, result));
        if (result) cerr << "Risque de boucle infinie" << endl;
        else{
            return nullptr;
        }
        
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, cfg->current_bb->test_var_name, "!reg"));
    }
    

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

antlrcpp::Any Linearize_optimized::visitExprSuffixe(ifccParser::ExprSuffixeContext *ctx)
{
    string varName;
    varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
    if (ctx->lvalue()->expr()) {
        auto e = this->visit(ctx->lvalue()->expr());
        if (e.is<int>()) {
            int result = e.as<int>();
            cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", result));
        }
        varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
        cfg->current_bb->add_IRInstr(new IRInstrCopyMem(cfg->current_bb, "!reg", varName));
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

    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprPrefixe(ifccParser::ExprPrefixeContext *ctx) {
    string varName;
    varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
    if (ctx->lvalue()->expr()) {
        auto e = this->visit(ctx->lvalue()->expr());
        if (e.is<int>()) {
            int result = e.as<int>();
            cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", result));
        }
        varName = cfg->getVarName(ctx->lvalue()->VAR()->getText(), scopeString);
        cfg->current_bb->add_IRInstr(new IRInstrCopyMem(cfg->current_bb, "!reg", varName));
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

    return nullptr;
}
antlrcpp::Any Linearize_optimized::visitExprTable(ifccParser::ExprTableContext *ctx)
{

    auto e1 = this->visit(ctx->expr());
    string tmpIndex = cfg->create_new_tempvar();
    if (e1.is<int>()) {
        int result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", result));
    }

    string baseVarName =cfg->getVarName( ctx->VAR()->getText(),scopeString);

    cfg->current_bb->add_IRInstr(new IRInstrCopyMem(cfg->current_bb, "!reg", baseVarName));
    return nullptr;
}



antlrcpp::Any Linearize_optimized::visitVarAffectation(ifccParser::VarAffectationContext *ctx) {
    string varName = cfg->getVarName(ctx->VAR()->getText(), scopeString);
    auto e1 = this->visit(ctx->expr());
    if (e1.is<int>()) {
        int result = e1.as<int>();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, varName, result));
    }
    else{
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, varName, "!reg"));
    }
    return 0;
}



antlrcpp::Any Linearize_optimized::visitExprShift(ifccParser::ExprShiftContext *ctx) {

    int resultLeft, resultRight;
    bool leftConst = false , rightConst= false;
    string tmp1;
    
    auto e2 = this->visit(ctx->expr()[1]);
    // If we get a constant value
    if (e2.is<int>()) {
        resultRight = e2.as<int>();
        rightConst= true;
    }
    else{
        tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    }

    auto e1 = this->visit(ctx->expr()[0]);
    if (e1.is<int>()) {
        resultLeft = e1.as<int>();
        leftConst= true;
    }
    

    // if (rightConst && resultRight==0 ) // If .. + or - 0
    // {
    //     if (!leftConst) // if a + or - 0 (value of a already in reg)
    //     {
    //         return nullptr;
    //     }
        
    // }
    // if (leftConst && resultLeft == 0)
    // {
    //     if (ctx->opA()->PLUS() && !rightConst) // If  0+a // we just put a that is in tmp1 in reg 
    //     {
    //         cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb,"!reg" ,tmp1 ));
    //         return nullptr;
    //     }
        
    // }
    
    if (leftConst && rightConst) // If we have both constant
    {
        if (ctx->opS()->SHL()) return (int) resultLeft <<  resultRight;
        else if (ctx->opS()->SHR()) return (int) resultLeft >>  resultRight;
        
    }
    else if (leftConst && !rightConst) // if only leftside is a constant  we put the result in reg
    {
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", resultLeft)); 
   
    }
    else if (!leftConst && rightConst)  // if only rightside is a constant  we put the result in tmp1
    {
        tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, resultRight));
        
    }


    if (ctx->opS()->SHL()) {
        cfg->current_bb->add_IRInstr(new IRInstrSHL(cfg->current_bb, tmp1, "!reg"));
    }
    else if (ctx->opS()->SHR()) {
        cfg->current_bb->add_IRInstr(new IRInstrSHR(cfg->current_bb, tmp1, "!reg"));
    }

    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprAndBit(ifccParser::ExprAndBitContext *ctx) {

    int resultLeft, resultRight;
    bool leftConst = false , rightConst= false;
    string tmp1;
    
    auto e1 = this->visit(ctx->expr(0));
    // If we get a constant value
    if (e1.is<int>()) {
        resultLeft = e1.as<int>();
        leftConst= true;
    }
    else{
        tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    }

    auto e2 = this->visit(ctx->expr(1));
    if (e2.is<int>()) {
        resultRight = e2.as<int>();
        rightConst= true;
    }

    if (leftConst && rightConst) // If we have both constant
    {
        return resultLeft & resultRight;
        
    }
    else if (!leftConst && rightConst) // if only rightside  is a constant  we put the result in reg
    {
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", resultRight)); 
   
    }
    else if (leftConst && !rightConst)  // if only leftside is a constant  we put the result in tmp1
    {
        tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, resultLeft));
        
    }


    cfg->current_bb->add_IRInstr(new IRInstrAndBit(cfg->current_bb, tmp1, "!reg"));
    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprOrBit(ifccParser::ExprOrBitContext *ctx) {

    int resultLeft, resultRight;
    bool leftConst = false , rightConst= false;
    string tmp1;
    
    auto e1 = this->visit(ctx->expr(0));
    // If we get a constant value
    if (e1.is<int>()) {
        resultLeft = e1.as<int>();
        leftConst= true;
    }
    else{
        tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    }

    auto e2 = this->visit(ctx->expr(1));
    if (e2.is<int>()) {
        resultRight = e2.as<int>();
        rightConst= true;
    }

    if (leftConst && rightConst) // If we have both constant
    {
        return resultLeft | resultRight;
        
    }
    else if (!leftConst && rightConst) // if only rightside  is a constant  we put the result in reg
    {
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", resultRight)); 
   
    }
    else if (leftConst && !rightConst)  // if only leftside is a constant  we put the result in tmp1
    {
        tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, resultLeft));
        
    }

    cfg->current_bb->add_IRInstr(new IRInstrOrBit(cfg->current_bb, tmp1, "!reg"));

    return nullptr;
}

antlrcpp::Any Linearize_optimized::visitExprXorBit(ifccParser::ExprXorBitContext *ctx) {

    int resultLeft, resultRight;
    bool leftConst = false , rightConst= false;
    string tmp1;
    
    auto e1 = this->visit(ctx->expr(0));
    // If we get a constant value
    if (e1.is<int>()) {
        resultLeft = e1.as<int>();
        leftConst= true;
    }
    else{
        tmp1 = cfg->create_new_tempvar();
        // Add a copy instruction to store the result in a temporary variable
        cfg->current_bb->add_IRInstr(new IRInstrCopy(cfg->current_bb, tmp1, "!reg"));
    }

    auto e2 = this->visit(ctx->expr(1));
    if (e2.is<int>()) {
        resultRight = e2.as<int>();
        rightConst= true;
    }

    if (leftConst && rightConst) // If we have both constant
    {
        return resultLeft ^ resultRight;
        
    }
    else if (!leftConst && rightConst) // if only rightside  is a constant  we put the result in reg
    {
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, "!reg", resultRight)); 
   
    }
    else if (leftConst && !rightConst)  // if only leftside is a constant  we put the result in tmp1
    {
        tmp1 = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(new IRInstrLDConst(cfg->current_bb, tmp1, resultLeft));
        
    }

    cfg->current_bb->add_IRInstr(new IRInstrXorBit(cfg->current_bb, tmp1, "!reg"));
    return nullptr;
}