#include "CodeGenVisitor.h"

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
    std::cout<< ".globl main\n" ;
    std::cout<< " main: \n" ;
    std::cout<< "    pushq %rbp \n";
    std::cout<< "    movq %rsp, %rbp\n";

    this->visit(ctx->block());
    
    std::cout<< "    popq %rbp\n";
    std::cout << "    ret\n";

    return 0;
}


antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
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

antlrcpp::Any CodeGenVisitor::visitAffectation(ifccParser::AffectationContext *ctx)
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

antlrcpp::Any CodeGenVisitor::visitRvalue(ifccParser::RvalueContext *ctx){
    if(ctx->VAR()){
        string varName = ctx->VAR()->getText();
        std::cout << "    movl "<< symbolTable[varName].index <<"(%rbp), %eax\n" ;
    }
    else if(ctx->CONST()){
        int retval = stoi(ctx->CONST()->getText());
        std::cout << "    movl $"<< retval <<", %eax\n" ;
    }
    else if (ctx->affectation())
    {
        this->visit(ctx->affectation());
    }
    
    return 0;
}

antlrcpp::Any CodeGenVisitor::visitLvalue(ifccParser::LvalueContext *ctx){

    if(ctx->VAR()){
        string varName = ctx->VAR()->getText();
        std::cout << "    movl %eax, "<< symbolTable[varName].index <<"(%rbp)\n" ;
    }
    return 0;   
}
