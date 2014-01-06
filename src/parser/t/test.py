import json
import sys

if len(sys.argv) < 2:
    print "ERROR: please provide a JSON formatted test file"
    sys.exit(2)

from emlang import emlang

tests = json.loads( open( sys.argv[1] ).read() )

failed = 0

for test in tests:
  result   = False
  output   = ""
  input    = test.get("data","")
  expected = test.get("result", input)
  model    = emlang.parse(input)
  
  if model != None:
    output   = "\n".join( ["%s" % (l) for l in model] )
    result   = output == expected
  
  if result != test.get( "expected", True ):
    print "Test %s failed:" % test.get("name", "unknown")
    print "GOT -->%s<--\nEXPECTED -->%s<--" % ( output, expected )
    failed += 1
    
print "-" * 23
print "%i tests run.\n%i tests failed.\n" % ( len(tests), failed )
