lib_LTLIBRARIES = libslcd.la

libslcd_la_SOURCES   = lcd_write.c 
libslcd_la_LDFLAGS   = -version-info 0:5:0  
libslcd_la_CPPFLAGS  = -I../include

bin_PROGRAMS            = simple_lcd
dist_man_MANS           = ../doc/simple_lcd.1
# dist_bin_SCRIPTS = 

nobase_include_HEADERS  = ../include/lcd_write.h 
simple_lcd_SOURCES        = simple_lcd.c
simple_lcd_CPPFLAGS       = 
simple_lcd_LDADD          = libslcd.la

ACLOCAL_AMFLAGS = -I m4

