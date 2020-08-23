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

package lcd

import ( "fmt" 
		 "syscall" 
		 "time" 
		 "os" )

// Private

const cOUTPUT_BUFF_SIZE uint8 = 6 
const cINIT_COLS        uint8 = 6 
const cINIT_ROWS        uint8 = 10

const cI2C_SLAVE        int  = 0x0703 
const cLCD_BACKLIGHT    byte = 0x08 
const cEN               byte = 0x04 

var   rows, columns, address           byte
var   fdI2c                            int
var   output[cOUTPUT_BUFF_SIZE]        byte
var   line_addrs                       []byte

var initMatrix  = [cINIT_ROWS] [cINIT_COLS] byte  {
	{ 0x08,0x0c,0x08,0x38,0x3c,0x38 },
	{ 0x08,0x0c,0x08,0x38,0x3c,0x38 },
	{ 0x08,0x0c,0x08,0x38,0x3c,0x38 },
	{ 0x08,0x0c,0x08,0x28,0x2c,0x28 },
	{ 0x28,0x2c,0x28,0x88,0x8c,0x88 },
	{ 0x08,0x0c,0x08,0xc8,0xcc,0xc8 },
	{ 0x08,0x0c,0x08,0x18,0x1c,0x18 },
	{ 0x08,0x0c,0x08,0x68,0x6c,0x68 },
	{ 0x08,0x0c,0x08,0x18,0x1c,0x18 },
	{ 0x08,0x0c,0x08,0x28,0x2c,0x28 } }

func hexCmd(cmd byte, mode byte){
	var first   byte   =   mode | ( cmd & 0xF0 )
	var second  byte   =   mode | ( (cmd << 4 ) & 0xF0 )

	output[0]=first | cLCD_BACKLIGHT 
	output[1]=first | cEN | cLCD_BACKLIGHT
	output[2]=(first & (^cEN)) | cLCD_BACKLIGHT 

	output[3]=second | cLCD_BACKLIGHT 
	output[4]=second | cEN | cLCD_BACKLIGHT 
	output[5]=(second & (^cEN)) | cLCD_BACKLIGHT 

	for idx := 0; idx < len(output); idx++ {
		 ret, err := syscall.Write(fdI2c, output[idx:idx+1]) 
		 if ret != 1 || err != nil { 
			 fmt.Fprintf(os.Stderr, "Error: Failed to write cmd to the i2c bus: %v", err)
		     os.Exit(1)
		 }
		 time.Sleep(50 * time.Millisecond)
	}
}

// Public

func Init(device *string, rws byte, cols byte, addr byte, init bool) {
	rows    =  rws
	columns =  cols
	address =  addr

	if rws == 4 {
	    line_addrs = append(line_addrs,  0x80)
	    line_addrs = append(line_addrs,  0xC0)
		line_addrs = append(line_addrs,  0x80 + cols)
	    line_addrs  = append(line_addrs,  0xC0 + cols)
	} else if rws == 2 {
	    line_addrs = append(line_addrs,  0x80)
	    line_addrs = append(line_addrs,  0xC0)
	} else if rws == 1 {
	    line_addrs = append(line_addrs,  0x80)
	}

	var err error
	fdI2c, err = syscall.Open(*device, syscall.O_RDWR, 0)
	if err != nil {
		 fmt.Fprintf(os.Stderr, "Error opening special device: %v", err)
		 os.Exit(1)
	}
	
    _, _, ierr := syscall.Syscall(syscall.SYS_IOCTL, uintptr(fdI2c), uintptr(cI2C_SLAVE), uintptr(address))
	if ierr != 0 {
		 fmt.Fprintf(os.Stderr, "Error on Ioctl: %v", ierr)
		 os.Exit(1)
	}

	if init {
	    for _, line := range initMatrix {
	       for idx := 0; idx < len(line); idx++ {
		       ret, err := syscall.Write(fdI2c, line[idx:idx+1]) 
		       if ret != 1 || err != nil { 
			       fmt.Fprintf(os.Stderr, "Error: Failed to write cmd to the i2c bus: %v", err)
		           os.Exit(1)
		       }
		       time.Sleep(50 * time.Millisecond)
		    }
		}
	}

}

func WriteLine(msg *string, row uint) {
	const MODE_RS byte = 0x1

	var bmsg []byte
	if(len(*msg) > int(columns)){
	   bmsg = []byte((*msg)[:columns])
	}else{
	   bmsg = []byte(*msg)
	}  

	hexCmd(line_addrs[row-1], 0)
	time.Sleep(50 * time.Millisecond)

	for _, el := range bmsg {
	   hexCmd(el, MODE_RS)
	   time.Sleep(50 * time.Millisecond)
	}
	if len(*msg) < int(columns) {
	   for filler := len(*msg); filler < int(columns); filler++ {
		   hexCmd(0x20 , MODE_RS)
	       time.Sleep(50 * time.Millisecond)
	   }
	}
}

func Close() {
	cerr := syscall.Close(fdI2c)
	if cerr != nil {
		 fmt.Fprintf(os.Stderr, "Error closing fd: %v", cerr)
	}
}
