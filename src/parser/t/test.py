# test.py
# author: Christophe VG

# small parser test driver for validating the parsing and stringification of
# snippets of emlang code

import sys
import json

from emlang import emlang
from antlr3 import RecognitionException

if len(sys.argv) < 2:
    print "ERROR: please provide a JSON formatted test file"
    sys.exit(2)

tests = json.loads( open( sys.argv[1] ).read() )

tried  = 0
failed = 0

for test in tests:
  tried    += 1

  result   = False
  output   = ""
  input    = test.get("input","")
  expected = test.get("output", input)
  model    = None

  exception = ""
  try:
    model = emlang.parse(input)
  except RecognitionException as e:
    if test.get( "result", True ):
      print "Test %s failed:" % test.get("name", "unknown")
      print "Exception: %s" % e
      failed += 1
    next

  if model != None:
    output = "\n".join( ["%s" % (l) for l in model] )

  result = output == expected

  if result != test.get( "result", True ):
    print "Test %s failed:" % test.get("name", "unknown")
    print "GOT -->%s<--\nEXPECTED -->%s<--" % ( output, expected )
    failed += 1

print "-" * 23
print "%i tests run.\n%i tests failed.\n" % ( tried, failed )
