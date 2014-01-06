ANTLR=java -cp antlr-3.1.jar org.antlr.Tool
PYTHON=/opt/local/bin/python2.7

APP=emlang

BUILD_DIR=$(APP)

all: clean
	@echo "*** generating parser"
	@$(ANTLR) -o $(APP) $(APP).g

test: all
	@echo "*** performing tests"
	@rm -rf t/$(APP)
	@(cd t; ln -s ../$(BUILD_DIR) .)
	@(cd t; $(PYTHON) test.py syntax.json)
	@rm -rf t/$(APP)

clean:
	@(cd $(APP); rm -f $(APP).tokens $(APP)Lexer.py $(APP)Parser.py *.pyc)
