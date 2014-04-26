FOO-LANG=../foo-lang
CODECANVAS=$(FOO-LANG)/lib/codecanvas/src
PY-UTIL=$(FOO-LANG)/lib/py-util/src
PYTHON=PYTHONPATH=$(CODECANVAS):$(PY-UTIL):$(FOO-LANG) /opt/local/bin/python2.7

FOO=$(PYTHON) $(FOO-LANG)/foo.py

OUTPUT=generated
SRCS=$(FOO-LANG)/examples/heartbeat.foo $(FOO-LANG)/examples/reputation.foo

all: generate

generate: $(OUTPUT)
	@echo "*** generating code from foo-lang sources:" $(SRCS)
	@$(FOO) -g code -i -c -o $(OUTPUT) $(SRCS)

$(OUTPUT):
	@mkdir -p $@

clean:
	@rm-rf $(OUTPUT)

.PHONY: clean generate
