#include "IR.h"
#include "SymbolTableVisitor.h" // Include the header for SymbolTableVisitor

// Generate assembly code for loading a constant
void IRInstrLDConst::gen_asm(ostream &o)
{
    string d = bb->cfg->IR_reg_to_asm(dest);
    o << "    movl $" << val << ", " << d << "\n";
}

// Generate assembly code for copying a value
void IRInstrCopy::gen_asm(ostream &o)
{
    string d = bb->cfg->IR_reg_to_asm(dest);
    string s = bb->cfg->IR_reg_to_asm(src);
    o << "    movl " << s << ", " << d << "\n";
}

// Generate assembly code for negating a value
void IRInstrNeg::gen_asm(ostream &o)
{
    string d = bb->cfg->IR_reg_to_asm(dest);
    o << "    negl " << d << "\n";
}

// Generate assembly code for the function prologue
void IRInstrPrologue::gen_asm(ostream &o)
{
    // Prologue classqieu
    o << "    pushq %rbp \n";
    o << "    movq %rsp, %rbp\n";

    // Allouer la mémoire
    int offset = -bb->cfg->getNextFreeSymbolIndex();
    offset += (16 - ((offset) % 16));
    o << "    subq $" << offset << ", " << "%rsp\n";

    // Recuperation des 6 premiers params
    for (int i = 1; i <= bb->cfg->nbParams && i < 7; i++)
    {
        if (i == 1)
            o << "    movl" << " " << "%edi" << ", " << -4 * i << "(%rbp)" << "\n";
        else if (i == 2)
            o << "    movl" << " " << "%esi" << ", " << -4 * i << "(%rbp)" << "\n";
        else if (i == 3)
            o << "    movl" << " " << "%edx" << ", " << -4 * i << "(%rbp)" << "\n";
        else if (i == 4)
            o << "    movl" << " " << "%ecx" << ", " << -4 * i << "(%rbp)" << "\n";
        else if (i == 5)
            o << "    movl" << " " << "%r8d" << ", " << -4 * i << "(%rbp)" << "\n";
        else if (i == 6)
            o << "    movl" << " " << "%r9d" << ", " << -4 * i << "(%rbp)" << "\n";
    }

    // Recuperation sur la pile des autres params
    for (int i = 7; i <= bb->cfg->nbParams; i++)
    {
        o << "    movl" << " " << 16 + 8 * (i - 7) << "(%rbp)" << ", " << "%eax" << "\n"; // A regler le probleme des offsets.
        o << "    movl" << " " << "%eax" << ", " << -4 * i << "(%rbp)" << "\n";
    }
}

// Generate assembly code for the function epilogue
void IRInstrEpilogue::gen_asm(ostream &o)
{
    string source = this->bb->cfg->IR_reg_to_asm("!returnVal");

    o << "    movl " << source << ", %eax" << "\n";
    // Remettre rsp à son état initial
    int offset = -bb->cfg->getNextFreeSymbolIndex();
    offset += (16 - ((offset) % 16));
    o << "    addq $" << offset << ", " << "%rsp\n";

    o << "    popq %rbp\n";
    o << "    ret\n";
}

// Generate assembly code for multiplication
void IRInstrMul::gen_asm(ostream &o)
{
    string var1 = bb->cfg->IR_reg_to_asm(src1);
    string var2 = bb->cfg->IR_reg_to_asm(src2);
    o << "    imull " << var1 << ", " << var2 << "\n";
}

// Generate assembly code for division
void IRInstrDiv::gen_asm(ostream &o)
{
    string var1 = bb->cfg->IR_reg_to_asm(src1);
    o << "    cltd\n";
    o << "    idivl " << var1 << "\n";
}

// Generate assembly code for addition
void IRInstrAdd::gen_asm(ostream &o)
{
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    addl " << source1 << ", " << source2 << "\n"; // add source, destination  # destination = destination + source
}

// Generate assembly code for subtraction
void IRInstrSub::gen_asm(ostream &o)
{
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    subl " << source1 << ", " << source2 << "\n";
}

void IrInstrCall::gen_asm(ostream &o)
{
    bool non_aligned = params.size() > 6 && params.size() % 2;
    int pushed = 0;

    for (int i = 0; i < 6 && i < params.size(); i++)
    {
        string param = this->bb->cfg->IR_reg_to_asm(params[i]);
        if (i == 0)
            o << "    movl" << " " << param << ", " << "%edi" << "\n";
        else if (i == 1)
            o << "    movl" << " " << param << ", " << "%esi" << "\n";
        else if (i == 2)
            o << "    movl" << " " << param << ", " << "%edx" << "\n";
        else if (i == 3)
            o << "    movl" << " " << param << ", " << "%ecx" << "\n";
        else if (i == 4)
            o << "    movl" << " " << param << ", " << "%r8d" << "\n";
        else if (i == 5)
            o << "    movl" << " " << param << ", " << "%r9d" << "\n";
    }

    for (int i = params.size() - 1; i >= 6; i--)
    {
        pushed++;
        string param = this->bb->cfg->IR_reg_to_asm(params[i]);
        o << "    pushq" << " " << param << "\n";
    }

    if (non_aligned)
        o << "    subq $8, " << "%rsp\n";

    o << "    call" << " " << label << "\n";

    if (non_aligned)
        o << "    addq $" << 8 * (pushed + 1) << ", " << "%rsp\n";
    else if (pushed)
        o << "    addq $" << 8 * pushed << ", " << "%rsp\n";
}

