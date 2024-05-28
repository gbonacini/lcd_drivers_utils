/*
# -----------------------------------------------------------------
# simple_lcd - a command line tools to print message using Hitachi  
#              HD44780 LCDs connected over I2C on Linux.                   
# Copyright (C) 2020  Gabriele Bonacini
#
# This program is free software for no profit use; you can redistribute
# it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
# -----------------------------------------------------------------
*/

#include <lcd_write.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>

void usage(char* pname);
void nomem(void);
void nodev(void);
void nobus(void);


int main(int argc, char** argv){
	char                 *device = DEFAULT_DEV_PATH,
                         *text   = NULL;
    const int            BASE    = 16;
    const unsigned int   majorno = MAJOR_NUMBER;
	int                  addr    = DEFAULT_DEV_ADDR,
                         ch      = -1,
                         rowMax  = DEFAULT_MAX_ROWS,
                         colMax  = DEFAULT_MAX_COLS,
                         row     = 0;
    bool                 hasT    = false,
                         hasR    = false,
                         hasInit = false;
    struct stat          sbuf;

    while ((ch = getopt(argc, argv, "R:c:d:a:t:r:ih")) != -1){
        switch (ch){
             case 'c':
                     colMax=atoi(optarg);
                     break;
             case 'R':
                     rowMax=atoi(optarg);
                     break;
             case 't':
                     hasT = true;
                     text = strdup(optarg);
                     if(text == NULL) nomem();
                     break;
             case 'd':
                     device = strdup(optarg);
                     if(text == NULL) nomem();
                     break;
             case 'a':
                     addr=strtol(optarg, NULL, BASE);
                     break;
             case 'r':
                     row=atoi(optarg);
                     hasR = true;
                     break;
             case 'i':
                     hasInit = true;
                     break;
             case 'h':
                     usage(argv[0]);
                     break;
             case '?':
             default:
                     usage(argv[0]);
        }
    }

    if(colMax >80 || colMax < 16 )                     usage(argv[0]);
    if(rowMax != 4  && rowMax != 2 && rowMax != 1 )    usage(argv[0]);
    if(!hasT || !hasR)                                 usage(argv[0]);
    if(stat(device, &sbuf) == -1)                      nodev();
    if(major(sbuf.st_dev != majorno))                  nodev();

    int            fdI2c = init(device, addr, hasInit);
	if (fdI2c < 0) nobus();

    writeLine(fdI2c, rowMax, colMax, text, row, true); 

    close(fdI2c);
    return 0;
}

void usage(char* pname){
    fprintf(stderr,"Usage:\n %s [-R row_max] [-c col_max] [-t text] [-r row_number] [-i] [ -d device ] [ -a hex_address ]\n", pname);
    fprintf(stderr,"\n* row_max can be 1 , 2 or 4, default 4\n");
    fprintf(stderr,"* col_max between 16 and 80, default 16\n");
    fprintf(stderr,"\nExample: \n");
    fprintf(stderr," sudo simple_lcd -R4 -c16 -r1 -t'hello world!' \n");
    fprintf(stderr,"\nwrites 'hello world!' on the first row of a 4x16 display. \n");
    exit(1);
}

void nomem(void){
    fprintf(stderr,"Can't allocate memory for arg parsing\n");
    exit(1);
}

void nodev(void){
    fprintf(stderr,"Wrong device path.\n");
    exit(1);
}

void nobus(void){
	fprintf(stderr, "Access error.");
	exit(1);
}