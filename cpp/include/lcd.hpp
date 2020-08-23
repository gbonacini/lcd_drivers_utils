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

#pragma once

#include <unistd.h>		
#include <fcntl.h>			
#include <sys/ioctl.h>

#include <string>
#include <array>
#include <iostream>

#define  anyexcept noexcept(false)

namespace lcd_hitachi_driver {

    class LcdDriver {
       public:
           LcdDriver(int addr=0x27, size_t rws=4, 
                     size_t cols=16, const std::string& dev="/dev/i2c-1")    anyexcept;
           ~LcdDriver(void)                                                  noexcept;
           void init(void)                                                   const anyexcept;
           void writeLine(std::string msg, unsigned int row, bool clean)     const anyexcept; 

       private:
           static const size_t OUTPUT_BUFF_SIZE { 6 };
           static const size_t ADDRESSES_SIZE   { 4 };
           static const size_t INIT_COLS        { 6 };
           static const size_t INIT_ROWS        { 10 };

           const int           I2C_SLAVE        { 0x0703 };
           const unsigned char LCD_BACKLIGHT    { 0x08 };
           const unsigned char EN               { 0x4 };

           size_t       rows,
                        columns;
           int          fdI2c,
                        address;
           std::string  device; 
           mutable std::array<unsigned char, OUTPUT_BUFF_SIZE> output;
           std::array<unsigned char, ADDRESSES_SIZE> addrs;
           std::array<std::array<unsigned char, INIT_COLS>, INIT_ROWS> initMatrix {{
               {{ 0x08,0x0c,0x08,0x38,0x3c,0x38 }},
               {{ 0x08,0x0c,0x08,0x38,0x3c,0x38 }},
               {{ 0x08,0x0c,0x08,0x38,0x3c,0x38 }},
               {{ 0x08,0x0c,0x08,0x28,0x2c,0x28 }},
               {{ 0x28,0x2c,0x28,0x88,0x8c,0x88 }},
               {{ 0x08,0x0c,0x08,0xc8,0xcc,0xc8 }},
               {{ 0x08,0x0c,0x08,0x18,0x1c,0x18 }},
               {{ 0x08,0x0c,0x08,0x68,0x6c,0x68 }},
               {{ 0x08,0x0c,0x08,0x18,0x1c,0x18 }},
               {{ 0x08,0x0c,0x08,0x28,0x2c,0x28 }}
            }};

            void hexCmd(unsigned char cmd, unsigned char mode)             const anyexcept;
    };
}