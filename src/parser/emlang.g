grammar emlang;

options {
    language=Python;
    backtrack=true;
}

@header {
  from emlang import *
}

compilationUnit returns [list] @init { list = [] }
                : statements? { list = $statements.list }
                ;

statements returns [list] @init { list = [] }
           : (statement { list.append($statement.instance) })+
           ;

statement returns [instance]
          : directive { instance = $directive.instance }
          | construct { instance = $construct.instance }
          ;

directive returns [instance]
          :  INCLUDEDIRECTIVE stringliteral 
             { instance = Directive( "include", [$stringliteral.instance] ) }
	        ;

construct returns [instance]
          : annotations? prefix_modifiers=modifiers? 
            type name? supers? suffix_modifiers=modifiers? value? children 
{
  instance = Construct( type        = $type.value, 
                        name        = $name.value, 
                        annotations = $annotations.list,
                        supers      = $supers.list,
                        value       = $value.instance,
                        children    = $children.list )
  if prefix_modifiers != None: instance.modifiers.extend(prefix_modifiers);
  if suffix_modifiers != None: instance.modifiers.extend(suffix_modifiers);
};

annotations returns [list] @init { list = [] }
            : ( ANNOTATION 
                { list.append( Annotation($ANNOTATION.getText()[2:-1])) } 
              )+
			      ;

type returns [value] 
     : IDENTIFIER generic?
{
  $value = $IDENTIFIER.getText()
  if $generic.value != None: $value += $generic.value
}
     ;

generic returns [value]
        : '<' identifiers '>' { $value = "<" + ",".join($identifiers.list) + ">" }
        ;

identifiers returns [list]
            : first=IDENTIFIER { $list = [ first.getText() ] }
              ( ',' more=IDENTIFIER )* { if more != None: $list.append( more.getText() ) }
            ;

name returns [value]
     : IDENTIFIER generic? 
{
  $value = $IDENTIFIER.getText() 
  if $generic.value != None: $value += $generic.value
}
     ;

modifiers returns [list] @init { list = [] }
            : ( modifier { list.append($modifier.instance) } )+
			      ;

modifier returns [instance] 
         : '+' IDENTIFIER { instance = Modifier( name=$IDENTIFIER.getText()) }
         | '+' IDENTIFIER '=' literal 
{ 
  instance = Modifier( name=$IDENTIFIER.getText(), value = $literal.instance ) 
}
         ;

supers returns [list] @init { list = [] }
       : ( super { list.append($super.instance) } )+
       ;

super returns [instance]
      : ':' type { instance = Reference($type.value) }
      ;

value returns [instance] 
      : '<=' literal { $instance = Value($literal.instance) }
      ;

literal returns [ instance ]
        : booleanliteral { $instance = $booleanliteral.instance }
        | integerliteral { $instance = $integerliteral.instance }
		    | stringliteral  { $instance = $stringliteral.instance  }
	      ;

booleanliteral returns [instance]
               : BOOLEAN 
                 {  
                   value = True if $BOOLEAN.getText() == "true" else False
                   $instance = Boolean(value)
                 }
               ;

integerliteral returns [instance]
               : INTEGER { $instance = Integer(int($INTEGER.getText())) }
               ;

stringliteral returns [instance]
              : STRING { $instance = String($STRING.getText()[1:-1]) }
              ;

children returns [list] 
   : '{' statements '}' { $list = $statements.list }
	 | ';'                { $list = [] }
	 ;

// TOKENS
BOOLEAN : 'true'
	| 'false'
	;

IDENTIFIER : ('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
    	   ;

INTEGER : ('+'|'-')?('0'..'9')+
        ;

STRING :  '"' ~('"')+ '"'
       |  '\'' ~('\'')+ '\''
       ;

ANNOTATION
           : '[@' ( options {greedy=false;} : . )* ']' 
	         ;

INCLUDEDIRECTIVE : '#include'
		 ;
WS  :  (' '|'\r'|'\t'|'\u000C'|'\n') { $channel=HIDDEN }
    ;

COMMENT
    :   '/*' ( options {greedy=false;} : . )* '*/' { $channel=HIDDEN }
    ;

LINE_COMMENT
    : '//' ~('\n'|'\r')* '\r'? '\n' { $channel=HIDDEN }
    ;
 
