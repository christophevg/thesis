SOURCE=paper
OCTAVE=LC_CTYPE=C /usr/local/bin/octave -q -p $(SOURCE)
PACKETS=50

all: paper.eps

%.mat: %.conf.m %
	@echo "*** performing simulation for $(word 2,$^)"
	@$(OCTAVE) --eval "source('$<');" simulate_$(word 2,$^).m

%.eps: %.mat %.conf.m %
	@echo "*** plotting trust for $(word 3,$^)"
	@$(OCTAVE) --eval "source('$(word 2,$^)');" \
						 --eval "plot_trust( '$<', '$@' );" 2> /dev/null

mrproper:
	@rm -f *.eps *.mat

.PRECIOUS: %.mat %.eps
.SECONDARY: %.mat %.eps
