#
# Makefile for non-Microsoft compilers
#

include ./src.mk
all: $(all_depends)

MakeAll:
	mkdir -p Release
	$(MAKE) -C agmfit
	$(MAKE) -C agmgen
	$(MAKE) -C bigclam
	$(MAKE) -C cascadegen
	$(MAKE) -C cascades
	$(MAKE) -C centrality
	$(MAKE) -C cesna
	$(MAKE) -C circles
	$(MAKE) -C cliques
	$(MAKE) -C coda
	$(MAKE) -C community
	$(MAKE) -C concomp
	$(MAKE) -C flows
	$(MAKE) -C forestfire
	$(MAKE) -C graphgen
	$(MAKE) -C graphhash
	$(MAKE) -C infopath
	$(MAKE) -C kcores
	$(MAKE) -C knnjaccardsim
	$(MAKE) -C kronem
	$(MAKE) -C kronfit
	$(MAKE) -C krongen
	$(MAKE) -C localmotifcluster
	$(MAKE) -C lshtest
	$(MAKE) -C magfit
	$(MAKE) -C maggen
	$(MAKE) -C mkdatasets
	# 'motifcluster' compilation skipped by default, it requires ARPACK
	#$(MAKE) -C motifcluster
	$(MAKE) -C motifs
	$(MAKE) -C ncpplot
	$(MAKE) -C netevol
	$(MAKE) -C netinf
	$(MAKE) -C netstat
	$(MAKE) -C node2vec
	$(MAKE) -C pool
	$(MAKE) -C randwalk
	$(MAKE) -C rolx
	$(MAKE) -C temporalmotifs
	$(MAKE) -C testgraph

clean: $(clean_all_depends)
	@echo "Cleaned all example subdirectories"

bin-linux:
	mv Release Snap-$(VERSION)-Ubuntu18.04
	tar zcvf Snap-$(VERSION)-Ubuntu18.04.tgz Snap-$(VERSION)-Ubuntu18.04
	mv Snap-$(VERSION)-Ubuntu18.04 Release

bin-mac:
	mv Release Snap-$(VERSION)-macOS14.10
	tar zcvf Snap-$(VERSION)-macOS14.10.tgz Snap-$(VERSION)-macOS14.10
	mv Snap-$(VERSION)-macOS14.10 Release

bin-win:
	mv Release Snap-$(VERSION)-Win32
	zip -r Snap-$(VERSION)-Win32.zip Snap-$(VERSION)-Win32/*.exe
	mv Snap-$(VERSION)-Win32 Release

