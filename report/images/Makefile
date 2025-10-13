FIGURE_DATA = agmfit.json
FIGURE_DATA += agmcpu.json
PYTHON = python

%.pdf: %.py $(FIGURE_DATA)
	@$(PYTHON) $<

agmfit-base.tex: agmfit.base.txt util.py tablegen.py
	@$(PYTHON) tablegen.py  agmfit.base.txt agmfit-base.tex
agmfit-opt.tex: agmfit.opt.txt util.py tablegen.py
	@$(PYTHON) tablegen.py  agmfit.opt.txt agmfit-opt.tex
centr-base.tex: centr.base.txt util.py tablegen.py
	@$(PYTHON) tablegen.py  centr.base.txt centr-base.tex

FIGURES = ompdemo.pdf
FIGURES += agmcomplete.pdf
FIGURES += agmcpu.pdf
TABLES = agmfit-base.tex agmfit-opt.tex
TABLES += centr-base.tex

all: $(FIGURES) $(TABLES)

.PHONY: clean
clean:
	-rm -f $(FIGURES) $(TABLES)
