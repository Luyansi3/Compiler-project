#include "SymbolTableVisitor.h"

//Declaration of the static attributes
unordered_map<string, FlagFonction> SymbolTableVisitor::symbolTableFonction;
vector<CFG*> SymbolTableVisitor::cfg_liste;

antlrcpp::Any SymbolTableVisitor::visitBlock(ifccParser::BlockContext *ctx) {

    // if this the first time we enter in the block higher
    if(scope.find(scopeString) == scope.end()){
        scope.insert({scopeString, 0});   
    }
    // increase the counter of block inside the block higher
    scope[scopeString]++;
    scopeString += "_"+to_string(scope[scopeString]);


    this->visit(ctx->instructions());


    size_t pos = scopeString.find_last_of('_');
    //once we are done with the block we delete his counter
    scope.erase(scopeString);
    // then we delete the last part of the string, looks like this before : x!main_1_1
    if (pos != string::npos) {
        scopeString.erase(pos);
    }
    // like this after x!main_1


    return 0;

}

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

    //Start of the scopeString
    scopeString = "main";
    nameCurrentFunction = "main";

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
    string varName;
    if(ctx->affectation()){
        varName = ctx->affectation()->lvalue()->VAR()->getText();
    }
    else{
        varName = ctx->VAR()->getText();
    }
    FlagVar flagVar;

    flagVar.index = index;
    flagVar.affected = false;
    flagVar.used = false;
    flagVar.functionName = nameCurrentFunction;
    flagVar.varName = varName;
    index -= 4;
    string var = varName + "!"+scopeString; // create the right format for the variable to store it in the table
                                            // like this each variable+scope is unique

    if(symbolTableVar.find(var) == symbolTableVar.end()){
        symbolTableVar.insert({var, flagVar});
    }
    else{        
        cerr<< "Variable " << varName << " dans " << nameCurrentFunction << " deja déclaré" << endl;
        exit(1);
    }

    if (ctx->affectation())
    {
        this->visit(ctx->affectation());
    }
    
    
    return 0;
}


//Visit the affectation
antlrcpp::Any SymbolTableVisitor::visitAffectation(ifccParser::AffectationContext *ctx){
    string varName = ctx->lvalue()->VAR()->getText();
    string var = varName + "!"+scopeString;
    bool find = false;

    size_t pos_bang = var.find_last_of('!');
    size_t pos_ = var.find_last_of('_');
    
    while (pos_ > pos_bang && pos_ != string::npos)
    {
        if(symbolTableVar.find(var) !=symbolTableVar.end()){
            find = true;
            symbolTableVar[var].affected = true;
            break;
        }

        // we are doing shadowing here, if we do not find the var at the current depth 
        // we go up to find if the variable is defined higher
        pos_ = var.find_last_of('_');
    
        if (pos_ != string::npos) {
            var.erase(pos_);
        }
        
        
    }
    // if it is not define higher until the main block we throw an error
    if(!find){
        cerr << "Variable " << varName  << " dans " << nameCurrentFunction <<  " non déclarée" << endl;
        exit(1);
    }
    this->visit(ctx->lvalue());
    this->visit(ctx->expr());
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitExprVar(ifccParser::ExprVarContext *ctx){

    string varName = ctx->VAR()->getText();
    bool find = false;  
    string var = varName + "!"+scopeString;

    size_t pos_bang = var.find_last_of('!');
    size_t pos_ = var.find_last_of('_');
    while (pos_ > pos_bang && pos_ != string::npos)
    {
        if(symbolTableVar.find(var) !=symbolTableVar.end()){
            find = true;
            symbolTableVar[var].used = true;

            break;
        }
        pos_ = var.find_last_of('_');
    
        if (pos_ != string::npos) {
            var.erase(pos_);
        }
        
        
    }

    if(!find){
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " non déclarée" << endl;
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
    scopeString = label;

    nameCurrentFunction = label;

    // Visting the params and block associated with the fonc and then creating its own CFG
    this->visit(ctx->decl_params());
    this->visit(ctx->block());
    CFG* cfg = new CFG(symbolTableVar, label, ctx->block(), nbParams);
    cfg_liste.push_back(cfg);

    return 0;    
}


// Visit the declaration of the params of a function
antlrcpp::Any SymbolTableVisitor::visitDecl_param(ifccParser::Decl_paramContext *ctx) {
    string varName = ctx->VAR()->getText();

    FlagVar flagVar;

    flagVar.index = index;
    flagVar.affected = false;
    flagVar.used = false;
    flagVar.functionName = nameCurrentFunction;
    flagVar.varName = varName;
    index -= 4;
    //we add _1 because params are not inside the block but have the scope so we add it manually
    string var = varName + "!"+scopeString+"_1"; 

    if(symbolTableVar.find(var) == symbolTableVar.end()){

        symbolTableVar.insert({var, flagVar});
    }
    else{        
        cerr<< "Variable " << varName << " dans " << nameCurrentFunction <<  " deja déclaré" << endl;
        exit(1);
    }



    return 0;
}


