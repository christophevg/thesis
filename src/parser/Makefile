ANTLR=java -cp antlr-3.1.jar org.antlr.Tool
PYTHON=/opt/local/bin/python2.7

APP=emlang

BUILD_DIR=$(APP)
ANTLR_OUT=antlr.out

PARSER=$(APP)/$(APP)Parser.py

all: $(PARSER)

test: $(PARSER)
	@echo "*** performing $(APP) parser tests"
	@(cd t; $(PYTHON) test.py syntax.json)

$(PARSER): $(APP).g
	@echo "*** generating $(APP) parser"
	@$(ANTLR) -o $(APP) $< > $(ANTLR_OUT) 2>&1 || (cat $(ANTLR_OUT); false)

clean:
	@rm -f $(ANTLR_OUT)
	@(cd $(APP); rm -f $(APP).tokens $(APP)Lexer.py $(APP)Parser.py *.pyc)

.PHONY: test clean
