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

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sysmacros.h>

#include <lcd.hpp>
#include <parseCmdLine.hpp>

using lcd_hitachi_driver::LcdDriver;
using parcmdline::ParseCmdLine;
using std::cerr;
using std::endl;
using std::string;
using std::stoi;

void usage(char* pname);
void nodev(void);

int main(int argc, char** argv){
    bool                 init    { false };
    string               dev     { "/dev/i2c-1" },
                         text    { "" };
    const unsigned int   majorno { 89 };
	int                  addr    { 0x27 },
                         row     { 1 };
    size_t               maxRows { 4 },
                         maxCols { 16 };
    struct stat          sbuf;

    constexpr char    flags[]    { "R:c:d:a:t:r:ih" };
    ParseCmdLine pcl(argc, argv, flags);
    if(pcl.getErrorState()){
        string exitMsg{string("Invalid  parameter or value").append(pcl.getErrorMsg())};
        cerr << exitMsg << "\n";
        usage(argv[0]);
    }

    if(pcl.isSet('h'))
        usage(argv[0]);

    if(!pcl.isSet('t') || !pcl.isSet('r')) 
        usage(argv[0]);
    text = pcl.getValue('t');
    row = stoi(pcl.getValue('r'));

    if(pcl.isSet('d') ) 
        dev = pcl.getValue('d');

    if(pcl.isSet('R') ) 
        maxRows = stoi(pcl.getValue('R'));
    if(maxRows != 1 && maxRows != 2 && maxRows != 4)
        usage(argv[0]);

    if(pcl.isSet('c') ) 
        maxCols = stoi(pcl.getValue('c'));
    if(maxCols < 16 || maxCols >80)
        usage(argv[0]);

    if(stat(dev.c_str(), &sbuf) == -1)    
        nodev();

    if(major(sbuf.st_dev != majorno)) 
        nodev();

    if(pcl.isSet('a') ) 
        addr = stoi(pcl.getValue('a'));

    if(pcl.isSet('i') ) 
        init = true;

    try{
        LcdDriver lcdDriver(addr, maxRows, maxCols, dev);
        if(init)
            lcdDriver.init();
        lcdDriver.writeLine(text, row, true);
    } catch (...) {
        cerr << "Program exits with errors\n";
        exit(1);
    }

    return 0;
}

void usage(char* pname){
    cerr << "Usage:\n" << pname << " [-R rowmax] [-c colmax] [ -t text ] [ -r row_number ] [-i] [ -d device ] [ -a hex_address ]\n"
         << "\n* row_max can be 1 , 2 or 4, default 4\n"
         << "* col_max between 16 and 80, default 16\n"
         << "\nExample: \n"
         << " sudo simple_lcdpp -R4 -c16 -r1 -t'hello world!' \n"
         << "\nwrites 'hello world!' on the first row of a 4x16 display. \n";
    exit(1);
}

void nodev(void){
    cerr << "Wrong device path.\n";
    exit(1);
}