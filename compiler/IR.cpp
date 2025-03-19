#include "IR.h"


void IRInstrLDConst::gen_asm(ostream &o){
    bb->cfg->IR_reg_to_asm(dest);
    string d = bb->cfg->IR_reg_to_asm(dest);
    o << "    movl $" << val << ", " << d << endl;
}

void IRInstrCopy::gen_asm(ostream &o){

    string d = bb->cfg->IR_reg_to_asm(dest);
    string s = bb->cfg->IR_reg_to_asm(src);
    o << "    movl " << s << ", " << d << endl;
}


void IRInstrNeg::gen_asm(ostream &o){

    string d = bb->cfg->IR_reg_to_asm(dest);
    o << "    negl " << d << endl;
}

void IRInstrPrologue::gen_asm(ostream &o){
    o<< "    pushq %rbp \n";
    o<< "    movq %rsp, %rbp\n";
}

void IRInstrEpilogue::gen_asm(ostream &o){
    o<< "    popq %rbp\n";
    o << "    ret\n";
}

void IRInstrMul::gen_asm(ostream &o){
    string var1 = bb->cfg->IR_reg_to_asm(src1);
    string var2 = bb->cfg->IR_reg_to_asm(src2);

    o << "    imull " << var1 << ", " << var2 << "\n";
}

void IRInstrDiv::gen_asm(ostream &o){
    string var1 = bb->cfg->IR_reg_to_asm(src1);

    o << "    cltd\n";
    o << "    idivl " << var1 << "\n";
}

void IRInstrAdd::gen_asm(ostream&o)
{   
    string source1=this->bb->cfg->IR_reg_to_asm(src1);
    string source2=this->bb->cfg->IR_reg_to_asm(src2);
    o<< "   addl" << " " << source1<< ", " << source2 << "\n"; //add source, destination  # destination = destination + source

}

void IRInstrSub::gen_asm(ostream&o)
{   
    string source1=this->bb->cfg->IR_reg_to_asm(src1);
    string source2=this->bb->cfg->IR_reg_to_asm(src2);

    o<< "   subl" << " " << source1 << ", " << source2 << "\n";
}
void BasicBlock::gen_asm(ostream& o){
    o << " " << label << ":" << endl; 
    for(auto &instr : instrs){
        instr->gen_asm(o);
    }

}
void BasicBlock::add_IRInstr(IRInstr* instr){
    instrs.push_back(instr);
}



void CFG::add_bb(BasicBlock* bb){
    nextBBnumber++;
    bbs.push_back(bb);
}

string CFG::IR_reg_to_asm(string reg){
    
    if(reg == "!reg"){
        return "%eax";
    }
    else{
        return to_string(symbolIndex[reg].index) + "(%rbp)";
    }
}

void CFG::gen_asm(ostream &o){

    // gen_asm_prologue(o);
    for(auto &bb: bbs){
        bb->gen_asm(o);
    }
    // gen_asm_epilogue(o);

}

string CFG::new_BB_name(){
    return "BB"+ to_string(nextBBnumber);
}

void CFG::gen_asm_prologue(ostream &o){
    o<< ".globl main\n" ;
    o<< " main: \n" ;
    o<< "    pushq %rbp \n";
    o<< "    movq %rsp, %rbp\n";
}
void CFG::gen_asm_epilogue(ostream &o){
    o<< "    popq %rbp\n";
    o << "    ret\n";
}

string CFG::create_new_tempvar()
{
    nextFreeSymbolIndex-=4;
    string tmpVar="!tmp"+to_string(abs(nextFreeSymbolIndex));
    Flag flag={nextFreeSymbolIndex,false,false};
    symbolIndex.insert({tmpVar,flag});
    return tmpVar;
}