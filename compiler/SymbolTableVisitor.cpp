#include "SymbolTableVisitor.h"

//Declaration of the static attributes
unordered_map<string, FlagFonction> SymbolTableVisitor::symbolTableFonction;
vector<CFG*> SymbolTableVisitor::cfg_liste;

//Visit the program
antlrcpp::Any SymbolTableVisitor::visitProg(ifccParser::ProgContext *ctx) {
    //Inserting putchar and getchar into the fonction symbol table    
    FlagFonction flag;
    flag.used = false;
    flag.declared = true;
    flag.nombreParams = 1;
    symbolTableFonction.insert({"putchar", flag});

    flag.nombreParams = 0;
    symbolTableFonction.insert({"getchar", flag});


    //Visiting the main block and creating its CFG
    this->visit(ctx->block());
    CFG* cfg = new CFG(symbolTableVar, "main", ctx->block(), 0);
    cfg_liste.push_back(cfg);


    //Visit all the fonctions declarations
    for (auto decl_fonction : ctx->decl_fonction())
        this->visit(decl_fonction);


    return 0;
}

// Visit the declaration of the elements 
antlrcpp::Any SymbolTableVisitor::visitDecl_element(ifccParser::Decl_elementContext *ctx) 
{
    if(ctx->affectation()){
        string var = ctx->affectation()->lvalue()->VAR()->getText();
        if(symbolTableVar.find(var) == symbolTableVar.end()){
            FlagVar flagVar;
            flagVar.used = false;
            flagVar.affected = false;
            flagVar.index = index;
            symbolTableVar.insert({var, flagVar});
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
        if(symbolTableVar.find(var) == symbolTableVar.end()){
            FlagVar flagVar;
            flagVar.used = false;
            flagVar.affected = false;
            flagVar.index = index;
            symbolTableVar.insert({var, flagVar});
            index -= 4;
        }
        else{
            cerr<< "Variable " << var << " deja déclaré" << endl;
            exit(1);
        }
    } 
    
    return 0;
}


//Visit the affectation
antlrcpp::Any SymbolTableVisitor::visitAffectation(ifccParser::AffectationContext *ctx){
    string varName = ctx->lvalue()->VAR()->getText();    
    if(symbolTableVar.find(varName)==symbolTableVar.end()){
        cerr << "Variable " << varName << " non déclarée" << endl;
        exit(1);
    }
    else{
        symbolTableVar[varName].affected = true;
    }
    this->visit(ctx->lvalue());
    this->visit(ctx->expr());
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitExprVar(ifccParser::ExprVarContext *ctx){

    string varName = ctx->VAR()->getText();
    if(symbolTableVar.find(varName)!=symbolTableVar.end()){
        symbolTableVar[varName].used = true;
    }
    else{
        cerr << "Variable " << varName << " non déclarée" << endl;
        exit(1);
    }

    return 0;
}



// Visit the call of a function
antlrcpp::Any SymbolTableVisitor::visitCall(ifccParser::CallContext* ctx) {
    string label = ctx->VAR()->getText();
    int nbParams = ctx->liste_param()->expr().size();

    if (symbolTableFonction.find(label) == symbolTableFonction.end()) {
        FlagFonction flag;
        flag.declared = false;
        flag.used = true;
        flag.nombreParams = nbParams;
        symbolTableFonction.insert({label, flag});
        cerr << "La fonction " << label << " est appelée avant d'être déclarée" << endl;
    } else if (symbolTableFonction[label].nombreParams == nbParams) {
        symbolTableFonction[label].used = true;
    } else {
        cerr << "La fonction " << label << " est appelée avec le mauvais nb de params" << endl;
        exit(1);
    }

    for (auto expression : ctx->liste_param()->expr()) {
        this->visit(expression);
    }

    return 0;
}


// Visit the declaration of a function 
antlrcpp::Any SymbolTableVisitor::visitDecl_fonction(ifccParser::Decl_fonctionContext *ctx) {
    int nbParams = ctx->decl_params()->decl_param().size();
    string label = ctx->VAR()->getText();


    if (symbolTableFonction.find(label) != symbolTableFonction.end()) {
        if (symbolTableFonction[label].declared) {
            cerr << "La fonction " << label << " a déjà été déclarée" << endl;
            exit(1); 
        } else if (symbolTableFonction[label].nombreParams == nbParams) {
            symbolTableFonction[label].declared = true;
        } else {
            cerr << "La fonction " << label << " est appelée avec le mauvais nb d'arguments" << endl;
            exit(1);
        }
    } else {
        FlagFonction flag;
        flag.used = false;
        flag.declared = true;
        flag.nombreParams = nbParams;
        symbolTableFonction.insert({label, flag});
    }
    
    //Reinitialize the var symbol table and so the index
    unordered_map<string, FlagVar> newSymbolTable;
    index = -4;
    symbolTableVar = newSymbolTable;


    // Visting the params and block associated with the fonc and then creating its own CFG
    this->visit(ctx->decl_params());
    this->visit(ctx->block());
    CFG* cfg = new CFG(symbolTableVar, label, ctx->block(), nbParams);
    cfg_liste.push_back(cfg);

    return 0;    
}


// Visit the declaration of the params of a function
antlrcpp::Any SymbolTableVisitor::visitDecl_param(ifccParser::Decl_paramContext *ctx) {
    string var = ctx->VAR()->getText();
    if(symbolTableVar.find(var) == symbolTableVar.end()){
        FlagVar flagVar;
        flagVar.used = false;
        flagVar.affected = false;
        flagVar.index = index;
        symbolTableVar.insert({var, flagVar});
        index -= 4;
    }
    else{
        cerr<< "Variable " << var << " deja déclaré" << endl;
        exit(1);
    }

    return 0;
}


