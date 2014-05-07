ifndef HEARTBEAT
	HEARTBEAT=no
else
	PATCH+=hb
endif

ifndef REPUTATION
	REPUTATION=no
else
	patch+=rep
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

all: build

generate: clean $(OUTPUT) generator
	@echo "*** generating code from foo-lang sources:" $(SRCS)
	@$(FOO) -g code -i -c -o $(OUTPUT) -p demo $(SRCS)
	@cp Makefile.gen $(OUTPUT)/Makefile

generator:
	@(cd $(FOO-LANG); make parser)

beautify: generate
	@echo "*** beautifying generated sources"
	@$(ASTYLE) --recursive "$(OUTPUT)/*.*"

patch: beautify
ifdef PATCH
	@echo "*** adding logging statements from $(PATCH).diff"
	@(cd $(OUTPUT); patch -s < ../$(PATCH).diff)
endif

make-patch: beautify
	@echo "*** creating patch"
	@diff -rupN $(OUTPUT) $(OUTPUT)-$(PATCH) > $(PATCH).diff || exit 0

build: patch
	@echo "*** building generated sources"
	@(cd $(OUTPUT); $(VARS) make clean all)

program: build
	@(cd $(OUTPUT); make program)

$(OUTPUT):
	@mkdir -p $@

clean:
	@rm -rf $(OUTPUT)

.PHONY: clean generate beautify build
