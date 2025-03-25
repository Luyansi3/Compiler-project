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




antlrcpp::Any SymbolTableVisitor::visitCall(ifccParser::CallContext* ctx) {
    string label = ctx->VAR()->getText();
    int nbParams = ctx->liste_param()->expr().size();

    if (symbolTableFonction.find(label) == symbolTableFonction.end()) {
        FlagFonction flag = {declared = false, used = true, nombreParams = nbParams};
        symbolTableFonction[label] = flag;
    } else if (symbolTableFonction[label].nombreParams == nbParams) {
        symbolTableFonction[label].used = true;
    } else {
        cerr << "La fonction " << label << " est appelée avec le mauvais nb de params" << endl;
    }



    for (auto expression : ctx->liste_param()->expr()) {
        this->visit(expression);
    }

    return 0;
}

antlrcpp::Any ifccParser::visitDecl_fonction(ifccParser::Decl_fonction *ctx) {
    int nbParams = ctx->decl_sans_assignation()->VAR().size();
    string label = ctx->VAR()->getText();


    if (symbolTableFonction.find(label) != symbolTableFonction.end()) {
        if (symbolTableFonction[label].declared) {
            cerr << "La fonction " << label << " a déjà été déclarée" << endl;
            exit(1); 
        } else if (symbolTableFonction[label].nombreParams == nbParams) {
            symbolTableFonction[label].declared = true;
        } else {
            cerr << "La fonction " << label << " est appelée avec le mauvais nb d'arguments"
        }
    } else {
        FlagFonction flag = {declared = true, used = false, nombreParams = nbParams};
        symbolTableFonction[label] = flag;
    }
    unordered_map<string, FlagVar> myMap;
    symbolTableVar[label] = myMap;
    currentSymbolTable = symbolTableVar[label];
    int copyIndex = index;
    index = -4;
    this->visit(ctx->bloc());
    index = copyIndex;


    return 0;    
}


