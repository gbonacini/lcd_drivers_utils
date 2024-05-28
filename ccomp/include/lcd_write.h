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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>		
#include <fcntl.h>			
#include <sys/ioctl.h>	
#include <errno.h>

#define I2C_SLAVE    0x0703

#define OUTBUFFSIZE  6
#define INIT_ROWS    10
#define INIT_COLS    6

#define DEFAULT_MAX_COLS     16
#define DEFAULT_MAX_ROWS     4
#define DEFAULT_DEV_ADDR     0x27
#define DEFAULT_DEV_PATH     "/dev/i2c-1"
#define MAJOR_NUMBER         89


void hexCmd(int fd, unsigned char cmd, unsigned char mode);
void writeLine(int fd, size_t maxRows, unsigned char maxCols, const char* const msg, unsigned int row, bool clean);
int init(const char *device, int addr, bool reset);
