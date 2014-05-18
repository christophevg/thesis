SOURCE=paper
OCTAVE=LC_CTYPE=C /usr/local/octave/3.8.0/bin/octave-3.8.0 -q -p $(SOURCE)

all: paper.pdf failure.pdf malicious.pdf

%.mat: %.conf.m %
	@echo "*** performing simulation for $(word 2,$^)"
	@$(OCTAVE) --eval "source('$<');" simulate_$(word 2,$^).m

%.pdf: %.mat %.conf.m %
	@echo "*** plotting trust for $(word 3,$^)"
	@$(OCTAVE) --eval "source('$(word 2,$^)');" \
						 --eval "plot_trust( '$<', '$@' );" > /dev/null

mrproper:
	@rm -f *.eps *.mat

.PHONY: paper failure malicious

.PRECIOUS: %.mat %.eps
.SECONDARY: %.mat %.eps
