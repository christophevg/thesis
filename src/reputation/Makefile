SOURCE=paper
OCTAVE=LC_CTYPE=C /usr/local/bin/octave -q -p $(SOURCE)
PACKETS=50

all:
	@make paper.eps SOURCE=paper
	@make simple.eps SOURCE=simple

%.mat:
	@$(OCTAVE) --eval "simulate( $(PACKETS), '$@' );"

%.eps: %.mat
	@$(OCTAVE) --eval "plot_trust( '$<', '$@' ); " 2> /dev/null

mrproper:
	@rm -f *.eps *.mat

.PRECIOUS: %.mat %.eps
.SECONDARY: %.mat %.eps
