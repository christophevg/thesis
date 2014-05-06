// this is a little trick to be able to use externally defined variables
// to use it, add one more #define <name-of-var>_name = STR(<name-of-var>)
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
