#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <vector>
#include <unordered_map>
#include <initializer_list>

#include "antlr4-runtime.h"
#include "generated/ifccParser.h"


#include "Flag.h"

class SymbolTableVisitor;

using namespace std;



// Forward declarations
class BasicBlock;
class CFG;

//! The class for one 3-address instruction
class IRInstr
{
public:
    /** The instructions themselves -- feel free to subclass instead */
    typedef enum
    {
        ldconst,
        copy,
        add,
        sub,
        mul,
        rmem,
        wmem,
        call,
        cmp_eq,
        cmp_lt,
        cmp_le
    } Operation;

    /** Constructor */
    IRInstr(BasicBlock *bb_) : bb(bb_) {}

    /** Actual code generation */
    virtual void gen_asm(ostream &o) = 0; /**< x86 assembly code generation for this IR instruction */
    virtual ~IRInstr() {}                 /**< Destructor */

protected:
    BasicBlock *bb; /**< The BB this instruction belongs to, which provides a pointer to the CFG this instruction belongs to */
};

// Class for loading a constant
class IRInstrLDConst : public IRInstr
{
private:
    string dest;
    int val;

public:
    IRInstrLDConst(BasicBlock *bb, string dest, int val) : IRInstr(bb), dest(dest), val(val) {}
    virtual void gen_asm(ostream &o) override;
};

// Class for copying a value
class IRInstrCopy : public IRInstr
{
private:
    string dest;
    string src;

public:
    IRInstrCopy(BasicBlock *bb, string dest, string src) : IRInstr(bb), dest(dest), src(src) {}
    virtual void gen_asm(ostream &o) override;
};

// Class for negating a value
class IRInstrNeg : public IRInstr
{
private:
    string dest;

public:
    IRInstrNeg(BasicBlock *bb, string dest) : IRInstr(bb), dest(dest) {}
    virtual void gen_asm(ostream &o) override;
};

// Class for the function prologue
class IRInstrPrologue : public IRInstr
{
public:
    IRInstrPrologue(BasicBlock *bb) : IRInstr(bb) {}
    virtual void gen_asm(ostream &o) override;
};

// Class for the function epilogue
class IRInstrEpilogue : public IRInstr
{
public:
    IRInstrEpilogue(BasicBlock *bb) : IRInstr(bb){}
    virtual void gen_asm(ostream &o) override;

};

// Class for addition
class IRInstrAdd : public IRInstr
{
private:
    string src1;
    string src2;

public:
    IRInstrAdd(BasicBlock *bb, string src1, string src2) : IRInstr(bb), src1(src1), src2(src2) {}
    virtual void gen_asm(ostream &o) override;
};

// Class for multiplication
class IRInstrMul : public IRInstr
{
private:
    string src1;
    string src2;

public:
    IRInstrMul(BasicBlock *bb, string src1, string src2) : IRInstr(bb), src1(src1), src2(src2) {}
    virtual void gen_asm(ostream &o) override;
};

// Class for division
class IRInstrDiv : public IRInstr
{
private:
    string src1;

public:
    IRInstrDiv(BasicBlock *bb, string src1) : IRInstr(bb), src1(src1) {}
    virtual void gen_asm(ostream &o) override;
};

// Class for subtraction
class IRInstrSub : public IRInstr
{
private:
    string src1;
    string src2;

public:
    IRInstrSub(BasicBlock *bb, string src1, string src2) : IRInstr(bb), src1(src1), src2(src2) {}
    virtual void gen_asm(ostream &o) override;
};

class IrInstrCall : public IRInstr
{
private:
    string label;
    vector<string> params;

public:
    IrInstrCall(BasicBlock *bb, string label, vector<string> params) : IRInstr(bb), label(label), params(params) {}
    virtual void gen_asm(ostream &o) override;
};


class IRInstrJump : public IRInstr
{
private:
    string label;

public:
    IRInstrJump(BasicBlock *bb, string label) : IRInstr(bb), label(label) {}
    virtual void gen_asm(ostream &o) override;
};

class IRInstrExit : public IRInstr
{
private:
    string src;

public:
    IRInstrExit(BasicBlock *bb, string src) : IRInstr(bb), src(src)
    {
    }
    virtual void gen_asm(ostream &o) override;
};

class IRInstrJump : public IRInstr
{
private:
    string label;

public:
    IRInstrJump(BasicBlock *bb, string label) : IRInstr(bb), label(label) {}
    virtual void gen_asm(ostream &o) override;
};
/**  The class for a basic block */

