AM_LDFLAGS = `libpng-config --ldflags` -lpng -lz
AM_CFLAGS = `libpng-config --I_opts` -Wall --pedantic -std=gnu99
LDADD = -lpng

bin_PROGRAMS = pngnq-s9 pngcomp
pngnq_s9_SOURCES = pngnq.c neuquant32.c rwpng.c  neuquant32.h rwpng.h errors.h nqcvector.h pngnqhelp.h
pngcomp_SOURCES = pngcomp.c rwpng.c colorspace.c  colorspace.h