// Generate assembly code for comparison
void IRInstrCmpEQ::gen_asm(ostream &o)
{
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    cmpl " << source1 << ", " << source2 << "\n";
    o << "    sete %al\n";
    o << "    movzbl %al, %eax\n";
}

// Generate assembly code for non equal comparison
void IRInstrCmpNEQ::gen_asm(ostream &o)
{
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    cmpl " << source1 << ", " << source2 << "\n";
    o << "    setne %al\n";
    o << "    movzbl %al, %eax\n";
}

// Generate assembly code for less than comparison
void IRInstrCmpINF::gen_asm(ostream &o)
{
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    cmpl " << source1 << ", " << source2 << "\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
}

// Generate assembly code for greater than comparison
void IRInstrCmpSUP::gen_asm(ostream &o)
{
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    cmpl " << source1 << ", " << source2 << "\n";
    o << "    setg %al\n";
    o << "    movzbl %al, %eax\n";
}

void IRInstrExit::gen_asm(ostream &o)
{
    // o << "Exit_block :" << endl;
    //     o << "    movl" << src1 << ", " << src2 << "\n";
    //     o << "    popq %rbp\n";
    //     o << "    ret\n";
    string source = this->bb->cfg->IR_reg_to_asm(src);

    o << "    movl " << source << ", %eax" << "\n";
    o << "    popq %rbp\n";
    o << "    ret\n";
}

void IRInstrMem::gen_asm(ostream &o)
{
    string srcReg = bb->cfg->IR_reg_to_asm(src);     // value to store, in memory (-16(%rbp))
    string indexReg = bb->cfg->IR_reg_to_asm(index); // memory containing index value (-24(%rbp))
    string baseStr = bb->cfg->IR_reg_to_asm(base);   // base address (-64(%rbp))

    int displacement = 0;
    string baseReg;
    size_t pos = baseStr.find('(');

    if (pos != string::npos && pos > 0)
        displacement = stoi(baseStr.substr(0, pos));
    else
        displacement = 0; // Just in case

    // // Step 1: Load the index from memory into %ecx
    // o << "    movl " << indexReg << ", %rcx" << "\n";
    // Charger l'index (valeur de i) en %ecx (32 bits)
    o << "    movl " << indexReg << ", %ecx\n";

    // Étendre %ecx (32 bits) en %rcx (64 bits)
    o << "    movslq %ecx, %rcx\n";

    o << "    movl " << srcReg << ", %eax" << "\n";
    o << "    neg %rcx\n";
    // Step 3: Store %eax into displacement(%rbp, %ecx, 4)
    o << "    movl %eax, " << displacement << "(%rbp, %rcx, 4)" << "\n";
}

void IRInstrCopyMem::gen_asm(ostream &o)
{
    string destReg = bb->cfg->IR_reg_to_asm(src);    // Not really "dest", it's where to store value (%eax)
    string indexReg = bb->cfg->IR_reg_to_asm(index); // index is memory, like -16(%rbp)
    string baseStr = bb->cfg->IR_reg_to_asm(base);   // base address of array, like -64(%rbp)

    int displacement = 0;
    size_t pos = baseStr.find('(');
    displacement = stoi(baseStr.substr(0, pos));
    // o << "movl " << indexReg << ", %rcx\n";
    // Charger l'index (valeur de i) en %ecx (32 bits)
    o << "    movl " << indexReg << ", %ecx\n";

    // Étendre %ecx (32 bits) en %rcx (64 bits)
    o << "    movslq %ecx, %rcx\n";
    o << "    neg %rcx\n";
    o << "movl " << displacement << "(%rbp," << "%rcx, 4), %eax\n";
}

void IRInstrJump::gen_asm(ostream &o)
{
    o << "    jmp " << label << "\n";
}

void IRInstrNot::gen_asm(ostream &o)
{
    string d = bb->cfg->IR_reg_to_asm(src);
    o << "    cmpl $0, " << d << "\n";
    o << "    sete %al" << "\n";
    o << "    movsbl %al, %eax" << "\n";
}

