lib_LTLIBRARIES = libslcdpp.la

libslcdpp_la_SOURCES   = libslcdpp.cpp parseCmdLine.cpp
libslcdpp_la_LDFLAGS   = -version-info 0:5:0  
libslcdpp_la_CPPFLAGS  = -I../include

bin_PROGRAMS            = simple_lcdpp
dist_man_MANS           = ../doc/simple_lcdpp.1
# dist_bin_SCRIPTS = 

nobase_include_HEADERS  = ../include/lcd.hpp ../include/parseCmdLine.hpp
simple_lcdpp_SOURCES    = simple_lcdpp.cpp
simple_lcdpp_CPPFLAGS   = 
simple_lcdpp_LDADD      = libslcdpp.la

ACLOCAL_AMFLAGS = -I m4

