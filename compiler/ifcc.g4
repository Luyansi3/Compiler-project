grammar ifcc;

axiom : prog EOF ;

prog : TYPE INT OPENPAR CLOSEPAR block ;
block: OPENCROCHET instruction CLOSECROCHET ;

instruction: return_stmt  SEMI  instruction 
            | declaration  SEMI  instruction 
            | expr  SEMI instruction
            | ;

declaration: TYPE decl_element ;

decl_element: VAR liste_decl
            | affectation liste_decl ;

liste_decl: COMMA decl_element 
            |  ;



affectation: lvalue EQUAL expr ;

return_stmt: RETURN expr ;

lvalue: VAR ;

expr: expr opM expr     #MulDiv
    | expr opA expr     #AddSous
    | opU OPENPAR expr CLOSEPAR  #Par
    | opU VAR           #ExprVar
    | opU CONST         #ExprConst 
    | affectation       #ExprAffectation ;

opU: MINUS | ;

opA: PLUS | MINUS ;
opM: DIV | MULT ;

INT: 'int' ;
RETURN : 'return' ;
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
CONST : [0-9]+ ;
COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);


OPENPAR       : '(';
CLOSEPAR      : ')';
OPENCROCHET   : '{';
CLOSECROCHET  : '}';
TYPE          : 'int';
SEMI          : ';';
EQUAL         : '=';
PLUS          : '+';
MINUS         : '-';
MULT          : '*';
DIV           : '/';
COMMA         : ',';