#!/usr/bin/env bash 

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

set -euf -o pipefail

#DEFAULTS
I2C_BUS=1       #I2C settings:
LCD_ADDR=0x27   # "   "
COLS=0x10
ROWS=4
MSG_1=""
MSG_2=""
MSG_3=""
MSG_4=""

function help { 
    readonly HELPMSG="Usage:\n\n $0  \n
    \t[-h] \n
    \t[-a <lcd_address>] \n
    \t[-b <i2c_bus>] \n
    \t[-c <cols_number>] \n
    \t[-r <rows_number>] \n
    \t[-1 <first_line_text>] \n
    \t[-2 <second_line_text>] \n
    \t[-3 <third_line_text>] \n
    \t[-4 <fourth_line_text>] \n
    \n
    Description:\n\n
    -h print this help message\n
    -a set the lcd address (i.e. 0x27)\n
    -b set I2C bus (i.e. 1)\n
    -c set the number ow available columns (i.e 16)\n
       Max -s 80.\n
    -r set the number of available rows (i.e 4)\n
       Must be <= 4.\n
    -1 to -4 set the string msg for a specific line.\n"
    
    echo -e ${HELPMSG}
    exit 1 
}

function parseArgs {
    while getopts "ha:b:c:r:1:2:3:4:" par; do
        case "${par}" in
            a)
                LCD_ADDR=${OPTARG}
                ;;
            b)
                I2C_BUS=${OPTARG}
                ;;
            c)
                ((${OPTARG} < 16 || ${OPTARG} > 80)) && help
                COLS=$(printf "0x%x" ${OPTARG})
                ;;
            r)
                ROWS=${OPTARG}
                ((${ROWS} < 1 || ${ROWS} > 4)) && help
                ;;
            1)
                MSG_1=${OPTARG}
                ;;
            2)
                MSG_2=${OPTARG}
                ;;
            3)
                MSG_3=${OPTARG}
                ;;
            4)
                MSG_4=${OPTARG}
                ;;
            h)
                help
                ;;
            *)
                help
                ;;
        esac
    done

    shift $((OPTIND - 1))
}

function init {
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0x38 0x3c 0x38 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0x38 0x3c 0x38 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0x38 0x3c 0x38 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0x28 0x2c 0x28 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x28 0x2c 0x28 0x88 0x8c 0x88 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0xc8 0xcc 0xc8 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0x18 0x1c 0x18 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0x68 0x6c 0x68 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0x18 0x1c 0x18 i
    i2cset -y $I2C_BUS $LCD_ADDR 0x08 0x0c 0x08 0x28 0x2c 0x28 i
}

function hexCmd {
    readonly LCD_BACKLIGHT=0x08
    readonly MODE=${2:-0x0}
    readonly EN=0x4

    FIRST=$((${MODE} | ( $1 & 0xF0 ) ))
    SECOND=$((${MODE} | ( ($1 << 4 ) & 0xF0 ) ))

    PART_A=$((${FIRST} | ${LCD_BACKLIGHT} ))
    PART_C=$((${FIRST} | ${EN} | ${LCD_BACKLIGHT}))
    PART_D=$((${FIRST} & (~${EN}) | ${LCD_BACKLIGHT} ))

    PART_B=$((${SECOND} | ${LCD_BACKLIGHT} ))
    PART_E=$((${SECOND} | ${EN} | ${LCD_BACKLIGHT} ))
    PART_F=$((${SECOND} & (~${EN}) | ${LCD_BACKLIGHT} ))

    printf '0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X' $PART_A $PART_B $PART_C $PART_D $PART_E $PART_F
}

function ordHex {
  LC_CTYPE=C printf '0x%X ' "'$1"
}

function sendString {
    msg=${1:-""}
    for (( i=0; i<${#msg}; i++ )); do
        ordHex "${msg:$i:1}"
    done
}

if [[ ${ROWS} == "1" ]]; then
    # One  Row
    ADDRS=(0x80)
elif [[ ${ROWS} == "2" ]]; then
    # Two Rows
    ADDRS=(0x80 0xC0)
elif [[ ${ROWS} == "4" ]]; then
    #Three Rows
    ROW3=$((0x80+${COLS}))
    ROW4=$((0xC0+${COLS}))
    ADDRS=(0x80 0xC0 $(printf '0x%X' ${ROW3}) $(printf '0x%X' ${ROW4}))
else 
    echo "Error: Invalid number of rows"
    exit 1
fi

function writeLine {
    MODE_RS=0x1
    LINEMSG=${1:-""}
    LINEADDR="${2:-${ADDRS[0]})}"
    CLEAN=${3:-true}
    COLSDEC=$((16#${COLS:2}))

    PMESG=""

    if (( ${#LINEMSG} > ${COLSDEC} )); then
        PMESG="${LINEMSG:0:${COLSDEC}}"
    else
        if [[ "${CLEAN}" == "true" ]]; then
            PMESG=$(printf "%-${COLSDEC}s" "${LINEMSG}" )
        else
            PMESG="${LINEMSG}"
        fi
    fi

    i2cset -y $I2C_BUS $LCD_ADDR $(hexCmd ${LINEADDR}) i
    for char in $(sendString "${PMESG}"); do
       i2cset -y $I2C_BUS $LCD_ADDR $( hexCmd ${char} ${MODE_RS}) i
    done
}

parseArgs $@

init

(( ${#MSG_1} > 0 && ${ROWS} >= 1 )) && writeLine "${MSG_1}" "${ADDRS[0]}"
(( ${#MSG_2} > 0 && ${ROWS} >= 2 )) && writeLine "${MSG_2}" "${ADDRS[1]}"
(( ${#MSG_3} > 0 && ${ROWS} >= 3)) && writeLine "${MSG_3}" "${ADDRS[2]}"
(( ${#MSG_4} > 0 && ${ROWS} == 4)) && writeLine "${MSG_4}" "${ADDRS[3]}"

exit 0
