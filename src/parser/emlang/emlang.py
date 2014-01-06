class EmlangBase():
  def __repr__(self):
    return self.emlang()
  def emlang(self,indent):
    print "WARNING: need to implement EmlangBase::emlang(self,indent)"

class Directive(EmlangBase):
  def __init__(self, command=None, args=None):
    self.command = command
    self.args    = []
    if args != None: self.args.extend(args)

  def emlang(self, indent=0):
    return " " * indent + "#%s %s" % (self.command," ".join(["%s" % (a) for a in self.args]))

class Construct(EmlangBase):
  def __init__(self, type=None, name=None, value=None, 
               modifiers=None, annotations=None, supers=None, children=None ):
    self.type        = type
    self.name        = name
    self.value       = value
    self.modifiers   = []
    self.annotations = []
    self.supers      = []
    self.children    = []
    if modifiers   != None: self.modifiers.extend(modifiers)
    if annotations != None: self.annotations.extend(annotations)
    if supers      != None: self.supers.extend(supers)
    if children    != None: self.children.extend(children)
  
  def emlang(self, indent=0):
    string = ""
    if self.annotations != []:
      string += "\n".join(["%s" % (a.emlang(indent)) for a in self.annotations])
      string += "\n"
    string += " " * indent + self.type
    if self.name != None: string += " " + self.name
    if self.supers != []:
      string += " " + " ".join(["%s" % (s.emlang()) for s in self.supers])
    if self.modifiers != []:
      string += " " + " ".join(["%s" % (m.emlang()) for m in self.modifiers])
    if self.value != None: string += " " + self.value.emlang()
    if self.children != []:
      string += " {\n" + "\n".join(["%s" % (c.emlang(indent+2)) for c in self.children]) + "\n" + " " * indent + "}"
    else:
      string += ";"
    
    return string

class Annotation(EmlangBase):
  def __init__(self, body=None):
    self.body = body
  
  def emlang(self, indent=0):
    return " " * indent + "[@%s]" % self.body

class Modifier(EmlangBase):
  def __init__(self, name=None, value=None):
    self.name  = name
    self.value = value
  
  def emlang(self, indent=0):
    return " " * indent + "+" +  self.name + ("" if self.value == None else "=" + str(self.value))

class Reference(EmlangBase):
  def __init__(self, target=None):
    self.target = target

  def emlang(self, indent=0):
    return " " * indent + ": " + self.target

class Value(EmlangBase):
  def __init__(self, value=None):
    self.value = value

  def emlang(self, indent=0):
    return " " * indent + "<= %s" % self.value

class String(EmlangBase):
  def __init__(self, value=None):
    self.value = value

  def emlang(self, indent=0):
    return " " * indent + "\"" + self.value + "\""

class Boolean(EmlangBase):
  def __init__(self, value=None):
    self.value = value

  def emlang(self, indent=0):
    return " " * indent + "true" if self.value == True else "false"

class Integer(EmlangBase):
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

  return parser.compilationUnit()
