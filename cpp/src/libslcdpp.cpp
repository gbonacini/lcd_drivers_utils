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

#include <lcd.hpp>

namespace lcd_hitachi_driver {

    using std::string;
    using std::cerr;
    using std::hex;

    LcdDriver::LcdDriver(int addr, size_t rws, size_t cols, const string& dev)  anyexcept
      : rows{rws}, columns{cols}, address{addr}, device{dev}
    {
        switch(rws){
            case 4:
               addrs = { 0x80,0xC0,static_cast<unsigned char>(0x80 + cols),static_cast<unsigned char>(0xC0 + cols) };
            break;
            case 2:
               addrs = { 0x80,0xC0 };
            break;
            case 1:
               addrs = { 0x80 };
            break;
            default:
		       cerr << "Max row number not supported.\n";
               throw;
        }
        fdI2c   = open(device.c_str(), O_RDWR);
	    if (fdI2c < 0) {
		    cerr << "Failed to open the i2c bus\n";
            throw;
        }

        if (ioctl(fdI2c, I2C_SLAVE, address) < 0) {
		    cerr << "Failed to acquire bus access and/or talk to slave.\n";
            throw;
        }
	}

    LcdDriver::~LcdDriver(void) noexcept {
        close(fdI2c);
    }

    void LcdDriver::writeLine(string msg, unsigned int row, bool clean) const anyexcept {
        try{
            const unsigned char MODE_RS = 0x1;

            hexCmd(addrs[row-1], 0);
            usleep(50000);
    
            if(msg.size() < columns && clean)
                 msg.append(static_cast<size_t>(columns - msg.size()), ' ');
    
            if(msg.size() > columns)
                 msg.erase(columns, msg.size()-columns);
    
            for(auto& el : msg) {
               hexCmd(el, MODE_RS);
               usleep(50000);
            }
        } catch (...) {
		        cerr << "Error: writeLine()\n";
                throw;
        }
    }

    void LcdDriver::hexCmd(unsigned char cmd, unsigned char mode) const anyexcept {
        unsigned char first  { static_cast<unsigned char>(mode | ( cmd & 0xF0 )) };
        unsigned char second { static_cast<unsigned char>(mode | ( (cmd << 4 ) & 0xF0 )) };

        output[0]=(first | LCD_BACKLIGHT );
        output[1]=(first | EN | LCD_BACKLIGHT);
        output[2]=((first & (~EN)) | LCD_BACKLIGHT );
    
        output[3]=(second | LCD_BACKLIGHT );
        output[4]=(second | EN | LCD_BACKLIGHT );
        output[5]=((second & (~EN)) | LCD_BACKLIGHT );
    
        for(auto& col : output){
	         if (write(fdI2c, &col, sizeof(unsigned char)) != sizeof(unsigned char)){ 
		         cerr << "Error: Failed to write cmd to the i2c bus.\n";
                 throw;
	         }
             usleep(50000);
        }
    }

    void LcdDriver::init(void) const anyexcept{
        for( auto& row : initMatrix){
           for( auto& elem : row){
	            if (write(fdI2c, &elem, sizeof(unsigned char)) != sizeof(unsigned char)){ 
		            cerr << "Error: Failed to write to the i2c bus.\n";
                    throw;
	            }
                usleep(50000);
            }
        }
    }

}