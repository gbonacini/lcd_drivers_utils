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

extern crate libc;
extern crate errno;

use errno::errno;

use std::{ffi::CString, 
          process, 
          thread, 
          time,
          str,
          vec,
          mem::size_of,
          default::Default};

extern {
    fn ioctl(fd: libc::c_int, request: libc::c_ulong, ...)  -> libc::c_int;
    fn open(path: *const libc::c_char, flags: libc::c_int)  -> libc::c_int;
    fn close(fd: libc::c_int)  -> libc::c_int;
    fn write(fd: libc::c_int, buf: *const libc::c_char, count: libc::c_ulong)  -> libc::c_int;
}

pub struct Lcd {
    fd: libc::c_int,
}

impl Default for Lcd {
    fn default() -> Lcd {
        Lcd { fd: -1 }
    }
}

impl Lcd {
    // Private:

    fn exit_on_error(msg: &str, display_errno: bool){
        println!("Error: {}\n", msg);
        let err_msg = errno();
        if display_errno {
           println!("Errno description: {} - {}\n", err_msg.0, err_msg);
        }
        process::exit(1);
    }

    fn hex_cmd(&self, cmd: u8, mode: u8){
        const OUTBUFFSIZE: usize =  6;
        let mut output = vec![0; OUTBUFFSIZE];
        const LCD_BACKLIGHT : u8 = 0x08;
        const EN : u8 = 0x4;
    
        let first : u8 =  mode | ( cmd & 0xF0 );
        let second : u8 = mode | ( (cmd << 4 ) & 0xF0 );
        
        output[0]=first | LCD_BACKLIGHT;
        output[1]=first | EN | LCD_BACKLIGHT;
        output[2]=(first & (!EN)) | LCD_BACKLIGHT;
    
        output[3]=second | LCD_BACKLIGHT;
        output[4]=second | EN | LCD_BACKLIGHT;
        output[5]=(second & (!EN)) | LCD_BACKLIGHT;
    
        for col in 0..OUTBUFFSIZE {
            let ret : libc::c_int = unsafe{ write(self.fd, &output[col], size_of::<libc::c_uchar>() as u64)};
	        if ret != size_of::<libc::c_uchar>() as i32 { 
                Self::exit_on_error("Failed to write cmd to the i2c bus.", true);
	        }
            thread::sleep(time::Duration::from_millis(50));
        }
    }

    // Public:

    pub fn init(&mut self, device: &str, address: i32, init_flagg: bool){
    
        const INIT_COLS : usize =  6;
        const INIT_ROWS : usize =  10;
    
        let init_sequence : [[libc::c_uchar; INIT_COLS]; INIT_ROWS]
        = [
            [ 0x08,0x0c,0x08,0x38,0x3c,0x38 ],
            [ 0x08,0x0c,0x08,0x38,0x3c,0x38 ],
            [ 0x08,0x0c,0x08,0x38,0x3c,0x38 ],
            [ 0x08,0x0c,0x08,0x28,0x2c,0x28 ],
            [ 0x28,0x2c,0x28,0x88,0x8c,0x88 ],
            [ 0x08,0x0c,0x08,0xc8,0xcc,0xc8 ],
            [ 0x08,0x0c,0x08,0x18,0x1c,0x18 ],
            [ 0x08,0x0c,0x08,0x68,0x6c,0x68 ],
            [ 0x08,0x0c,0x08,0x18,0x1c,0x18 ],
            [ 0x08,0x0c,0x08,0x28,0x2c,0x28 ]
        ];

        // Connection Params
        let filename = CString::new(device).unwrap();
        let addr : libc::c_int = address;
        const O_RDWR	: libc::c_int = 0x2;

	    // Open the I2C Bus 
        self.fd = unsafe { open(filename.as_ptr(), O_RDWR) };
        if self.fd < 0 {
            Self::exit_on_error("Failed to open the i2c bus", true);
        }
       
        const I2C_SLAVE : libc::c_ulong = 0x0703;
        let ret : libc::c_int = unsafe { ioctl(self.fd, I2C_SLAVE, addr) };
	    if ret < 0 {
            Self::exit_on_error("Failed to acquire bus access and/or talk to slave.", true);
	    }
        if init_flagg {
            for row in 0..INIT_ROWS {
                for col in 0..INIT_COLS {
                    let ret : libc::c_int = unsafe{ write(self.fd, &init_sequence[row][col], size_of::<libc::c_uchar>() as u64 ) };
	                if  ret != size_of::<libc::c_uchar>() as i32 { 
                        Self::exit_on_error("Failed to write to the i2c bus.", true);
                    }
                    thread::sleep(time::Duration::from_millis(50));
                }
            }
        }
    }

    pub fn close(&self){
        unsafe { close(self.fd) };
    }

    pub fn write_line(&self, msg: &str, maxrows: usize, maxcols: usize, row: usize) {
        let mut addrs: Vec<libc::c_uchar> = Vec::new();
        if maxrows == 4{
            addrs.push(0x80);
            addrs.push(0xC0);
            addrs.push(0x80+maxcols as u8);
            addrs.push(0xC0+maxcols as u8);
        } else if maxrows == 2 {
            addrs.push(0x80);
            addrs.push(0xC0);
        } else if maxrows == 1 {
            addrs.push(0x80);
        }
    
        const MODE_RS : u8 = 0x1;
    
        self.hex_cmd(addrs[row-1], 0);
        thread::sleep(time::Duration::from_millis(50));
    
        let limit: usize = if msg.len() <= maxcols { msg.len() } else { maxcols };
        for el in 0..limit {
           self.hex_cmd(msg.as_bytes()[el], MODE_RS);
           thread::sleep(time::Duration::from_millis(50));
        }
        if limit < maxcols {
           for _filler in limit..maxcols {
               self.hex_cmd(0x20 , MODE_RS);
               thread::sleep(time::Duration::from_millis(50));
           }
        }
    }

} // End Lcd Impl
