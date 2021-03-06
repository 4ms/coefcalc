#!/bin/bash

# Using "./mk -a" rebuilds all ignoring date-stamps

OPT="-O3 -c -DT_LINUX"
#OPT="-g -c -Wall -pedantic -DDEBUG_ON -DT_LINUX"

[ "$1" = "-a" ] && {
    rm *.o
    shift
}

OBJ=""

for xx in \
  fidlib.c \
  main.c 
do
    obj=${xx%.c}
    obj=${obj%.cxx}
    obj=$obj.o
    obj=${obj#fidlib/}
    if [ ! -f $obj ] || [ $xx -nt $obj ]
    then
	echo === $xx
	gcc $OPT $xx || { echo "FAILED"; exit 1; }
    fi
    OBJ="$OBJ $obj"
done

gcc $OBJ -o ./calc-filters || { echo "FAILED"; exit 1; }

