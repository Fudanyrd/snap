#!/usr/bin/env bash
set -ex
CXXFLAGS="-std=c++98 -Wall -O3 -DNDEBUG -fopenmp -ffast-math -g -rdynamic -ggdb -I../glib-core"

tmpf=$( mktemp "/tmp/tmp.XXXXXXXXXX.cc" )
tmpo=$( mktemp "/tmp/tmp.XXXXXXXXXX.o"  )

for f in $( ls *.cpp ../glib-core/*.cpp ); do
    fail=0
    g++ $CXXFLAGS -c $f -o $tmpo >/dev/null 2>&1 || fail=1
    # if compilation failed, add proper include path.
    if [[ $fail -eq 1 ]]; then
        /usr/bin/printf "\x23 include \x22Snap.h\x22 \xa\xa" > $tmpf
        cat $f >> $tmpf
        cp $tmpf $f
    fi
done

rm -f $tmpf $tmpo

