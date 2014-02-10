# Experiment to setup a transparent, but also feature-complete visitor pattern
# author: Christophe VG

# The abstract Visitor and Visitable base-class, providing the redirection

class Visitable(object):
  def accept(self, visitor):
    class_name  = self.__class__.__name__
    method_name = "handle_" + class_name
    try:
      return getattr(visitor, method_name)(self)
    except AttributeError:
      print visitor.__class__.__name__ + " doesn't handle " + class_name

class Visitor(object):
  pass

# The Code Model with an unimplemented CodeVisitor

class Stmt(Visitable):
  pass

class IfStmt(Stmt):
  def __init__(self, condition, true, false):
    self.condition = condition
    self.true      = true
    self.false     = false

class PrintStmt(Stmt):
  def __init__(self, data):
    self.data = data

class ReturnStmt(Stmt):
  pass

class CodeVisitor(Visitor):
  pass

import sys
import inspect

# dynamically adding all Stmt subclasses tot the visitor
def NotImplemented(name):
  def dummy(self, *args):
    print name + " is not implemented in " + self.__class__.__name__
  return dummy

classes = inspect.getmembers(sys.modules[__name__], inspect.isclass)
for name, clazz in classes:
  if issubclass(clazz, Stmt):
    setattr(CodeVisitor, "handle_" + name, NotImplemented("handle_" + name))

# A first CodeVisitor, interpreting

class Interpreter(CodeVisitor):

  def handle_IfStmt(self, stmt):
    if stmt.condition:
      stmt.true.accept(self)
    else:
      stmt.false.accept(self)

  def handle_PrintStmt(self, stmt):
    print stmt.data

# A different CodeVisitor, emitting code

class Emitter(CodeVisitor):
  
  def handle_IfStmt(self, stmt):
    return "if(" + str(stmt.condition) + ") { " + \
           stmt.true.accept(self) + \
           " } else { " + \
           stmt.false.accept(self) + \
           " }"

  def handle_PrintStmt(self, stmt):
    return "print '" + str(stmt.data) + "'"

  def handle_ReturnStmt(self, stmt):
    return "return"

code = IfStmt(True, ReturnStmt(), PrintStmt("false"))

code.accept(Interpreter())
print code.accept(Emitter())
