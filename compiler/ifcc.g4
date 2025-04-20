grammar ifcc;

axiom : prog EOF ;

prog : pre_decl_fonction* INT MAIN OPENPAR CLOSEPAR block post_decl_fonction*;
block: OPENCROCHET instructions CLOSECROCHET ;

instructions: (instruction)* ;

instruction: return_stmt  SEMI      #InstrReturn
            | declaration  SEMI     #InstrDecl
            | call SEMI             #InstrCall
            | expr  SEMI            #InstrExpr
            | if_stmt               #InstrIf_stmt
            | while_stmt            #InstrWhile_stmt
            | block                 #InstrBlock
            ;

pre_decl_fonction: typeFonc VAR OPENPAR decl_params CLOSEPAR block ;
post_decl_fonction: typeFonc VAR OPENPAR decl_params CLOSEPAR block ;

decl_params: decl_param (COMMA decl_param)*
            | ;

decl_param : type VAR ;


declaration: type decl_element (COMMA decl_element)* ;

decl_element: VAR (OPENBRACKET constante CLOSEBRACKET)?                         # ClassicDeclaration
            | VAR OPENBRACKET (constante)? CLOSEBRACKET EQUAL array_litteral    # TableAffectation
            | VAR EQUAL expr                                                    # VarAffectation ;

if_stmt: IF OPENPAR expr CLOSEPAR (instruction | block) (elif_stmt)* (else_stmt)? ;

elif_stmt: ELSE IF OPENPAR expr CLOSEPAR (instruction | block) ;

else_stmt: ELSE (instruction | block) ;

while_stmt: WHILE OPENPAR expr CLOSEPAR (instruction | block) ;

affectation: lvalue op_compose expr;

op_compose: EQUAL | PLUSEQUAL | MOINSEQUAL | MULTEQUAL | DIVEQUAL | SHLEQUAL | SHREQUAL | XOREQUAL | OREQUAL | ANDEQUAL | MODEQUAL;

array_litteral : OPENCROCHET (expr (COMMA expr)*)? CLOSECROCHET;
return_stmt: RETURN expr ;

lvalue: VAR (OPENBRACKET expr CLOSEBRACKET)? ;


expr: OPENPAR expr CLOSEPAR #ExprPar
    | lvalue opD            #ExprSuffixe
    | opD lvalue            #ExprPrefixe
    | opU expr              #ExprUnary
    | expr opM expr         #ExprMulDivMod
    | expr opA expr         #ExprAddSub
    | expr opS expr         #ExprShift
    | expr compRelationnal expr    #ExprCompRelationnal
    | expr compEqual expr    #ExprCompEqual
    | expr ANDBIT expr      #ExprAndBit
    | expr XORBIT expr      #ExprXorBit
    | expr ORBIT expr       #ExprOrBit
    | expr AND expr         #ExprAnd
    | expr OR expr           #ExprOr
    | affectation           #ExprAffectation 
    | call                  #ExprCall   
    | VAR                   #ExprVar
    | constante             #ExprConst 
    | VAR OPENBRACKET expr CLOSEBRACKET #ExprTable;


opD: PLUSPLUS | MOINSMOINS;


liste_param: expr (COMMA expr)*
            | ;

call: VAR OPENPAR liste_param CLOSEPAR ;



compRelationnal: INF | SUP ;
compEqual: EQ | NEQ ;

opU: MINUS | NOT | PLUS;

opA: PLUS | MINUS ;
opM: DIV | MULT | MOD;
opS: SHL | SHR;

typeFonc : type | VOID ;
type: INT ;

constante: CONSTINT
         | CONSTCHAR ; 

VOID: 'void' ;
INT: 'int' ;
RETURN : 'return' ;
MAIN: 'main';
IF: 'if';
WHILE: 'while' ;
ELSE: 'else';
VAR : [a-zA-Z_][a-zA-Z0-9_]* ;
CONSTINT : [0-9]+ ;
CONSTCHAR : '\'' ( '\\' [btnrf\\'"0] | ~['\\\r\n] ) '\'' ;
PLUSPLUS: '++';
MOINSMOINS: '--';
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

OPENBRACKET   : '[';
CLOSEBRACKET  : ']';

SEMI          : ';';
PLUS          : '+';
MINUS         : '-';
MOD           : '%';
SHL           : '<<';
SHR           : '>>';
ANDBIT        : '&';
ORBIT         : '|';
XORBIT        : '^';
NOT           : '!';
MULT          : '*';
DIV           : '/';
EQUAL         : '=';
PLUSEQUAL     : '+=';
MOINSEQUAL    : '-=';
MULTEQUAL     : '*=';
DIVEQUAL      : '/=';
SHLEQUAL      : '<<=';
SHREQUAL      : '>>=';
XOREQUAL      : '^=';
OREQUAL       : '|=';
ANDEQUAL      : '&=';
MODEQUAL      : '%=';
COMMA         : ',';
COMMENT : '/*' .*? '*/' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);