#include "SymbolTableVisitor.h"

antlrcpp::Any SymbolTableVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx) 
{
    if(ctx->affectation()){
        string var = ctx->affectation()->lvalue()->VAR()->getText();
        if(symbolTable.find(var) == symbolTable.end()){
            Flag flag;
            flag.used = false;
            flag.affected = false;
            flag.index = index;
            symbolTable.insert({var, flag});
            index -= 4;
        }
        else{
            cerr<< "Variable " << var << " deja déclaré" << endl;
            exit(1);
        }
        this->visit(ctx->affectation());

    }
    else{
        string var = ctx->VAR()->getText();
        if(symbolTable.find(var) == symbolTable.end()){
            Flag flag;
            flag.used = false;
            flag.affected = false;
            flag.index = index;
            symbolTable.insert({var, flag});
            index -= 4;
        }
        else{
            cerr<< "Variable " << var << " deja déclaré" << endl;
            exit(1);
        }
    }
    
    
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitAffectation(ifccParser::AffectationContext *ctx){
    string varName = ctx->lvalue()->VAR()->getText();
    if(symbolTable.find(varName)==symbolTable.end()){
        cerr << "Variable " << varName << " non déclarée" << endl;
        exit(1);
    }
    else{
        symbolTable[varName].affected = true;
    }
    this->visit(ctx->lvalue());
    this->visit(ctx->rvalue());
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitRvalue(ifccParser::RvalueContext *ctx){
    // cout << "VAR " << ctx->VAR()->getText() << endl;
    if(ctx->VAR()){
        string varName = ctx->VAR()->getText();
        if(symbolTable.find(varName)!=symbolTable.end()){
            symbolTable[varName].used = true;
        }
        else{
            cerr << "Variable " << varName << " non déclarée" << endl;
            exit(1);
        }
    }
    if(ctx->affectation()){
        this->visit(ctx->affectation());
    }

    return 0;
}

