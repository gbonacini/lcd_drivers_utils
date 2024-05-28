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

#include <assert.h>

#define  MAX_BUFF    64

void hexCmd(int fd, unsigned char cmd, unsigned char mode){
    unsigned char       output[OUTBUFFSIZE];
    const unsigned char LCD_BACKLIGHT=0x08,
                        EN=0x4;

    unsigned char first=(mode | ( cmd & 0xF0 ) ),
                  second=(mode | ( (cmd << 4 ) & 0xF0 ) );

    output[0]=(first | LCD_BACKLIGHT );
    output[1]=(first | EN | LCD_BACKLIGHT);
    output[2]=((first & (~EN)) | LCD_BACKLIGHT );

    output[3]=(second | LCD_BACKLIGHT );
    output[4]=(second | EN | LCD_BACKLIGHT );
    output[5]=((second & (~EN)) | LCD_BACKLIGHT );

    for(int col=0; col<OUTBUFFSIZE; col++){
	     if (write(fd, output+col, sizeof(unsigned char)) != sizeof(unsigned char)) 
		     fprintf(stderr, "Error: Failed to write cmd to the i2c bus. %s\n", strerror(errno));

         usleep(50000);
    }
}

void writeLine(int fd, size_t maxRows, unsigned char maxCols, const char* const msg, unsigned int row, bool clean) {

    assert(maxCols + 1 <= MAX_BUFF);

    // add ctrl params
    unsigned char* ADDRS = malloc( maxRows * sizeof(unsigned char));

    if(ADDRS == NULL){
		     fprintf(stderr, "Error: ADDRS allocation\n");
             exit(1);
    }

    switch(maxRows){
        case 4:
                ADDRS[0] = 0x80;
                ADDRS[1] = 0xC0;
                ADDRS[2] = 0x80+maxCols;
                ADDRS[3] = 0xC0+maxCols;
        break;
        case 2:
                ADDRS[0] = 0x80;
                ADDRS[1] = 0xC0;
        break;
        case 1:
                ADDRS[0] = 0x80;
        break;
        default:
                // never here
                ADDRS[0]  = 0x00;
		        fprintf(stderr, "Error: not compatible rows number %ld\n", maxRows);
                exit(1);
    }

    char buff[MAX_BUFF];
    memset(buff, 0, sizeof(buff));

    if( strlen(msg) >= maxCols ){
        memcpy(buff, msg, maxCols);
    }else{
        if(clean)
           memset(buff, ' ', sizeof(buff) - 1);
        memcpy(buff, msg, strlen(msg));
    }
    const unsigned char MODE_RS=0x1;

    hexCmd(fd, ADDRS[row-1], 0);
    usleep(50000);
    for(int col=0; col<(int)strlen(buff); col++)
       hexCmd(fd, buff[col], MODE_RS);

    free(ADDRS);
}

int init(const char *device, int addr, bool reset){
    const unsigned char initSequence[INIT_ROWS][INIT_COLS] = {
        { 0x08,0x0c,0x08,0x38,0x3c,0x38 },
        { 0x08,0x0c,0x08,0x38,0x3c,0x38 },
        { 0x08,0x0c,0x08,0x38,0x3c,0x38 },
        { 0x08,0x0c,0x08,0x28,0x2c,0x28 },
        { 0x28,0x2c,0x28,0x88,0x8c,0x88 },
        { 0x08,0x0c,0x08,0xc8,0xcc,0xc8 },
        { 0x08,0x0c,0x08,0x18,0x1c,0x18 },
        { 0x08,0x0c,0x08,0x68,0x6c,0x68 },
        { 0x08,0x0c,0x08,0x18,0x1c,0x18 },
        { 0x08,0x0c,0x08,0x28,0x2c,0x28 }
    };

    // Open the I2C Bus 
    int fdI2c = open(device, O_RDWR);
	if (fdI2c < 0) {
		fprintf(stderr, "Failed to open the i2c bus: %s\n", strerror(errno));
		return -1;
	}
	
	if (ioctl(fdI2c, I2C_SLAVE, addr) < 0) {
		fprintf(stderr, "Failed to acquire bus access and/or talk to slave. %s\n", strerror(errno));
		return -1;
	}
    if(reset){
        for(int row=0; row<INIT_ROWS; row++){
            for(int col=0; col<INIT_COLS; col++){
	            if (write(fdI2c, &initSequence[row][col], sizeof(unsigned char)) != sizeof(unsigned char)) 
		            fprintf(stderr, "Error: Failed to write to the i2c bus: %s\n", strerror(errno));
	            
                usleep(50000);
            }
        }
    }

    return fdI2c;
}
 
