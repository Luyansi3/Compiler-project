#include "SymbolTableVisitor.h"



antlrcpp::Any SymbolTableVisitor::visitProg(ifccParser::ProgContext *ctx) {
    
    
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
    if(ctx->affectation()){
        string var = ctx->affectation()->lvalue()->VAR()->getText();
        if(symbolTable.find(var) == symbolTable.end()){
            Flag flag;
            flag.used = false;
            flag.affected = false;
            flag.index = index;
            flag.nombreParams = -1;
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
            flag.nombreParams = -1;
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

antlrcpp::Any SymbolTableVisitor::visitExprCall(ifccParser::ExprCallContext *ctx) {
    this->visit(ctx->call());


    return 0;
}


antlrcpp::Any SymbolTableVisitor::visitCall(ifccParser::CallContext* ctx) {
    string funcname = ctx->funcname()->getText();

    if (symbolTable.find(funcname) == symbolTable.end()) {
        cerr << "The fonction " << funcname << " is never declared" << endl;
        exit(1);
    } else {
        symbolTable[funcname].used = true;
    }

    int expectedNombreParams = symbolTable[funcname].nombreParams;
    int actualNombreParams = 0;

    //Mettre le contexte  celui du param
    ifccParser::ParamsContext* ctxParam = ctx->params();

    if (ctxParam->expr()) {
        actualNombreParams++;
        ifccParser::ExprContext* expression = ctxParam->expr();
        this->visit(expression);
        
        
        ifccParser::Liste_paramContext *liste = ctxParam->liste_param();

        //Parcours de la liste de paramètres et évaluer chacun des expressions.
        while (liste != nullptr && liste->expr() != nullptr) {
            actualNombreParams++;
            expression = liste->expr();
            this->visit(expression);
            liste = liste->liste_param();
        }
    }

    if (actualNombreParams != expectedNombreParams) {
        cerr << "La fonction " << funcname << " n'a pas le bon nombre de paramètres" << endl;
        exit(1);
    }

    return 0;
}

