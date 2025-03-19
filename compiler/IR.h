#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <vector>
#include <unordered_map>
#include <initializer_list>

using namespace std;

#include "SymbolTableVisitor.h"
// Declarations from the parser -- replace with your own
// #include "type.h"
// #include "symbole.h"

class BasicBlock;
class CFG;
// class DefFonction;

// typedef struct Flag
// {
//     int index;
//     bool used;
//     bool affected;
// }Flag;

//! The class for one 3-address instruction
class IRInstr {
 
   public:
	/** The instructions themselves -- feel free to subclass instead */
	typedef enum {
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


	/**  constructor */
	IRInstr(BasicBlock* bb_): bb(bb_) {}
	
	/** Actual code generation */
	virtual void gen_asm(ostream &o) = 0; /**< x86 assembly code generation for this IR instruction */
	
 protected:
	BasicBlock* bb; /**< The BB this instruction belongs to, which provides a pointer to the CFG this instruction belong to */
	Operation op;
	//Type t;
	vector<string> params; /**< For 3-op instrs: d, x, y; for ldconst: d, c;  For call: label, d, params;  for wmem and rmem: choose yourself */
	// if you subclass IRInstr, each IRInstr subclass has its parameters and the previous (very important) comment becomes useless: it would be a better design. 
};


class IRInstrLDConst : public IRInstr
{
private:
	string dest;
	int val;
public:
	IRInstrLDConst(BasicBlock* bb, string dest, int val): IRInstr(bb), dest(dest), val(val){};
	virtual void gen_asm(ostream &o) override;

};

class IRInstrCopy : public IRInstr
{
private:
	string dest;
	string src;
public:
	IRInstrCopy(BasicBlock* bb, string dest, string src): IRInstr(bb), dest(dest), src(src){};
	virtual void gen_asm(ostream &o) override;
};

class IRInstrNeg : public IRInstr
{
private:
	string dest;
public:
	IRInstrNeg(BasicBlock* bb, string dest): IRInstr(bb), dest(dest) {};
	virtual void gen_asm(ostream &o) override;
};

class IRInstrPrologue : public IRInstr
{
public:
	IRInstrPrologue(BasicBlock* bb): IRInstr(bb) {}
	virtual void gen_asm(ostream &o) override;
};

class IRInstrEpilogue : public IRInstr
{
public:
	IRInstrEpilogue(BasicBlock* bb): IRInstr(bb){}
	virtual void gen_asm(ostream &o) override;
};

class IRInstrMul : public IRInstr
{
public:
	IRInstrMul(BasicBlock* bb, string src1, string src2): IRInstr(bb), src1(src1), src2(src2){}
	virtual void gen_asm(ostream &o) override;
private:
	string src1;
	string src2;
};

class IRInstrDiv : public IRInstr
{
public:
	IRInstrDiv(BasicBlock* bb, string src1): IRInstr(bb), src1(src1){}
	virtual void gen_asm(ostream &o) override;
private:
	string src1;
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

class BasicBlock {
 public:
	BasicBlock(CFG* cfg, string entry_label): cfg(cfg), label(entry_label) {}
	void gen_asm(ostream &o); /**< x86 assembly code generation for this basic block (very simple) */

	void add_IRInstr(IRInstr* inst);

	// No encapsulation whatsoever here. Feel free to do better.
	BasicBlock* exit_true;  /**< pointer to the next basic block, true branch. If nullptr, return from procedure */ 
	BasicBlock* exit_false; /**< pointer to the next basic block, false branch. If null_ptr, the basic block ends with an unconditional jump */
	string label; /**< label of the BB, also will be the label in the generated code */
	CFG* cfg; /** < the CFG where this block belongs */
	vector<IRInstr*> instrs; /** < the instructions themselves. */
  string test_var_name;  /** < when generating IR code for an if(expr) or while(expr) etc,
													 store here the name of the variable that holds the value of expr */
 protected:

 
};




/** The class for the control flow graph, also includes the symbol table */

/* A few important comments:
	 The entry block is the one with the same label as the AST function name.
	   (it could be the first of bbs, or it could be defined by an attribute value)
	 The exit block is the one with both exit pointers equal to nullptr.
     (again it could be identified in a more explicit way)

 */

class CFG {
 public:
	CFG(/*DefFonction* ast*/unordered_map<string, Flag>& symbolIndex, string nameFunction):symbolIndex(symbolIndex), 
	nextBBnumber(0), nameFunction(nameFunction), nextFreeSymbolIndex(symbolIndex.size()*-4)
	{
		BasicBlock* bb_prologue = new BasicBlock(this, nameFunction);
		bb_prologue->add_IRInstr(new IRInstrPrologue(bb_prologue));
		add_bb(bb_prologue);
		
		BasicBlock* bb_body = new BasicBlock(this, new_BB_name());
		nextBBnumber++;
		add_bb(bb_body);
		current_bb = bb_body;
		bb_prologue->exit_true=bb_body;


		BasicBlock* bb_epilogue = new BasicBlock(this, nameFunction + "_epilogue");
		bb_epilogue->add_IRInstr(new IRInstrEpilogue(bb_epilogue));
		add_bb(bb_epilogue);
		bb_body->exit_true = bb_epilogue;
	}

	// DefFonction* ast; /**< The AST this CFG comes from */
	
	void add_bb(BasicBlock* bb);

	// x86 code generation: could be encapsulated in a processor class in a retargetable compiler
	void gen_asm(ostream& o);
	string IR_reg_to_asm(string reg); /**< helper method: inputs a IR reg or input variable, returns e.g. "-24(%rbp)" for the proper value of 24 */
	void gen_asm_prologue(ostream& o);
	void gen_asm_epilogue(ostream& o);

	// symbol table methods
	// void add_to_symbol_table(string name, Type t);
	string create_new_tempvar(/* Type t */);
	int get_var_index(string name);
	// Type get_var_type(string name);

	// basic block management
	string new_BB_name();
	BasicBlock* current_bb;

 protected:
 	unordered_map<string, Flag> symbolIndex; /**< part of the symbol table  */
	int nextFreeSymbolIndex; /**< to allocate new symbols in the symbol table */
	int nextBBnumber; /**< just for naming */
	string nameFunction;
	vector <BasicBlock*> bbs; /**< all the basic blocks of this CFG*/
};

