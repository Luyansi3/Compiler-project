#include "SymbolTableVisitor.h"

// Declaration of the static attributes
unordered_map<string, FlagFonction> SymbolTableVisitor::symbolTableFonction;
vector<CFG *> SymbolTableVisitor::cfg_liste;

antlrcpp::Any SymbolTableVisitor::visitBlock(ifccParser::BlockContext *ctx)
{

    // if this the first time we enter in the block higher
    if (scope.find(scopeString) == scope.end())
    {
        scope.insert({scopeString, 0});
    }
    // increase the counter of block inside the block higher
    scope[scopeString]++;
    scopeString += "_" + to_string(scope[scopeString]);

    this->visit(ctx->instructions());

    size_t pos = scopeString.find_last_of('_');
    // once we are done with the block we delete his counter
    scope.erase(scopeString);
    // then we delete the last part of the string, looks like this before : x!main_1_1
    if (pos != string::npos)
    {
        scopeString.erase(pos);
    }
    // like this after x!main_1

    return 0;
}

// Visit the program
antlrcpp::Any SymbolTableVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    // Inserting putchar and getchar into the fonction symbol table
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

    // Start of the scopeString
    scopeString = "main";
    nameCurrentFunction = "main";
    unordered_map<string, FlagVar> newSymbolTable;
    index = -4;
    symbolTableVar = newSymbolTable;
    this->visit(ctx->block());
    CFG *cfg = new CFG(symbolTableVar, "main", ctx->block(), 0);
    cfg_liste.push_back(cfg);


    //Visit all the fonctions declarations
    for (auto decl_fonction : ctx->post_decl_fonction())
        this->visit(decl_fonction);

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitAffectation(ifccParser::AffectationContext *ctx)
{
    string varName = ctx->lvalue()->VAR()->getText();
    string var = varName + "!" + scopeString;
    bool find = false;

    size_t pos_bang = var.find_last_of('!');
    size_t pos_ = var.find_last_of('_');

    while (pos_ > pos_bang && pos_ != string::npos)
    {
        if (symbolTableVar.find(var) != symbolTableVar.end())
        {
            find = true;
            symbolTableVar[var].affected = true;
            break;
        }

        // we are doing shadowing here, if we do not find the var at the current depth
        // we go up to find if the variable is defined higher
        pos_ = var.find_last_of('_');

        if (pos_ != string::npos)
        {
            var.erase(pos_);
        }
    }
    // if it is not define higher until the main block we throw an error
    if (!find)
    {
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " non déclarée" << endl;
        exit(1);
    }
    this->visit(ctx->lvalue());
    this->visit(ctx->expr());
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitTableAffectation(ifccParser::TableAffectationContext *ctx)
{
    string varName = ctx->VAR()->getText();
    int tableSize = 0;
    int exprListSize = (ctx->array_litteral()->expr()).size();
    if (ctx->constante())
    {
        tableSize = stoi(ctx->constante()->getText());
        if (exprListSize > tableSize)
        {
            cerr << "Warning : Trop d'élément dans le tableau " << varName << endl;
        }
        
    }


    FlagVar flagVar;

    flagVar.index = index;
    flagVar.affected = false;
    flagVar.used = false;
    flagVar.functionName = nameCurrentFunction;
    flagVar.varName = varName;
    flagVar.isTable = true;
    this->index -= (tableSize) * 4;
    string var = varName + "!" + scopeString; // create the right format for the variable to store it in the table
                                                // like this each variable+scope is unique

    if (symbolTableVar.find(var) == symbolTableVar.end())
    {
        symbolTableVar.insert({var, flagVar});
    }
    else
    {
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " deja déclaré" << endl;
        exit(1);
    }

    if (ctx->constante())
    {
        for (int i = 0; i < min(tableSize, exprListSize); i++)
        {
            symbolTableVar[var].affected = true;
        }
    }
    else
    {
        for (int i = 0; i < exprListSize; i++)
            symbolTableVar[var].affected = true;
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
    bool find = false;
    string var = varName + "!" + scopeString;

    size_t pos_bang = var.find_last_of('!');
    size_t pos_ = var.find_last_of('_');
    while (pos_ > pos_bang && pos_ != string::npos)
    {
        if (symbolTableVar.find(var) != symbolTableVar.end())
        {
            find = true;
            symbolTableVar[var].used = true;

            break;
        }
        pos_ = var.find_last_of('_');

        if (pos_ != string::npos)
        {
            var.erase(pos_);
        }
    }

    if (!find)
    {
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " non déclarée" << endl;
        exit(1);
    }
    else if (symbolTableVar[var].isTable)
    {
        cerr << "Utilisation du pointeur " << varName << " dans " << nameCurrentFunction << endl;
    }

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitExprTable(ifccParser::ExprTableContext *ctx)
{

    string varName = ctx->VAR()->getText();
    bool find = false;
    string var = varName + "!" + scopeString;

    size_t pos_bang = var.find_last_of('!');
    size_t pos_ = var.find_last_of('_');
    while (pos_ > pos_bang && pos_ != string::npos)
    {
        if (symbolTableVar.find(var) != symbolTableVar.end())
        {
            find = true;
            symbolTableVar[var].used = true;

            break;
        }
        pos_ = var.find_last_of('_');

        if (pos_ != string::npos)
        {
            var.erase(pos_);
        }
    }
    

    if (!find)
    {
        cerr << "Tableau " << varName << " dans " << nameCurrentFunction << " non déclarée" << endl;
        exit(1);
    }
    else if (!symbolTableVar[var].isTable)
    {
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " utilisée en tant que tableau" << endl;
        exit(1);
    }
    

    return 0;
}


// Visit the call of a function
antlrcpp::Any SymbolTableVisitor::visitCall(ifccParser::CallContext *ctx)
{
    string label = ctx->VAR()->getText();
    int nbParams = ctx->liste_param()->expr().size();

    if (symbolTableFonction.find(label) == symbolTableFonction.end())
    {
        FlagFonction flag;
        flag.declared = false;
        flag.used = true;
        flag.nombreParams = nbParams;
        flag.type = "";
        symbolTableFonction.insert({label, flag});
        if (label != "putchar" && label != "getchar")
            cerr << "La fonction " << label << " est appelée avant d'être déclarée" << endl;
    } else if (symbolTableFonction[label].nombreParams != nbParams) {
        cerr << "La fonction " << label << " est appelée avec le mauvais nombre de paramètres" << endl;
        exit(1);
    } else {
        symbolTableFonction[label].used = true;
    }

    for (auto expression : ctx->liste_param()->expr())
    {
        this->visit(expression);
    }

    return 0;
}


// Visit the declaration of a function 
antlrcpp::Any SymbolTableVisitor::visitPre_decl_fonction(ifccParser::Pre_decl_fonctionContext *ctx) {
    int nbParams = ctx->decl_params()->decl_param().size();
    string label = ctx->VAR()->getText();
    string type = ctx->typeFonc()->getText();

    if (symbolTableFonction.find(label) != symbolTableFonction.end())
    {
        if (symbolTableFonction[label].declared)
        {
            cerr << "La fonction " << label << " a déjà été déclarée" << endl;
            exit(1); 
        } else if (symbolTableFonction[label].nombreParams != nbParams) {
            cerr << "La fonction " << label << " est appelée avec le mauvais nombre d'arguments" << endl;
        } else if (symbolTableFonction[label].type != "" && symbolTableFonction[label].type != type) {
            cerr << "La fonction " << label << " est appelée avec le mauvais type de retour" << endl;
        } else if (symbolTableFonction[label].type == ""){
            symbolTableFonction[label].declared = true;
            symbolTableFonction[label].type = type;         
        } else {
            symbolTableFonction[label].declared = true;
        }
    }
    else
    {
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

    scopeString = label;

    nameCurrentFunction = label;

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

    // Reinitialize the var symbol table and so the index
    unordered_map<string, FlagVar> newSymbolTable;
    this->index = -4;
    symbolTableVar = newSymbolTable;
    scopeString = label;

    nameCurrentFunction = label;

    // Visting the params and block associated with the fonc and then creating its own CFG
    this->visit(ctx->decl_params());
    this->visit(ctx->block());
    CFG *cfg = new CFG(symbolTableVar, label, ctx->block(), nbParams);
    cfg_liste.push_back(cfg);

    return 0;
}

// Visit the declaration of the params of a function
antlrcpp::Any SymbolTableVisitor::visitDecl_param(ifccParser::Decl_paramContext *ctx)
{
    string varName = ctx->VAR()->getText();

    FlagVar flagVar;

    flagVar.index = index;
    flagVar.affected = false;
    flagVar.used = false;
    flagVar.functionName = nameCurrentFunction;
    flagVar.isTable = false;
    flagVar.varName = varName;
    index -= 4;
    // we add _1 because params are not inside the block but have the scope so we add it manually
    string var = varName + "!" + scopeString + "_1";

    if (symbolTableVar.find(var) == symbolTableVar.end())
    {

        symbolTableVar.insert({var, flagVar});
    }
    else
    {
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " deja déclaré" << endl;
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



antlrcpp::Any SymbolTableVisitor::visitInstrExpr(ifccParser::InstrExprContext *ctx) {
    if (auto parCtx = dynamic_cast<ifccParser::ExprParContext *>(ctx->expr())) {
        ifccParser::CallContext* callCtx = isASimpleCall(parCtx);
        this->visit(callCtx);
    } else {
        this->visit(ctx->expr());
    }     
    

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitClassicDeclaration(ifccParser::ClassicDeclarationContext *ctx) {
    string varName = ctx->VAR()->getText();
    FlagVar flagVar;

    flagVar.index = index;
    flagVar.affected = false;
    flagVar.used = false;
    flagVar.functionName = nameCurrentFunction;
    flagVar.isTable = false;
    flagVar.varName = varName;
    int val=0;
    if (ctx->constante())
    {
        val=stoi(ctx->constante()->getText());
        flagVar.isTable = true;
        this->index-=(val-1)*4;
    }
    else{
        index -= 4;
    }
    
    string var = varName + "!" + scopeString; // create the right format for the variable to store it in the table
                                                // like this each variable+scope is unique

    if (symbolTableVar.find(var) == symbolTableVar.end())
    {
        symbolTableVar.insert({var, flagVar});
    }
    else
    {
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " deja déclaré" << endl;
        exit(1);
    }

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitVarAffectation(ifccParser::VarAffectationContext *ctx) {
    string varName = ctx->VAR()->getText();
    FlagVar flagVar;

    flagVar.index = index;
    flagVar.affected = false;
    flagVar.used = false;
    flagVar.functionName = nameCurrentFunction;
    flagVar.varName = varName;
    flagVar.isTable = false;
    this->index -= 4;
    string var = varName + "!" + scopeString; // create the right format for the variable to store it in the table
                                                // like this each variable+scope is unique

    if (symbolTableVar.find(var) == symbolTableVar.end())
    {
        symbolTableVar.insert({var, flagVar});
    }
    else
    {
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " deja déclaré" << endl;
        exit(1);
    }

    bool find = false;

    size_t pos_bang = var.find_last_of('!');
    size_t pos_ = var.find_last_of('_');

    while (pos_ > pos_bang && pos_ != string::npos)
    {
        if (symbolTableVar.find(var) != symbolTableVar.end())
        {
            find = true;
            symbolTableVar[var].affected = true;
            break;
        }

        // we are doing shadowing here, if we do not find the var at the current depth
        // we go up to find if the variable is defined higher
        pos_ = var.find_last_of('_');

        if (pos_ != string::npos)
        {
            var.erase(pos_);
        }
    }
    // if it is not define higher until the main block we throw an error
    if (!find)
    {
        cerr << "Variable " << varName << " dans " << nameCurrentFunction << " non déclarée" << endl;
        exit(1);
    }
    this->visit(ctx->VAR());
    this->visit(ctx->expr());

    return 0;
}
