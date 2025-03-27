#include "IR.h"

// Generate assembly code for loading a constant
void IRInstrLDConst::gen_asm(ostream &o){
    string d = bb->cfg->IR_reg_to_asm(dest);
    o << "    movl $" << val << ", " << d <<"\n";
}

// Generate assembly code for copying a value
void IRInstrCopy::gen_asm(ostream &o){
    string d = bb->cfg->IR_reg_to_asm(dest);
    string s = bb->cfg->IR_reg_to_asm(src);
    o << "    movl " << s << ", " << d <<"\n";
}

// Generate assembly code for negating a value
void IRInstrNeg::gen_asm(ostream &o){
    string d = bb->cfg->IR_reg_to_asm(dest);
    o << "    negl " << d <<"\n";
}

// Generate assembly code for the function prologue
void IRInstrPrologue::gen_asm(ostream &o){
    o<< "    pushq %rbp \n";
    o<< "    movq %rsp, %rbp\n";
}

// Generate assembly code for the function epilogue
void IRInstrEpilogue::gen_asm(ostream &o){
    o<< "    popq %rbp\n";
    o << "    ret\n";
}

// Generate assembly code for multiplication
void IRInstrMul::gen_asm(ostream &o){
    string var1 = bb->cfg->IR_reg_to_asm(src1);
    string var2 = bb->cfg->IR_reg_to_asm(src2);
    o << "    imull " << var1 << ", " << var2 << "\n";
}

// Generate assembly code for division
void IRInstrDiv::gen_asm(ostream &o){
    string var1 = bb->cfg->IR_reg_to_asm(src1);
    o << "    cltd\n";
    o << "    idivl " << var1 << "\n";
}

// Generate assembly code for addition
void IRInstrAdd::gen_asm(ostream &o){
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "   addl " << source1 << ", " << source2 << "\n"; // add source, destination  # destination = destination + source
}

// Generate assembly code for subtraction
void IRInstrSub::gen_asm(ostream &o){
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "   subl " << source1 << ", " << source2 << "\n";
}

void IrInstrCall::gen_asm(ostream &o) {
    for (int i = 0; i<6 && i<params.size(); i++) {
        string param = this->bb->cfg->IR_reg_to_asm(params[i]);
        if (i==0)
            o << "    movl" << " " << param << ", " << "%edi" <<"\n";
        else if (i==1)
            o << "    movl" << " " << param << ", " << "%esi" <<"\n";
        else if (i==2)
            o << "    movl" << " " << param << ", " << "%edx" <<"\n";
        else if (i==3)
            o << "    movl" << " " << param << ", " << "%ecx" <<"\n";
        else if (i==4)
            o << "    movl" << " " << param << ", " << "%r8d" <<"\n";
        else if (i==5)
            o << "    movl" << " " << param << ", " << "%r9d" <<"\n";
    }

    for (int i = params.size() - 1; i>5; i++) {
        string param = this->bb->cfg->IR_reg_to_asm(params[i]);
        o << "    pushq" << " " << param << "\n";
    }
    o << "    call" << " " << label << "\n";
}

// Generate assembly code for comparison
void IRInstrCmpEQ::gen_asm(ostream &o){
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    cmpl " << source1 << ", " << source2 << "\n";
    o << "    sete %al\n";
    o << "    movzbl %al, %eax\n";
}

// Generate assembly code for non equal comparison
void IRInstrCmpNEQ::gen_asm(ostream &o){
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    cmpl " << source1 << ", " << source2 << "\n";
    o << "    setne %al\n";
    o << "    movzbl %al, %eax\n";
}

// Generate assembly code for less than comparison
void IRInstrCmpINF::gen_asm(ostream &o){
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    cmpl " << source1 << ", " << source2 << "\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
}

