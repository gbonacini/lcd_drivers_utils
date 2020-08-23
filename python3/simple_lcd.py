#!/usr/bin/env python3

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
import os
import traceback
import traceback as trbck
import configparser as cfg

assert sys.version_info >= (3, 0, 0), "Python 3 is required."

import getopt
from time import sleep

import lcd_writer
import time

class LcdPrintHelper:
    @staticmethod
    def printAllRow(lcd, rowsData, max):
        assert(max == 1 or max == 2 or max == 4)

        lcd.writeDumpLabel("Rows:")
        for idx in range(1, max+1):
            lcd.writeDumpLabel("RowData ({}):".format(rowsData[idx]))
            lcd.lcd_display_string(rowsData[idx],  idx)
            lcd.writeDumpLabel("End RowData.")
        lcd.writeDumpLabel("End Rows")

    @staticmethod
    def page(lcd, buff, rows, columns, waitTime):
            assert(rows == 1 or rows == 2 or rows == 4)
    
            start=0
            stop=rows*columns
            rowsData = {}
     
            if rows == 1: 
               rowsData = { 1 : ''}
            elif rows == 2: 
               rowsData = { 1 : '', 2 : ''}
            elif rows == 4: 
               rowsData = { 1 : '', 2 : '', 3 : '', 4 : '' }

            while True:
                 thisPage=buff[start:stop]
                
                 if len(thisPage) == 0:
                     break
                 else:
                     time.sleep(waitTime)
    
                 if rows == 1: 
                     rowsData[1] = thisPage[0:columns]
                 elif rows == 2: 
                     rowsData[1] = thisPage[0:columns]
                     rowsData[2] = thisPage[columns:columns*2]
                 elif rows == 4: 
                     rowsData[1] = thisPage[0:columns]
                     rowsData[2] = thisPage[columns:columns*2]
                     rowsData[3] = thisPage[columns*2:columns*3]
                     rowsData[4] = thisPage[columns*3:columns*4]
    
                 LcdPrintHelper.printAllRow(lcd, rowsData, rows)
    
                 start += rows*columns
                 stop  += rows*columns
    
            lcd.lcd_clear()