/* A few important comments.
     IRInstr has no jump instructions.
     cmp_* instructions behaves as an arithmetic two-operand instruction (add or mult),
      returning a boolean value (as an int)

     Assembly jumps are generated as follows:
     BasicBlock::gen_asm() first calls IRInstr::gen_asm() on all its instructions, and then
            if  exit_true  is a  nullptr,
            the epilogue is generated
        else if exit_false is a nullptr,
          an unconditional jmp to the exit_true branch is generated
                else (we have two successors, hence a branch)
          an instruction comparing the value of test_var_name to true is generated,
                    followed by a conditional branch to the exit_false branch,
                    followed by an unconditional branch to the exit_true branch
     The attribute test_var_name itself is defined when converting
  the if, while, etc of the AST  to IR.

Possible optimization:
     a cmp_* comparison instructions, if it is the last instruction of its block,
       generates an actual assembly comparison
       followed by a conditional jump to the exit_false branch
*/

class IRInstrCmpEQ : public IRInstr
{
private:
    string src1;
    string src2;

public:
    IRInstrCmpEQ(BasicBlock *bb, string src1, string src2) : IRInstr(bb), src1(src1), src2(src2) {}
    virtual void gen_asm(ostream &o) override;
};

class IRInstrCmpNEQ : public IRInstr
{
private:
    string src1;
    string src2;

public:
    IRInstrCmpNEQ(BasicBlock *bb, string src1, string src2) : IRInstr(bb), src1(src1), src2(src2) {}
    virtual void gen_asm(ostream &o) override;
};

class IRInstrCmpINF : public IRInstr
{
private:
    string src1;
    string src2;

public:
    IRInstrCmpINF(BasicBlock *bb, string src1, string src2) : IRInstr(bb), src1(src1), src2(src2) {}
    virtual void gen_asm(ostream &o) override;
};

class IRInstrCmpSUP : public IRInstr
{
private:
    string src1;
    string src2;

public:
    IRInstrCmpSUP(BasicBlock *bb, string src1, string src2) : IRInstr(bb), src1(src1), src2(src2) {}
    virtual void gen_asm(ostream &o) override;
};

class IRInstrNot : public IRInstr
{
private:
    string src;

public:
    IRInstrNot(BasicBlock *bb, string src) : IRInstr(bb), src(src) {}
    virtual void gen_asm(ostream &o) override;
};

/** The class for a basic block */
class BasicBlock
{
public:
    BasicBlock(CFG *cfg, string entry_label) : cfg(cfg), label(entry_label), exit_true(nullptr), exit_false(nullptr) {}
    ~BasicBlock();
    void gen_asm(ostream &o); /**< x86 assembly code generation for this basic block (very simple) */

    void add_IRInstr(IRInstr *inst);
    BasicBlock *exit_true;    /**< pointer to the next basic block, true branch. If nullptr, return from procedure */
    BasicBlock *exit_false;   /**< pointer to the next basic block, false branch. If nullptr, the basic block ends with an unconditional jump */
    string label;             /**< label of the BB, also will be the label in the generated code */
    CFG *cfg;                 /**< the CFG where this block belongs */
    vector<IRInstr *> instrs; /**< the instructions themselves */
    string test_var_name;     /**< when generating IR code for an if(expr) or while(expr) etc, store here the name of the variable that holds the value of expr */
    bool isExit;
};

/** The class for the control flow graph, also includes the symbol table */
class CFG
{
public:
    CFG(unordered_map<string, FlagVar> symbolIndex, string nameFunction, antlr4::tree::ParseTree* tree, int nbParams);
    ~CFG();

    void add_bb(BasicBlock *bb);

    // x86 code generation: could be encapsulated in a processor class in a retargetable compiler
    void gen_asm(ostream &o);
    string IR_reg_to_asm(string reg); /**< helper method: inputs an IR reg or input variable, returns e.g. "-24(%rbp)" for the proper value of 24 */
    void gen_asm_prologue(ostream &o);
    void gen_asm_epilogue(ostream &o);
    // void genrateExitAsm(ostream&o);
    string create_return_var();
    string create_new_tempvar();

    antlr4::tree::ParseTree* getTree() {return tree;}

    string getNameFunction() { return nameFunction; }
    vector<BasicBlock*> getBbs() {return bbs;}
    unordered_map<string, FlagVar> &getSymbolIndex() {return symbolIndex;}
    int getNextFreeSymbolIndex() {return nextFreeSymbolIndex;}
    string getVarName(string name, string scopeString);

    string new_BB_name();
    BasicBlock *current_bb;
    BasicBlock *bb_epi;

    int nbParams;
protected:
    unordered_map<string, FlagVar> symbolIndex; /**< part of the symbol table */
    int nextFreeSymbolIndex;                 /**< to allocate new symbols in the symbol table */
    int nextBBnumber;                        /**< just for naming */
    string nameFunction;
    antlr4::tree::ParseTree* tree;
    vector<BasicBlock *> bbs;                /**< all the basic blocks of this CFG */
    BasicBlock *bb_Exit;
    
};