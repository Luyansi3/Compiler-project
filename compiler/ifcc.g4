grammar ifcc;

axiom : prog EOF ;

prog : decl_fonction* INT MAIN OPENPAR CLOSEPAR block decl_fonction*;
block: OPENCROCHET instructions CLOSECROCHET ;

instructions: (instruction)* ;

instruction: return_stmt  SEMI      #InstrReturn
            | declaration  SEMI     #InstrDecl
            | expr  SEMI            #InstrExpr
            | call SEMI             #InstrCall
            | if_stmt               #InstrIf_stmt
            | while_stmt            #InstrWhile_stmt
            | block                 #InstrBlock
            ;

decl_fonction: type VAR OPENPAR decl_params CLOSEPAR block ;

decl_params: decl_param (COMMA decl_param)*
            | ;

decl_param : type VAR ;


declaration: type decl_element (COMMA decl_element)* ;

decl_element: VAR
            | affectation ;

if_stmt: IF OPENPAR expr CLOSEPAR (instruction | block) (elif_stmt)* (else_stmt)? ;

elif_stmt: ELSE IF OPENPAR expr CLOSEPAR (instruction | block) ;

else_stmt: ELSE (instruction | block) ;

while_stmt: WHILE OPENPAR expr CLOSEPAR (instruction | block) ;

affectation: lvalue EQUAL expr ;

return_stmt: RETURN expr ;

lvalue: VAR ;

expr: OPENPAR expr CLOSEPAR #ExprPar
    | opU expr              #ExprUnary
    | expr opM expr         #MulDiv
    | expr opA expr         #AddSub
    | expr compRelationnal expr    #ExprCompRelationnal
    | expr compEqual expr    #ExprCompEqual
    | expr AND expr         #ExprAnd
    | expr OR expr           #ExprOr
    | affectation           #ExprAffectation 
    | call                  #ExprCall   
    | VAR                   #ExprVar
    | constante             #ExprConst ;


liste_param: expr (COMMA expr)*
            | ;

call: VAR OPENPAR liste_param CLOSEPAR ;



compRelationnal: INF | SUP ;
compEqual: EQ | NEQ ;

opU: MINUS | NOT;

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
NOT           : '!';
MULT          : '*';
DIV           : '/';
COMMA         : ',';
COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);