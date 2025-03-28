grammar ifcc;

axiom : prog EOF ;

prog : INT MAIN OPENPAR CLOSEPAR block ;
block: OPENCROCHET instructions CLOSECROCHET ;

instructions: (instruction)* ;

instruction: return_stmt  SEMI
            | declaration  SEMI 
            | expr  SEMI
            | call SEMI
            | if_stmt
            | while_stmt
            ;

declaration: type decl_element (COMMA decl_element)* ;

decl_element: VAR
            | affectation ;

if_stmt: IF OPENPAR expr CLOSEPAR block (elif_stmt)? (else_stmt)? ;

elif_stmt: ELSE IF OPENPAR expr CLOSEPAR block (elif_stmt)? ;

else_stmt: ELSE block ;

while_stmt: WHILE OPENPAR expr CLOSEPAR block ;

affectation: lvalue EQUAL expr ;

return_stmt: RETURN expr ;

lvalue: VAR ;

expr: OPENPAR expr CLOSEPAR #ExprPar
    | opU expr              #ExprUnary
    | expr opM expr         #MulDiv
    | expr opA expr         #AddSub
    | expr comp expr    #ExprComp
    | expr AND expr         #ExprAnd
    | expr OR expr           #ExprOr
    | VAR                   #ExprVar
    | constante             #ExprConst 
    | affectation           #ExprAffectation 
    | call                  #ExprCall   ;


liste_param: expr (COMMA expr)*
            | ;

call: VAR OPENPAR liste_param CLOSEPAR ;



comp: EQ | NEQ | INF | SUP ;

opU: MINUS ;

opA: PLUS | MINUS ;
opM: DIV | MULT ;

type: INT ;

constante: CONSTINT
         | CONSTCHAR ; 


INT: 'int' ;
RETURN : 'return' ;
MAIN: 'main';
IF: 'if';
WHILE: 'while' ;
ELSE: 'else';
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
CONSTINT : [0-9]+ ;
CONSTCHAR : '\''[a-zA-Z0-9]'\'' ;
EQ: '==';
NEQ: '!=';
INF: '<';
SUP: '>';
AND: '&&';
OR: '||';
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