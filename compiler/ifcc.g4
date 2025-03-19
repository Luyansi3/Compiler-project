grammar ifcc;

axiom : prog EOF ;

prog : 'int' 'main' '(' ')' block ;
block: '{' instruction '}' ;

instruction: return_stmt ';' instruction 
            | declaration ';' instruction 
            | affectation ';' instruction 
            | expr ';' instruction
            | ;

declaration: TYPE decl_element ;

decl_element: VAR liste_decl
            | affectation liste_decl ;

liste_decl: ',' decl_element 
            |  ;



affectation: lvalue '=' rvalue ;

return_stmt: RETURN rvalue ;

rvalue: affectation | expr ;

lvalue: VAR ;

expr: expr opM expr     #MulDiv
    | expr opA expr     #AddSous
    | opU '(' expr ')'  #Par
    | opU VAR
    | opU CONST ;

opU: '-' | ;

opA: '+' | '-' ;
opM: '/' | '*';

TYPE: 'int' ;
RETURN : 'return' ;
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
CONST : [0-9]+ ;
COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);