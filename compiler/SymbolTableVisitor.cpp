#include "SymbolTableVisitor.h"

antlrcpp::Any SymbolTableVisitor::visitProg(ifccParser::ProgContext *ctx)
{

    Flag flag;
    flag.used = false;
    flag.affected = true;
    flag.nombreParams = 1;
    symbolTable.insert({"putchar", flag});

    flag.nombreParams = 0;
    symbolTable.insert({"getchar", flag});

    this->visit(ctx->block());

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitDecl_element(ifccParser::Decl_elementContext *ctx)
{
    if (ctx->affectation() && dynamic_cast<ifccParser::LvalueAffectationContext *>(ctx->affectation()))
    {

        auto affect = dynamic_cast<ifccParser::LvalueAffectationContext *>(ctx->affectation());
        string var = affect->lvalue()->VAR()->getText();
        if (symbolTable.find(var) == symbolTable.end())
        {
            Flag flag;
            flag.used = false;
            flag.affected = false;
            flag.index = index;
            flag.nombreParams = -1;
            symbolTable.insert({var, flag});
            index -= 4;
        }
        else
        {
            cerr << "Variable " << var << " deja déclaré" << endl;
            exit(1);
        }
        this->visit(ctx->affectation());
    }
    else if (ctx->affectation() && dynamic_cast<ifccParser::TableAffectationContext *>(ctx->affectation()))
    {
        auto affect = dynamic_cast<ifccParser::TableAffectationContext *>(ctx->affectation());
        string baseVar = affect->VAR()->getText();
        int tableSize = stoi(affect->constante()->getText());
        // Iterate in reverse so that element 0 is allocated last, receiving the smallest index.

        if (symbolTable.find(baseVar) == symbolTable.end())
        {
            Flag flag;
            flag.used = false;
            flag.affected = false;
            index -= (tableSize-1)*4; // allocate 4 bytes per element
            flag.index = index;
            flag.nombreParams = -1;
            symbolTable.insert({baseVar, flag});
            index-=4;
        }
        else
        {
            cerr << "Variable " << baseVar << " deja déclaré" << endl;
            exit(1);
        }

        this->visit(ctx->affectation());
    }
    else
    {
        string var = ctx->VAR()->getText();
        if (symbolTable.find(var) == symbolTable.end())
        {
            Flag flag;
            flag.used = false;
            flag.affected = false;
            flag.index = index;
            flag.nombreParams = -1;
            symbolTable.insert({var, flag});
            index -= 4;
        }
        else
        {
            cerr << "Variable " << var << " deja déclaré" << endl;
            exit(1);
        }
    }

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitLvalueAffectation(ifccParser::LvalueAffectationContext *ctx)
{
    string varName = ctx->lvalue()->VAR()->getText();
    if (symbolTable.find(varName) == symbolTable.end())
    {
        cerr << "Variable " << varName << " non déclarée" << endl;
        exit(1);
    }
    else
    {
        symbolTable[varName].affected = true;
    }
    this->visit(ctx->lvalue());
    this->visit(ctx->expr());
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitTableAffectation(ifccParser::TableAffectationContext *ctx)
{
    string varName = ctx->VAR()->getText();
    if (symbolTable.find(varName) == symbolTable.end())
    {
        cerr << "Variable " << ctx->VAR()->getText() << " non déclarée" << endl;
        exit(1);
    }
    else
    {
        int exprListSize = (ctx->array_litteral()->expr()).size();
        int constante = stoi(ctx->constante()->getText());

        for (int i = 0; i < min(constante, exprListSize); i++)
            symbolTable[ctx->VAR()->getText()].affected = true;
    }
    this->visit(ctx->array_litteral());
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitArray_litteral(ifccParser::Array_litteralContext *ctx)
{
    for (auto &expression : ctx->expr())
    {
        this->visit(expression);
    }
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitExprVar(ifccParser::ExprVarContext *ctx)
{

    string varName = ctx->VAR()->getText();
    if (symbolTable.find(varName) != symbolTable.end())
    {
        symbolTable[varName].used = true;
    }
    else
    {
        cerr << "Variable " << varName << " non déclarée" << endl;
        exit(1);
    }

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitCall(ifccParser::CallContext *ctx)
{
    string funcname = ctx->VAR()->getText();

    if (symbolTable.find(funcname) == symbolTable.end())
    {
        cerr << "The fonction " << funcname << " is never declared" << endl;
        exit(1);
    }
    else
    {
        symbolTable[funcname].used = true;
    }

    int expectedNombreParams = symbolTable[funcname].nombreParams;
    int actualNombreParams = ctx->liste_param()->expr().size();

    for (auto &expression : ctx->liste_param()->expr())
    {
        this->visit(expression);
    }

    if (actualNombreParams != expectedNombreParams)
    {
        cerr << "La fonction " << funcname << " n'a pas le bon nombre de paramètres" << endl;
        exit(1);
    }

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitExprConst(ifccParser::ExprConstContext *ctx)
{
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitMulDiv(ifccParser::MulDivContext *ctx)
{
    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    this->visit(expr2);
    this->visit(expr1);
    return 0;
}
antlrcpp::Any SymbolTableVisitor::visitAddSub(ifccParser::AddSubContext *ctx)
{
    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    this->visit(expr2);
    this->visit(expr1);
    return 0;
}
antlrcpp::Any SymbolTableVisitor::visitExprUnary(ifccParser::ExprUnaryContext *ctx)
{
    auto expr1 = ctx->expr();
    this->visit(expr1);
    return 0;
}
antlrcpp::Any SymbolTableVisitor::visitExprCompRelationnal(ifccParser::ExprCompRelationnalContext *ctx)
{
    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    this->visit(expr2);
    this->visit(expr1);
    return 0;
}
antlrcpp::Any SymbolTableVisitor::visitExprCompEqual(ifccParser::ExprCompEqualContext *ctx)
{
    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    this->visit(expr2);
    this->visit(expr1);
    return 0;
}
antlrcpp::Any SymbolTableVisitor::visitExprAnd(ifccParser::ExprAndContext *ctx)
{
    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    this->visit(expr2);
    this->visit(expr1);
    return 0;
}
antlrcpp::Any SymbolTableVisitor::visitExprOr(ifccParser::ExprOrContext *ctx)
{
    auto expr1 = ctx->expr(0);
    auto expr2 = ctx->expr(1);

    this->visit(expr2);
    this->visit(expr1);
    return 0;
}