#
# Small makefile for testing with non-Microsoft compilers
#

include ../Makefile.config
include src.mk

%.h.gch: %.h
	@$(CC) $(CXXFLAGS) -D__SNAP_SEPARATE -I$(CGLIB) -I. -o $@ $<

%.o: %.cpp $(LIBSNAP_HXX)
	@$(CC) $(CXXFLAGS) -I$(CGLIB) -I. -c $< -o $@

all: testSnap Snap.o

libsnap.a: $(LIBSNAP_OBJS)
	@echo AR libsnap.a && ar rcs libsnap.a $(LIBSNAP_OBJS)

testSnap: testSnap.o libsnap.a 
	$(CC) $(CXXFLAGS) -o testSnap testSnap.o libsnap.a -I$(CGLIB) $(LDFLAGS) $(LIBS)

Snap.o: $(LIBSNAP_CXX) $(LIBSNAP_HXX) Snap.cpp
	$(CC) -c $(CXXFLAGS) Snap.cpp -I$(CGLIB) -o Snap.o

lib: Snap.o
	rm -f libsnap.a
	ar -cvq libsnap.a Snap.o

libinstall: lib
	cp libsnap.a /usr/local/lib/libsnap.a

clean:
	rm -f $(LIBSNAP_OBJS) *.a testSnap testSnap.exe *.plt *.tab *.png

