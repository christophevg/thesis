// emlang.g
// author: Christophe VG

// ANTLR3 grammar for emlang, a small PoC language for my master's thesis

grammar emlang;

options {
  language=Python;
  backtrack=true;
}

// to have our parser raise its exceptions we need to override some methods in
// the lexer
// trick found at:
// http://www.dalkescientific.com/writings/diary/archive/2007/11/03/

@lexer::members {
def reportError(self, e):
   raise e
}

@rulecatch {
except RecognitionException, e:
    raise
}

@header {
  from emlang import *
}


// PARSING RULES

compilation_unit returns [list] @init { list = [] }
                 : statements? { list = $statements.list }
                 ;

statements returns [list] @init { list = [] }
           : (statement { list.append($statement.instance) })+
           ;

statement returns [instance]
          : assignment { instance = $assignment.instance }
          ;

assignment returns [instance]
          : IDENTIFIER EQUALS expression {
            instance = assignment($IDENTIFIER.getText(), $expression.instance)
          }
          ;

expression returns [instance]
           : literal { instance = $literal.instance }
           ;

literal returns [ instance ]
        : boolean_literal { $instance = $boolean_literal.instance }
        | float_literal   { $instance = $float_literal.instance   }
        | integer_literal { $instance = $integer_literal.instance }
        ;

boolean_literal returns [instance]
                : BOOLEAN {
                  value = True if $BOOLEAN.getText() == "true" else False
                  $instance = boolean(value)
                }
                ;

float_literal returns [instance]
              : FLOAT { instance = floating( float($FLOAT.getText())) }
              ;

integer_literal returns [instance]
                : INTEGER { instance = integer( int($INTEGER.getText())) }
                ;

// TOKENS

BOOLEAN : 'true'
	| 'false'
	;

FLOAT : ('+'|'-')? ('0'..'9')* '.' ('0'..'9')+
        ;

INTEGER : ('+'|'-')? ('0' | '1'..'9' '0'..'9'*)
        ;

IDENTIFIER : ('A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
      	   ;

ATOM : ('a'..'z') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
     |  '\'' ~('\'')+ '\''
 	   ;

EQUALS : '='
       ;

WS  :  (' '|'\r'|'\t'|'\u000C'|'\n') { $channel=HIDDEN }
    ;

COMMENT
    : '%' ~('\n'|'\r')* '\r'? '\n' { $channel=HIDDEN }
    ;
