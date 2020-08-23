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

package main

import ( "lcd" 
		 "os" 
		 "flag" 
		 "fmt")

func main(){
	device  := flag.String("d", "/dev/i2c-1", "device path")
	address := flag.Int(   "a", 0x27,         "address")
	text    := flag.String("t", "",           "text to write")
	row     := flag.Int   ("r", 0,            "row number")
	maxRows := flag.Int   ("R", 4,            "max rows available")
	maxCols := flag.Int   ("c", 16,           "max columns available")
	init    := flag.Bool(  "i", false,        "send init sequence")

	flag.Parse()

	if *maxRows != 1 && *maxRows != 2 && *maxRows != 4 {
	   fmt.Fprintf(os.Stderr, "MaxRows can be 1, 2 or 4.\n");
	   os.Exit(1)
	}

	if *maxCols < 16 || *maxRows > 80 {
	   fmt.Fprintf(os.Stderr, "MaxCols must be value between 16 and 80.\n");
	   os.Exit(1)
	}

    if *row <=0 || *row > int(*maxRows) {
	   fmt.Fprintf(os.Stderr, "Row number must be between 1 and %d\n", *maxRows);
	   os.Exit(1)
	}

	lcd.Init(device, byte(*maxRows), byte(*maxCols), byte(*address), *init)

	lcd.WriteLine(text, uint(*row))

	lcd.Close()
	os.Exit(0)
}