# emlang.py
# author: Christophe VG

# classes to construct the AST using the ANTLR3-based emlang parser

class emlang_base():
  # entry point of request for conversion to string
  def __repr__(self):
    return self.emlang()

  def emlang(self,indent):
    print "WARNING: need to implement emlang(self, indent)"

class assignment(emlang_base):
  def __init__(self, variable, expression):
    self.variable   = variable
    self.expression = expression

  def emlang(self, indent=0):
    return " " * indent + self.variable + " = " + self.expression.emlang(indent);

class boolean(emlang_base):
  def __init__(self, value=None):
    self.value = value

  def emlang(self, indent=0):
    return " " * indent + "true" if self.value == True else "false"

class integer(emlang_base):
  def __init__(self, value=None):
    self.value = value

  def emlang(self, indent=0):
    return " " * indent + str(self.value)

class floating(emlang_base):
  def __init__(self, value=None):
    self.value = value

  def emlang(self, indent=0):
    return " " * indent + str(self.value)
  
import antlr3

from emlangLexer  import emlangLexer
from emlangParser import emlangParser

def parse(string):
  cStream = antlr3.StringStream(string)
  lexer   = emlangLexer(cStream)
  tStream = antlr3.CommonTokenStream(lexer)
  parser  = emlangParser(tStream)

  return parser.compilation_unit()
