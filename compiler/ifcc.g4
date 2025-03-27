grammar ifcc;

axiom : prog EOF ;

prog : decl_fonction* INT MAIN OPENPAR CLOSEPAR block decl_fonction*;
block: OPENCROCHET instruction CLOSECROCHET ;

instructions: (instruction)* ;

instruction: return_stmt  SEMI
            | declaration  SEMI 
            | expr  SEMI
            | call SEMI
            ;

decl_fonction: type VAR OPENPAR decl_sans_assignation CLOSEPAR block ;

decl_sans_assignation: type VAR (COMMA type VAR)* ;

declaration: type decl_element (COMMA decl_element)* ;

decl_element: VAR
            | affectation ;


affectation: lvalue EQUAL expr ;

return_stmt: RETURN expr ;

lvalue: VAR ;

expr: OPENPAR expr CLOSEPAR #ExprPar
    | opU expr              #ExprUnary
    | expr opM expr         #MulDiv
    | expr opA expr         #AddSub
    | VAR                   #ExprVar
    | constante             #ExprConst 
    | affectation           #ExprAffectation 
    | call                  #ExprCall   ;


liste_param: expr (COMMA expr)*
            | ;

call: VAR OPENPAR liste_param CLOSEPAR ;



opU: MINUS ;

opA: PLUS | MINUS ;
opM: DIV | MULT ;

type: INT ;

constante: CONSTINT
         | CONSTCHAR ; 


INT: 'int' ;
RETURN : 'return' ;
MAIN: 'main';
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
CONSTINT : [0-9]+ ;
CONSTCHAR : '\''[a-zA-Z0-9]'\'' ;
OPENPAR       : '(';
CLOSEPAR      : ')';
OPENCROCHET   : '{';
CLOSECROCHET  : '}';
SEMI          : ';';
EQUAL         : '=';
PLUS          : '+';
MINUS         : '-';
MULT          : '*';
DIV           : '/';
COMMA         : ',';
COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);