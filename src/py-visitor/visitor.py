# Experiment to setup a transparent, but also feature-complete visitor pattern
# author: Christophe VG

# The abstract Visitor and Visitable base-class, providing the redirection

import inspect

class Visitable(object):
  """
  Baseclass for visitable classes
  """
  def accept(self, visitor):
    class_name  = self.__class__.__name__
    method_name = "handle_" + class_name
    try:
      return getattr(visitor, method_name)(self)
    except AttributeError:
      print visitor.__class__.__name__ + " doesn't handle " + class_name

class Visitor(object):
  """
  Baseclass for visitors.
  """
  pass

class visitor_for(object):
  """
  Decorator @visit(superclass) makes a decorated class a visitor for all classes in
  the module of the decorated class. The visitable classes can be limited to
  a common superclass.
  """
  def __init__(self, super=None):
    self.super = super

  def __call__(self, visitor):
    # make the visitor a subclass of Visitor
    visitor = type(visitor.__name__, (Visitor,), dict(visitor.__dict__))

    # retrieve calling module and its classes
    frame   = inspect.stack()[1]
    module  = inspect.getmodule(frame[0])
    classes = inspect.getmembers(module, inspect.isclass)

    # dynamically adding all Stmt subclasses tot the visitor
    def NotImplemented(name):
      def dummy(self, *args):
        print name + " is not implemented in " + self.__class__.__name__
      return dummy

    for name, clazz in classes:
      if super == None or issubclass(clazz, self.super):
        setattr(visitor, "handle_" + name, NotImplemented("handle_" + name))

    return visitor

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

class ReturnStmt(Stmt): pass

@visitor_for(Stmt)
class CodeVisitor(): pass

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
