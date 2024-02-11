Command line options:
======================

You can print these instructions usinf -h, example:

./simple_lcd.py -h


./simple_lcd.py [-1234 message] | [-s message] | [-p message] | [-h] 
-1 or --first 
        Insert "message" in the first LCD row.

-2 or --second 
        Insert "message" in the second LCD row.

-3 or --third 
        Insert "message" in the third LCD row.

-4 or --fourth 
        Insert "message" in the fourth LCD row.

-D or --dump   
        Dump all bytes sento to the attacched LCD 

-s or --split  
        Split the initial part of "message" using the four rows
        The part of "message" that exceeds  the available LCD space
        will be ignored.

-p or --page   
        Insert all "message" characters, one page at time, using
        all LCD's available space. So, at the end, the entire "message" will be printed.

-h or --help 
        Print this help synopsis.

Environment:
===========

* LCD_CONFIG_FILE specifies full path of configuration file. If it isn't present, program will look for ./simpleLcd.conf
* Example:
```
LCD_CONFIG_FILE=/home/bg/_Service_Scripts/_LcdDaemon/simpleLcd.conf
```

Configuration:
==============

This program needs a configuration file,  simpleLcd.conf.
Tha files contains few sections with configuration data:

* [filesystem]
* [lcd]
* [i2c]
* [time]


[filesystem]
=============

This section contains the directory and the file name of the log file, example:

 LOGDIR=.
 LOGNAME=lcd.log

[lcd]
=====

This section contains LCD's  "geometry", that is the number or rows and columns, example:

 columns=16
 rows=4

[i2c]
======

This section contains treferences to the device and address assigned to the LCD, example:

 port=1
 address=0x27

port is the distinctive number of the related special file, so 1 means /dev/i2c-1 .

[time]
======

This section specifies time delays needed for some operation, example:

 pager=3

"pager" is the delay applied befor printing next page, "wait"
