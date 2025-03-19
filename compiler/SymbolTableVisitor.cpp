#include "SymbolTableVisitor.h"

antlrcpp::Any SymbolTableVisitor::visitDecl_element(ifccParser::Decl_elementContext *ctx) 
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
    this->visit(ctx->liste_decl());
    
    
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
    this->visit(ctx->expr());
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitExprVar(ifccParser::ExprVarContext *ctx){

    string varName = ctx->VAR()->getText();
    if(symbolTable.find(varName)!=symbolTable.end()){
        symbolTable[varName].used = true;
    }
    else{
        cerr << "Variable " << varName << " non déclarée" << endl;
        exit(1);
    }

    return 0;
}

