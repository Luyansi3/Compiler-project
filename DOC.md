# **Developer Documentation**
This document provides an overview of the architecture and key components of our simplified C compiler.

## **1. Compilation Process**
The *main.cpp* file orchestrates the compilation process, which can be broken down into the following stages:

### Input Processing:

- Reads the source code from the file specified as a command-line argument.
- Handles an optional second argument `-O0` to disable optimizations.
- Uses an ANTLR input stream to feed the source code to the lexer.

### Lexical Analysis (Lexing):

The *ifccLexer.h* (generated from *ifcc.g4*) tokenizes the input stream into a sequence of tokens.

### Syntactic Analysis (Parsing):

The *ifccParser.h* (generated from *ifcc.g4*) parses the token stream according to the grammar defined in *ifcc.g4*.
This stage produces a Parse Tree, representing the syntactic structure of the source code.
Syntax errors are detected and reported during this phase.

### Semantic Analysis and Intermediate Representation Generation:

The *SymbolTableVisitor.h* defines a visitor (`SymbolTableVisitor`) that traverses the Parse Tree.
Symbol Table Construction: During the traversal, it builds two symbol tables:
- `symbolTableVar`: Stores information about variables (name, scope, type, memory location, usage).
- `symbolTableFonction`: Stores information about functions (name, declaration status, usage).

**Control Flow Graph (CFG) Construction**: For each function in the source code, the visitor creates a Control Flow Graph (`CFG` class defined in *IR.h*). The CFG represents the flow of execution within the function as a set of basic blocks.

The `cfg_liste` in `SymbolTableVisitor` holds a list of all generated CFGs.

Semantic Checks: The visitor also performs semantic checks, such as verifying function declarations and usage, and variable usage.

### Intermediate Representation Linearization:

The *Linearize.h* (or *Linearize_optimized.h* for optimized compilation) defines a visitor (`Linearize` or `Linearize_optimized`) that traverses the Parse Tree for each `CFG`.

This visitor generates a sequence of three-address code instructions (Intermediate Representation - `IRInstr` classes defined in *IR.h*) within the basic blocks of the CFG.

The optimized version likely implements transformations to improve the generated IR.

### Assembly Code Generation:

The *IR.h* file contains the definitions of the `IRInstr` abstract base class and its concrete derived classes, each representing a specific IR instruction (e.g., `IRInstrLDConst`, `IRInstrAdd`, `IRInstrMem`).

Each `IRInstr` subclass implements the `gen_asm(ostream &o)` method, which is responsible for emitting the corresponding x86 assembly code.

The `CFG` class also has a `gen_asm(ostream &o)` method that iterates through its basic blocks and their instructions, calling the gen_asm method of each `IRInstr` to produce the final assembly code, which is printed to standard output.

## **2. Key Components**

***ifcc.g4***: ANTLR grammar file defining the syntax of the simplified C language. ANTLR uses this file to generate the lexer (*ifccLexer.h*) and parser (*ifccParser.h*).

***main.cpp***: The main program file that drives the compilation process. It handles command-line arguments, invokes the lexer and parser, performs semantic analysis, generates intermediate representation, and finally produces assembly code.

***Linearize.h / Linearize_optimized.h***: Header files defining the visitors responsible for traversing the Parse Tree and generating the Intermediate Representation (IR) within the Control Flow Graphs (CFGs). The _optimized version likely includes optimizations during IR generation.

***SymbolTableVisitor.h***: Header file defining the visitor that traverses the Parse Tree to construct the Symbol Table (for variables and functions) and the Control Flow Graphs (CFGs) for each function. It also performs semantic checks.

***IR.h***: Header file defining the classes and data structures for the Intermediate Representation (IR). This includes the `IRInstr` base class and its derived classes representing different types of IR instructions, as well as the `BasicBlock` and `CFG` classes for organizing the IR.

***Flag.h***: This file defines structures (`FlagVar`, `FlagFonction`) to hold metadata associated with variables and functions in the symbol tables (e.g., scope, type, usage).

## **3. Data Flow**
The compilation process follows this general data flow:

1. Source code (.c file) is read by *main.cpp*.

2. The lexer (*ifccLexer.h*) converts the source code into a stream of tokens based on the grammar in *ifcc.g4*.

3. The parser (*ifccParser.h*) consumes the token stream and builds a Parse Tree, also based on *ifcc.g4*.

4. The `SymbolTableVisitor` traverses the Parse Tree, populating the Symbol Table (`symbolTableVar`, `symbolTableFonction`) and creating a list of Control Flow Graphs (cfg_liste), one for each function.

5. For each `CFG` in `cfg_liste`, the `Linearize` (or `Linearize_optimized`) visitor traverses the original Parse Tree again (within the context of that function's scope) and generates a sequence of `IRInstr` objects within the CFG's basic blocks.

6. Finally, the `gen_asm` method of each `CFG` and its constituent `IRInstr` objects is called to output the target assembly code to standard output.