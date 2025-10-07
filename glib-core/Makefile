include ./src.mk

all: libglib.a

%.h.gch: %.h
	g++ $< -D__SNAP_SEPARATE -o $@

%.o: %.cpp $(LIBGLIB_HXX)
	g++ $< -D__SNAP_SEPARATE -c -O2 -o $@

libglib.a: $(LIBGLIB_OBJS)
	ar rcs libglib.a $(LIBGLIB_OBJS)

.PHONY: clean
clean:
	-rm *.h.gch *.o *.a
 
