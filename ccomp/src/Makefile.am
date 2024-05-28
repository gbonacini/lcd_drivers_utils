lib_LIBRARIES = libslcd.a

libslcd_a_SOURCES   = lcd_write.c 
libslcd_a_LDFLAGS   = -version-info 0:5:0  
libslcd_a_CPPFLAGS  = -I../include

bin_PROGRAMS            = simple_lcd
dist_man_MANS           = ../doc/simple_lcd.1
# dist_bin_SCRIPTS = 

nobase_include_HEADERS  = ../include/lcd_write.h 
simple_lcd_SOURCES        = simple_lcd.c
simple_lcd_CPPFLAGS       = 
simple_lcd_LDADD          = libslcd.a

ACLOCAL_AMFLAGS = -I m4

