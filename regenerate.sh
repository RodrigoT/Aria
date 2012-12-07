#!/bin/bash

#genera aclocal.m4 desde *.m4 y configure.{in,ac}
aclocal
# crea congig.h.in desde aclocal.m4, con las macros genreales
autoheader
# Usa aclocal.m4 y genera configure
autoconf
# conlos MAkefile.am genera los Makefile.in
automake --add-missing
# Genera los Makefile tras testear el sistema para comprobar las capabilidades
./configure 
# Genera laplicaciondesde los Makefile
make