class CmdLineParams:
    def __init__(self, rows, cols, args):
        self.rows = rows                     
        self.columns = cols
        self.arguments = args
        self.flags = ''
        self.buff = ''
        self.longFlags = ['split=', 'page=', 'dump=' 'help=' ]
        self.rowsData = {}
        self.remainder = None
        self.commonFlags = 's:p:t:Dh'

        self.status = {'split' : False, 'page' : False }

        if      self.rows == 1:
            self.flags = '1:{}'.format(self.commonFlags)
            self.longFlags = ['first='] + self.longFlags
            self.rowsData = { 1 : ''}
            self.status.update({ 'first' : False })
        
            self.options, self.remainder = getopt.getopt(self.arguments, self.flags, self.longFlags)

            for opt, arg in self.options:
                if opt in ('-1', '--first'):
                    self.status['first'] = True
                    self.rowsData[1] = arg[0:self.columns]
                elif opt in ('-2', '--second'):
                    self.status['second'] = True
                    self.rowsData[2] = arg[0:self.columns]
                elif opt in ('-3', '--third'):
                    self.status['third'] = True
                    self.rowsData[3] = arg[0:self.columns]
                elif opt in ('-4', '--fourth'):
                    self.status['fourth'] = True
                    self.rowsData[4] = arg[0:self.columns]
                elif opt in ('-s', '--split'):
                    self.status['split'] = True
                    self.rowsData[1] = arg[0:self.columns]
                elif opt in ('-p', '--page'):
                    self.status['page'] = True
                    self.buff=arg
                elif opt in ('-D', '--dump'):
                    self.status['dump'] = True
                elif opt in ('-h', '--help'):
                    self.helper(sys.argv[0])
        elif self.rows == 2:
            self.flags = '1:2:{}'.format(self.commonFlags)
            self.longFlags = ['first=', 'second='] + self.longFlags
            self.rowsData = { 1 : '', 2 : ''}
            self.status.update({ 'first' : False, 'second' : False })

            self.options, self.remainder = getopt.getopt(self.arguments, self.flags, self.longFlags)

            for opt, arg in self.options:
                if opt in ('-1', '--first'):
                    self.status['first'] = True
                    self.rowsData[1] = arg[0:self.columns]
                elif opt in ('-2', '--second'):
                    self.status['second'] = True
                    self.rowsData[2] = arg[0:self.columns]
                elif opt in ('-s', '--split'):
                    self.status['split'] = True
                    self.rowsData[1] = arg[0:self.columns]
                    self.rowsData[2] = arg[self.columns:self.columns*2]
                elif opt in ('-p', '--page'):
                    self.status['page'] = True
                    self.buff=arg
                elif opt in ('-D', '--dump'):
                    self.status['dump'] = True
                elif opt in ('-h', '--help'):
                    self.helper(sys.argv[0])
        elif self.rows == 4:
            self.flags = '1:2:3:4:{}'.format(self.commonFlags)
            self.longFlags = ['first=', 'second=', 'third=', 'fourth='] + self.longFlags
            self.rowsData = { 1 : '', 2 : '', 3 : '', 4 : '' }
            self.status.update( { 'first' : False, 'second' : False, 'third' : False, 'fourth' : False })

            self.options, self.remainder = getopt.getopt(self.arguments, self.flags, self.longFlags)

            for opt, arg in self.options:
                if opt in ('-1', '--first'):
                    self.status['first'] = True
                    self.rowsData[1] = arg[0:self.columns]
                elif opt in ('-2', '--second'):
                    self.status['second'] = True
                    self.rowsData[2] = arg[0:self.columns]
                elif opt in ('-3', '--third'):
                    self.status['third'] = True
                    self.rowsData[3] = arg[0:self.columns]
                elif opt in ('-4', '--fourth'):
                    self.status['fourth'] = True
                    self.rowsData[4] = arg[0:self.columns]
                elif opt in ('-s', '--split'):
                    self.status['split'] = True
                    self.rowsData[1] = arg[0:self.columns]
                    self.rowsData[2] = arg[self.columns:self.columns*2]
                    self.rowsData[3] = arg[self.columns*2:self.columns*3]
                    self.rowsData[4] = arg[self.columns*3:self.columns*4]
                elif opt in ('-p', '--page'):
                    self.status['page'] = True
                    self.buff=arg
                elif opt in ('-D', '--dump'):
                    self.status['dump'] = True
                elif opt in ('-h', '--help'):
                    self.helper(sys.argv[0])
        else:
            raise Exception("Invalid Row number.")

        self.checkParams()


    def checkParams(self):
        if len(self.arguments) == 0:
            self.helper(sys.argv[0], ' -> Error: no parameter')

        if self.rows == 1:
            if ( self.status['first'] and
                   ( self.status['split'] or self.status['page'] )):
                       self.helper(sys.argv[0], ' -> Error: [-1] and [-spt] are mutually exclusive')
        if self.rows == 2:
            if (( self.status['first'] or self.status['second'] )
                   and
                   ( self.status['split'] or self.status['page'] )):
                       self.helper(sys.argv[0], ' -> Error: [-12] and [-spt] are mutually exclusive')
        if self.rows == 4:
            if (( self.status['first'] or self.status['second'] or self.status['third'] or self.status['fourth'])
                   and
                   ( self.status['split'] or self.status['page'] )):
                       self.helper(sys.argv[0], ' -> Error: [-1234] and [-spt] are mutually exclusive')

        if self.status['split'] and self.status['page']:
            self.helper(sys.argv[0], ' -> Error: [-s] and [-p] are mutually exclusive')


    def helper(self, cmd, msg=''):
        print('{}\n'.format(msg), file=sys.stderr)
        if self.rows == 1:
            print('{} [-1 message] | [-s message] | [-p message] | [-h] '.format(cmd), file=sys.stderr)
            print('-1 or --first \n\tInsert "message" in the first LCD row.\n', file=sys.stderr)
        elif self.rows == 2:
            print('{} [-12 message] | [-s message] | [-p message] | [-h] '.format(cmd), file=sys.stderr)
            print('-1 or --first \n\tInsert "message" in the first LCD row.\n', file=sys.stderr)
            print('-2 or --second \n\tInsert "message" in the second LCD row.\n', file=sys.stderr)
        elif self.rows == 4:
            print('{} [-1234 message] | [-s message] | [-p message] | [-h] '.format(cmd), file=sys.stderr)
            print('-1 or --first \n\tInsert "message" in the first LCD row.\n', file=sys.stderr)
            print('-2 or --second \n\tInsert "message" in the second LCD row.\n', file=sys.stderr)
            print('-3 or --third \n\tInsert "message" in the third LCD row.\n', file=sys.stderr)
            print('-4 or --fourth \n\tInsert "message" in the fourth LCD row.\n', file=sys.stderr)
        else:
            raise Exception("helper: invalid line number.")

        print('-D or --dump   \n\tDump all bytes sento to the attacched LCD \n', file=sys.stderr)
        print('-s or --split  \n\tSplit the initial part of "message" using the four rows', file=sys.stderr)
        print('\tThe part of "message" that exceeds  the available LCD space', file=sys.stderr)
        print('\twill be ignored.\n', file=sys.stderr)
        print('-p or --page   \n\tInsert all "message" characters, one page at time, using', file=sys.stderr)
        print('\tall LCD\'s available space. So, at the end, the entire "message" will be printed.\n', file=sys.stderr)
        print('-h or --help \n\tPrint this help synopsis.\n', file=sys.stderr)
        os._exit(1)

    def getRowsData(self):
        return self.rowsData 

    def getBuff(self):
        return self.buff 
    
    def isPaged(self):
        return self.status.get('page', False)

    def isDumped(self):
        return self.status.get('dump', False)

if __name__ == "__main__":
    try:
        config = cfg.RawConfigParser(allow_no_value=False)

        with open('simpleLcd.conf', 'r') as cFile:
            config.read_string(cFile.read())

        #stdWait   =  int(config['time']['wait'])
        pageTimer =  int(config['time']['pager'])
        columns   =  int(config['lcd']['columns'])
        rows      =  int(config['lcd']['rows'])
        address   =  int(config['i2c']['address'], 16)
        port      =  int(config['i2c']['port'])

        cmdLineParams = CmdLineParams(rows, columns, sys.argv[1:])
    
        lcd = lcd_writer.lcd(rows, columns, port, address )
    
        if cmdLineParams.isDumped(): 
            lcd.setDump(True)

        lcd.init()
        lcd.lcd_clear()

        if cmdLineParams.isPaged(): 
            LcdPrintHelper.page(lcd, cmdLineParams.getBuff(), rows, columns, pageTimer)
    
        LcdPrintHelper.printAllRow(lcd, cmdLineParams.getRowsData(), rows)
    
        if cmdLineParams.isDumped(): 
            lcd.setDump(False)
    
    except:
        errFile = 'simpleLcd.log'
        with open(errFile, 'w') as errLog:
            print('{}'.format(trbck.format_exc()), file=errLog)
            print('Error: see log file: {}'.format(errFile), file=sys.stderr)

        os._exit(1)
  
    os._exit(0)
