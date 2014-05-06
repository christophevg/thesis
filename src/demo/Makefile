ifndef HEARTBEAT
HEARTBEAT=no
endif

ifndef REPUTATION
REPUTATION=no
endif

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
ifeq ($(HEARTBEAT), yes)
SRCS+=$(FOO-LANG)/examples/heartbeat.foo
VARS+=WITH_CRYPTO=yes
VARS+=WITH_TIME=yes
endif

ifeq ($(REPUTATION), yes)
SRCS+=$(FOO-LANG)/examples/reputation.foo
VARS+=WITH_TIME=yes
endif

all: clean generate beautify build

generate: $(OUTPUT)
	@echo "*** generating code from foo-lang sources:" $(SRCS)
	@$(FOO) -g code -i -c -o $(OUTPUT) -p demo $(SRCS)
	@cp Makefile.gen $(OUTPUT)/Makefile

beautify:
	@echo "*** beautifying generated sources"
	@$(ASTYLE) --recursive "$(OUTPUT)/*.*"

build:
	@echo "*** building generated sources"
	@(cd $(OUTPUT); $(VARS) make clean all)

$(OUTPUT):
	@mkdir -p $@

clean:
	@rm -rf $(OUTPUT)

.PHONY: clean generate beautify build