// Generate assembly code for a basic block and its instructions
void BasicBlock::gen_asm(ostream &o)
{
    o << " " << label << ":" << "\n";
    for (auto &instr : instrs)
    {
        instr->gen_asm(o);
    }

    if (exit_true != nullptr && exit_false != nullptr)
    {
        string eval = this->cfg->IR_reg_to_asm(test_var_name);
        o << "    cmpl $0, " << eval << "\n";
        o << "    je " << exit_false->label << "\n";
        o << "    jmp " << exit_true->label << "\n";
    }
    else if (exit_true != nullptr and exit_true != this->cfg->bb_epi)
    {
        o << "    jmp " << exit_true->label << "\n";
    }
}

// Add an IR instruction to a basic block
void BasicBlock::add_IRInstr(IRInstr *instr)
{
    instrs.push_back(instr);
}

// Destructor for BasicBlock
BasicBlock::~BasicBlock()
{
    for (auto &instr : instrs)
    {
        delete instr;
    }
}

// Add a basic block to the CFG
void CFG::add_bb(BasicBlock *bb)
{
    bbs.push_back(bb);
}

// Convert an IR register to assembly
string CFG::IR_reg_to_asm(string reg)
{
    if (reg == "!reg")
    {
        return "%eax";
    }
    else
    {
        return to_string(symbolIndex[reg].index) + "(%rbp)";
    }
}

// Generate assembly code for the CFG and its basic blocks
void CFG::gen_asm(ostream &o)
{
    // gen_asm_prologue(o);
    for (auto &bb : bbs)
    {
        bb->gen_asm(o);
    }
    // gen_asm_epilogue(o);
}

// Generate a new basic block name
string CFG::new_BB_name()
{
    nextBBnumber++;
    return nameFunction + "_BB" + to_string(nextBBnumber);
}

// Generate assembly code for the function prologue
void CFG::gen_asm_prologue(ostream &o)
{
    o << ".globl main\n";
    o << " main: \n";
    o << "    pushq %rbp \n";
    o << "    movq %rsp, %rbp\n";
}

// Generate assembly code for the function epilogue
void CFG::gen_asm_epilogue(ostream &o)
{
    o << "    popq %rbp\n";
    o << "    ret\n";
}

// Create a new temporary variable
string CFG::create_new_tempvar()
{
    nextFreeSymbolIndex -= 4;
    string tmpVar = "!tmp" + to_string(abs(nextFreeSymbolIndex));
    FlagVar flag = {nextFreeSymbolIndex, false, false, nameFunction, tmpVar};
    symbolIndex.insert({tmpVar, flag});
    return tmpVar;
}

// Constructor for CFG
CFG::CFG(unordered_map<string, FlagVar> symbolIndex, string nameFunction, antlr4::tree::ParseTree *tree, int nbParams)
    : symbolIndex(symbolIndex), nextBBnumber(0), nameFunction(nameFunction), tree(tree), nbParams(nbParams)

{
    // int next = 0;
    // for (auto pair : symbolIndex)
    // {
    //     next = min(next, pair.second.index);
    // }
    // nextFreeSymbolIndex = next;
    BasicBlock *bb_prologue = new BasicBlock(this, nameFunction);
    bb_prologue->add_IRInstr(new IRInstrPrologue(bb_prologue));
    add_bb(bb_prologue);

    BasicBlock *bb_body = new BasicBlock(this, new_BB_name());
    add_bb(bb_body);
    current_bb = bb_body;
    bb_prologue->exit_true = bb_body;

    // Create a new basic block for the epilogue
    BasicBlock *bb_epilogue = new BasicBlock(this, getNameFunction() + "_epilogue");
    create_return_var();
    bb_epilogue->add_IRInstr(new IRInstrEpilogue(bb_epilogue));
    add_bb(bb_epilogue);
    bb_body->exit_true = bb_epilogue;
    this->bb_epi = bb_epilogue;
}

// Destructor for CFG
CFG::~CFG()
{
    for (auto &bb : bbs)
    {
        delete bb;
    }
}

string CFG::create_return_var()
{
    int next = 0;
    for (auto pair : symbolIndex)
    {
        next = min(next, pair.second.index);
    }
    nextFreeSymbolIndex = next-4;
    
    string returnVar = "!returnVal";
    FlagVar flag = {nextFreeSymbolIndex, false, false};
    symbolIndex.insert({returnVar, flag});
    return returnVar;
}
// Get the variable index from the symbol table
string CFG::getVarName(string name, string scopeString)
{
    string var = name + "!" + scopeString;

    size_t pos_bang = var.find_last_of('!');
    size_t pos_ = var.find_last_of('_');
    // We are searching the right variables with the right scopes (shadowing if necessary)
    while (pos_ > pos_bang && pos_ != string::npos)
    {
        if (this->getSymbolIndex().find(var) != this->getSymbolIndex().end())
            break;
        pos_ = var.find_last_of('_');
        if (pos_ != string::npos)
        {
            var.erase(pos_);
        }
    }

    return var;
}
