# -----------------------------------------------------------------
# simple_lcd - command line tools to print message using Hitachi  
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


import sys
assert sys.version_info >= (3, 0, 0), "Python 3 is required."
import os

import fcntl
from   time import sleep

class i2c:
    def __init__(self, dev="1", addr=0x27):
        self.I2C_SLAVE = 0x0703
        self.device    = "/dev/i2c-{}".format(dev)
        self.address   = addr
        self.sfile     = open(self.device, 'r+b', buffering=0)
        self.fd        = self.sfile.fileno()
        fcntl.ioctl(self.fd, self.I2C_SLAVE, self.address)

    def write(self, cbyte):
         abyte = bytearray([cbyte])
         self.sfile.write(abyte)
         sleep(.005)

class lcd:
   #initializes objects and lcd
   def __init__(self, nrows=4, ncols=16, prt=1, addr=0x27, init=True):

      if nrows > 4 or nrows < 1 or ncols <1 or (nrows * ncols) > 80 or (nrows * ncols) < 16:
          raise("Invalid LCD size specified")

      self.commands = { 'LCD_CLEAR'          : 0x01,   'LCD_RETURN_HOME'    : 0x02,  'LCD_ENTRY_MODE'   : 0x04,  
                        'LCD_DISPLAY_CTRL'   : 0x08,   'LCD_SHIFT_CURSOR'   : 0x10,  'LCD_FUNCTION_SET' : 0x20, 
                        'LCD_SET_CGRAM_ADDR' : 0x40,   'LCD_SET_DDRAM_ADDR' : 0x80 
                      }

      self.flags    = {
                        # Entry mode
                        'LCD_ENTRY_RIGHT'      : 0x00, 'LCD_ENTRY_SHIFT_DECR' : 0x00,
                        'LCD_ENTRY_SHIFT_INC'  : 0x01, 'LCD_ENTRY_LEFT'       : 0x02,   

                        # Movement
                        'LCD_MOVE_LEFT'        : 0x00,  'LCD_CURSOR_MOVE'     : 0x00,   
                        'LCD_MOVE_RIGHT'       : 0x04,  'LCD_DISPLAY_MOVE'    : 0x08, 

                        # Mode Set
                        'LCD_5x8_DOTS'         : 0x00,  'LCD_4BIT_MODE'       : 0x00,   'LCD_1LINE'       : 0x00,       
                        'LCD_5x10_DOTS'        : 0x04,  'LCD_2LINE'           : 0x08,   'LCD_8BIT_MODE'   : 0x10,       

                        # Switches
                        'LCD_BACKLIGHT_OFF'    : 0x00,  'LCD_DISPLAY_OFF'     : 0x00,   'LCD_CURSOR_OFF'  : 0x00,   
                        'LCD_BLINK_OFF'        : 0x00,  'LCD_BLINK_ON'        : 0x01,   'LCD_CURSOR_ON'   : 0x02,   'LCD_DISPLAY_ON'  : 0x04,  
                        'LCD_BACKLIGHT_ON'     : 0x08  
                      }

      self.registers = {
                        'EN' : 0b00000100, # Enable bit
                        'RW' : 0b00000010, # Read/Write bit
                        'RS' : 0b00000001, # Register Select bit  
                       }

      self.dump        = False
      self.dumpFile    = "./lcd.dmp"
      self.dumpFd      = None
      self.address     = addr
      self.port        = int(prt)
      self.rows        = nrows
      self.columns     = ncols
      self.lineEntries = []

      if self.rows == 1:
          self.lineEntries.append( 0x80 )
      if self.rows == 2:
          self.lineEntries.append( 0x80 )
          self.lineEntries.append( 0xC0 )
      if self.rows == 4:
          self.lineEntries.append( 0x80 )
          self.lineEntries.append( 0xC0 )
          self.lineEntries.append( 0x80 + self.columns)
          self.lineEntries.append( 0xC0 + self.columns)

      self.bus = i2c()

   def init(self):
       self.writeDumpLabel("Init:")
       self.lcd_write_array([0x03, 0x03, 0x03, 0x02])

       self.lcd_write(self.commands['LCD_FUNCTION_SET'] | self.flags['LCD_2LINE'] | self.flags['LCD_5x8_DOTS'] | self.flags['LCD_4BIT_MODE'])
       self.lcd_write(self.commands['LCD_DISPLAY_CTRL'] | self.flags['LCD_DISPLAY_ON'])
       self.lcd_write(self.commands['LCD_CLEAR'])
       self.lcd_write(self.commands['LCD_ENTRY_MODE'] | self.flags['LCD_ENTRY_LEFT'])
       self.writeDumpLabel("End Init.")

   def setDump(self, onOff=False):
       if onOff and not self.dump:
            try:
                self.dumpFd = os.open(self.dumpFile, os.O_WRONLY | os.O_CREAT)
                self.dump   = onOff
            except:
                print("Can't oped dump file.", file=sys.stderr)
       else:
           try:
               if self.dumpFd:
                   os.close(self.dumpFd)
           except:
               pass

           self.dump   = onOff
           self.dumpFd = None

   def writeDumpLabel(self, label):
       if self.dump:
           os.write(self.dumpFd, bytes("\n#{}\n".format(label), 'ascii'))

   def writeDump(self, data):
           os.write(self.dumpFd, bytes("0x{:02x} ".format(data), 'ascii'))

   def lcd_write(self, cmd, mode=0):
      fourBitsPair = [(mode | (cmd & 0xF0)), mode | ((cmd << 4) & 0xF0)]
      self.writeDumpLabel("Begin Write:")
      for fourBits in fourBitsPair :
          msg = fourBits | self.flags['LCD_BACKLIGHT_ON']
          self.writeDumpLabel("Header:")
          self.write_cmd(msg)

          self.writeDumpLabel("Footer:")
          first  = fourBits | self.registers['EN'] | self.flags['LCD_BACKLIGHT_ON']
          second = (fourBits & ~ self.registers['EN']) | self.flags['LCD_BACKLIGHT_ON']

          self.write_cmd(first)
          self.write_cmd(second)
          
      self.writeDumpLabel("End Write.")

   def lcd_write_array(self, vect, mode=0) :
       for cmd in vect:
           self.lcd_write(cmd, mode)
      
   #turn on/off the lcd backlight
   def lcd_backlight(self, state):
      if state in ("on","On","ON"):
         self.write_cmd(self.flags['LCD_BACKLIGHT_ON'])
      elif state in ("off","Off","OFF"):
         self.write_cmd(self.flags['LCD_BACKLIGHT_OFF'])
      else:
         print("Unknown State!")

   # put string function
   def lcd_display_char(self, char, row, col):
      if (row > self.rows)  or (row < 1)  or col > self.columns or col < 1:
           raise('lcd_display_char: invalid coordinates')
      self.writeDumpLabel("Position:")
      self.lcd_write(int(self.lineEntries[row-1])+col)
      self.writeDumpLabel("Data (RS):")
      self.lcd_write(char, self.registers['RS'])

   # put string function
   def lcd_display_string(self, string, line):
      self.writeDumpLabel("Set Position:")
      self.lcd_write(self.lineEntries[line - 1])

      for char in string:
         self.writeDumpLabel("Print char ({}):".format(char))
         self.lcd_write(ord(char), self.registers['RS'])
         self.writeDumpLabel("End Print.")

   # clear lcd and set to home
   def lcd_clear(self):
      self.lcd_write(self.commands['LCD_CLEAR'])
      self.lcd_write(self.commands['LCD_RETURN_HOME'])

   # Write a single command
   def write_cmd(self, cmd):
      if not self.dump:
          self.bus.write(cmd)
      else:
         self.writeDump(cmd)
