FIGURE_DATA = agmfit.json

%.pdf: %.py $(FIGURE_DATA)
	@python $<

agmfit-base.tex: agmfit.base.txt util.py tablegen.py
	@python tablegen.py  agmfit.base.txt agmfit-base.tex
agmfit-opt.tex: agmfit.opt.txt util.py tablegen.py
	@python tablegen.py  agmfit.opt.txt agmfit-opt.tex

FIGURES = ompdemo.pdf
FIGURES += agmcomplete.pdf
TABLES = agmfit-base.tex agmfit-opt.tex

all: $(FIGURES) $(TABLES)

.PHONY: clean
clean:
	rm -f $(FIGURES) $(TABLES)