// Generate assembly code for greater than comparison
void IRInstrCmpSUP::gen_asm(ostream &o){
    string source1 = this->bb->cfg->IR_reg_to_asm(src1);
    string source2 = this->bb->cfg->IR_reg_to_asm(src2);
    o << "    cmpl " << source1 << ", " << source2 << "\n";
    o << "    setg %al\n";
    o << "    movzbl %al, %eax\n";
}


// Generate assembly code for a basic block and its instructions
void BasicBlock::gen_asm(ostream& o){
    o << " " << label << ":" <<"\n"; 
    for(auto &instr : instrs){
        instr->gen_asm(o);
    }

    if (exit_true != nullptr && exit_false != nullptr) {
        string eval = this->cfg->IR_reg_to_asm(test_var_name);
        o << "    cmpl $0, " << eval << "\n";
        o << "    je " << exit_false->label << "\n";
        o << "    jmp " << exit_true->label << "\n";
    }
    else if (exit_true != nullptr) {
        o << "    jmp " << exit_true->label << "\n";
        
    }
}

// Add an IR instruction to a basic block
void BasicBlock::add_IRInstr(IRInstr* instr){
    instrs.push_back(instr);
}

// Destructor for BasicBlock
BasicBlock::~BasicBlock(){
    for(auto &instr : instrs){
        delete instr;
    }
}

// Add a basic block to the CFG
void CFG::add_bb(BasicBlock* bb){
    bbs.push_back(bb);
}

// Convert an IR register to assembly
string CFG::IR_reg_to_asm(string reg){
    if(reg == "!reg"){
        return "%eax";
    }
    else{
        return to_string(symbolIndex[reg].index) + "(%rbp)";
    }
}

// Generate assembly code for the CFG and its basic blocks
void CFG::gen_asm(ostream &o){
    // gen_asm_prologue(o);
    for(auto &bb: bbs){
        bb->gen_asm(o);
    }
    // gen_asm_epilogue(o);
}

// Generate a new basic block name
string CFG::new_BB_name(){
    nextBBnumber++;
    return "BB"+ to_string(nextBBnumber);
}

// Generate assembly code for the function prologue
void CFG::gen_asm_prologue(ostream &o){
    o<< ".globl main\n" ;
    o<< " main: \n" ;
    o<< "    pushq %rbp \n";
    o<< "    movq %rsp, %rbp\n";
}

// Generate assembly code for the function epilogue
void CFG::gen_asm_epilogue(ostream &o){
    o<< "    popq %rbp\n";
    o << "    ret\n";
}

// Create a new temporary variable
string CFG::create_new_tempvar(){
    nextFreeSymbolIndex -= 4;
    string tmpVar = "!tmp" + to_string(abs(nextFreeSymbolIndex));
    FlagVar flag = {nextFreeSymbolIndex, false, false};
    symbolIndex.insert({tmpVar, flag});
    return tmpVar;
}

// Constructor for CFG
CFG::CFG(unordered_map<string, FlagVar> symbolIndex, string nameFunction, antlr4::tree::ParseTree* tree)
    : symbolIndex(symbolIndex), nextBBnumber(0), nameFunction(nameFunction), nextFreeSymbolIndex(symbolIndex.size() * -4), tree(tree)
{
    BasicBlock *bb_prologue = new BasicBlock(this, nameFunction);
    bb_prologue->add_IRInstr(new IRInstrPrologue(bb_prologue));
    add_bb(bb_prologue);

    BasicBlock *bb_body = new BasicBlock(this, new_BB_name());
    add_bb(bb_body);
    current_bb = bb_body;
    bb_prologue->exit_true = bb_body;

    // Create a new basic block for the epilogue
    BasicBlock *bb_epilogue = new BasicBlock(this, getNameFunction() + "_epilogue");
    bb_epilogue->add_IRInstr(new IRInstrEpilogue(bb_epilogue));
    add_bb(bb_epilogue);
    bb_body->exit_true = bb_epilogue;
    this->bb_epi = bb_epilogue;
}

// Destructor for CFG
CFG::~CFG(){
    for(auto &bb : bbs){
        delete bb;
    }
}