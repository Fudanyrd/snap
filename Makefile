#
# Makefile for non-Microsoft compilers
#

all: MakeAll

test: TestAll

# MAKEFLAGS += -j8

MakeAll:
	@./init
	$(MAKE) -C snap-core $(MAKEFLAGS)
	$(MAKE) all -C examples $(MAKEFLAGS)

TestAll:
	$(MAKE) run -C test

clean:
	$(MAKE) clean -C snap-core
	$(MAKE) clean -C examples
	$(MAKE) clean -C test
	$(MAKE) clean -C tutorials
