FOO-LANG=../foo-lang
CODECANVAS=$(FOO-LANG)/lib/codecanvas/src
PY-UTIL=$(FOO-LANG)/lib/py-util/src
PYTHON=PYTHONPATH=$(CODECANVAS):$(PY-UTIL):$(FOO-LANG) /opt/local/bin/python2.7
ASTYLE=/opt/local/bin/astyle --style=attach --indent=spaces=2 \
														 --indent-col1-comments --suffix=none \
														 --quiet --errors-to-stdout \
														 --max-code-length=80

FOO=$(PYTHON) $(FOO-LANG)/foo.py

OUTPUT=generated
SRCS+=$(FOO-LANG)/examples/heartbeat.foo 
SRCS+=$(FOO-LANG)/examples/reputation.foo

all: clean generate beautify build

generate: $(OUTPUT)
	@echo "*** generating code from foo-lang sources:" $(SRCS)
	@$(FOO) -g code -i -c -o $(OUTPUT) $(SRCS)
	@cp Makefile.gen $(OUTPUT)/Makefile

beautify:
	@echo "*** beautifying generated sources"
	@$(ASTYLE) --recursive "$(OUTPUT)/*.*"

build:
	@echo "*** building generated sources"
	@(cd $(OUTPUT); make clean all)

$(OUTPUT):
	@mkdir -p $@

clean:
	@rm -rf $(OUTPUT)

.PHONY: clean generate beautify build
