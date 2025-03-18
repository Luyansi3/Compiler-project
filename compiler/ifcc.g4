grammar ifcc;

axiom : prog EOF ;

prog : 'int' 'main' '(' ')' block ;
block: '{' instruction '}' ;

instruction: return_stmt ';' instruction 
            | declaration ';' instruction 
            | affectation ';' instruction 
            |  ;

declaration: TYPE decl_element ;

decl_element: VAR liste_decl
            | affectation liste_decl ;

liste_decl: ',' decl_element 
            |  ;



affectation: lvalue '=' rvalue ;

return_stmt: RETURN rvalue ;

rvalue: CONST | VAR | affectation ;

lvalue: VAR ;


TYPE: 'int' ;
RETURN : 'return' ;
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
CONST : [0-9]+ ;
COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);