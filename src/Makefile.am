INCLUDES =

filterdir = @CUPSDIR@/lib/cups/filter

filter_PROGRAMS = rastertosii
rastertosii_SOURCES = rastertosii.c rastertosii.h sii_cmd.c sii_cmd.h sii.h ImgDither.c ImgDither.h
rastertosii_LDADD = -lm -lcups -lcupsimage
rastertosii_LDFLAGS = -Wl,--rpath -Wl,@CUPSDIR@/lib
