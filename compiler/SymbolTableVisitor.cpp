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


    
    //Visit all the fonctions declarations
    for (auto decl_fonction : ctx->pre_decl_fonction())
        this->visit(decl_fonction);


    //Visiting the main block and creating its CFG
    unordered_map<string, FlagVar> newSymbolTable;
    index = -4;
    symbolTableVar = newSymbolTable;
    this->visit(ctx->block());
    CFG* cfg = new CFG(symbolTableVar, "main", ctx->block(), 0);
    cfg_liste.push_back(cfg);


    //Visit all the fonctions declarations
    for (auto decl_fonction : ctx->post_decl_fonction())
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
        flag.type = "";
        symbolTableFonction.insert({label, flag});
        cerr << "La fonction " << label << " est appelée avant d'être déclarée" << endl;
    } else if (symbolTableFonction[label].nombreParams != nbParams) {
        cerr << "La fonction " << label << " est appelée avec le mauvais nb de params" << endl;
        exit(1);
    } else {
        symbolTableFonction[label].used = true;
    }

    for (auto expression : ctx->liste_param()->expr()) {
        this->visit(expression);
    }

    return 0;
}


// Visit the declaration of a function 
antlrcpp::Any SymbolTableVisitor::visitPre_decl_fonction(ifccParser::Pre_decl_fonctionContext *ctx) {
    int nbParams = ctx->decl_params()->decl_param().size();
    string label = ctx->VAR()->getText();
    string type = ctx->typeFonc()->getText();


    if (symbolTableFonction.find(label) != symbolTableFonction.end()) {
        if (symbolTableFonction[label].declared) {
            cerr << "La fonction " << label << " a déjà été déclarée" << endl;
            exit(1); 
        } else if (symbolTableFonction[label].nombreParams != nbParams) {
            cerr << "La fonction " << label << " est appelée avec le mauvais nb d'arguments" << endl;
        } else if (symbolTableFonction[label].type != "" && symbolTableFonction[label].type != type) {
            cerr << "La fonction " << label << " est appelée avec le mauvais type de retour" << endl;
        } else if (symbolTableFonction[label].type == ""){
            symbolTableFonction[label].declared = true;
            symbolTableFonction[label].type = type;         
        } else {
            symbolTableFonction[label].declared = true;
        }
    } else {
        FlagFonction flag;
        flag.used = false;
        flag.declared = true;
        flag.nombreParams = nbParams;
        flag.type = type;
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


// Visit the declaration of a function 
antlrcpp::Any SymbolTableVisitor::visitPost_decl_fonction(ifccParser::Post_decl_fonctionContext *ctx) {
    int nbParams = ctx->decl_params()->decl_param().size();
    string label = ctx->VAR()->getText();
    string type = ctx->typeFonc()->getText();


    if (symbolTableFonction.find(label) != symbolTableFonction.end()) {
        if (symbolTableFonction[label].declared) {
            cerr << "La fonction " << label << " a déjà été déclarée" << endl;
            exit(1); 
        } else if (symbolTableFonction[label].nombreParams != nbParams) {
            cerr << "La fonction " << label << " est appelée avec le mauvais nb d'arguments" << endl;
        } else if (symbolTableFonction[label].type != "" && symbolTableFonction[label].type != type) {
            cerr << "La fonction " << label << " est appelée avec le mauvais type de retour" << endl;
        } else if (symbolTableFonction[label].type == ""){
            symbolTableFonction[label].declared = true;
            symbolTableFonction[label].type = type;         
        } else {
            symbolTableFonction[label].declared = true;
        }
    } else {
        FlagFonction flag;
        flag.used = false;
        flag.declared = true;
        flag.nombreParams = nbParams;
        flag.type = type;
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


antlrcpp::Any SymbolTableVisitor::visitExprCall(ifccParser::ExprCallContext *ctx) {
    ifccParser::CallContext* callctx = ctx->call();

    string label = callctx->VAR()->getText();
    int nbParams = callctx->liste_param()->expr().size();

    if (symbolTableFonction.find(label) == symbolTableFonction.end()) {
        FlagFonction flag;
        flag.declared = false;
        flag.used = true;
        flag.nombreParams = nbParams;
        flag.type = "int";
        symbolTableFonction.insert({label, flag});
        cerr << "La fonction " << label << " est appelée avant d'être déclarée" << endl;
    } else if (symbolTableFonction[label].type == "") {
        symbolTableFonction[label].type = "int";
    } else if (symbolTableFonction[label].type == "void") {
        cerr << "La fonction " << label << " est appelé avec le mauvais type de retour" << endl;
        exit(1);
    }


    this->visit(callctx);
    return 0;
}


ifccParser::CallContext* SymbolTableVisitor::isASimpleCall(ifccParser::ExprParContext *ctx) {
    if (auto callCtx = dynamic_cast<ifccParser::ExprCallContext *>(ctx->expr())) {
        return callCtx->call();
    } else if (auto parCtx = dynamic_cast<ifccParser::ExprParContext*>(ctx->expr())) {
        return isASimpleCall(parCtx);
    } else {
        return nullptr;
    }
}



antlrcpp::Any SymbolTableVisitor::visitExprInstr(ifccParser::ExprInstrContext *ctx) {
    if (auto parCtx = dynamic_cast<ifccParser::ExprParContext *>(ctx->expr())) {
        ifccParser::CallContext* callCtx = isASimpleCall(parCtx);
        this->visit(callCtx);
    } else {
        this->visit(ctx->expr());
    }     
    

    return 0;
}
