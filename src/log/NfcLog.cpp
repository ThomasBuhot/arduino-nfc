/*
 * NfcLog.cpp
 *
 * Copyright (c) Thomas Buhot. All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "NfcLog.h"

void NfcLog::print(uint8_t level, const char* str, va_list args)
{
    if (_level < level) {
        return;
    }

    // loop through format string
    for (; *str != 0; ++str) {
        if (*str == '%') {
            ++str;
            if (*str == '\0') break;
            if (*str == '%') {
                Serial.print(*str);
                continue;
            }
            if( *str == 's' ) {
                register char *s = (char *)va_arg( args, int);
                Serial.print(s);
                continue;
            }
            if( *str == 'd' || *str == 'i') {
                Serial.print(va_arg( args, int), DEC);
                continue;
            }
            if( *str == 'x' ) {
                Serial.print(va_arg( args, int), HEX);
                continue;
            }
            if( *str == 'X' ) {
                Serial.print("0x");
                Serial.print(va_arg( args, int), HEX);
                continue;
            }
            if( *str == 'b' ) {
                Serial.print(va_arg( args, int), BIN);
                continue;
            }
            if( *str == 'B' ) {
                Serial.print("0b");
                Serial.print(va_arg( args, int), BIN);
                continue;
            }
            if( *str == 'l' ) {
                Serial.print(va_arg( args, long), DEC);
                continue;
            }

            if( *str == 'c' ) {
                Serial.print(va_arg( args, int));
                continue;
            }
            if( *str == 't' ) {
                if (va_arg( args, int ) == 1) {
                    Serial.print("T");
                }
                else {
                    Serial.print("F");        
                }
                continue;
            }
            if( *str == 'T' ) {
                if (va_arg( args, int ) == 1) {
                    Serial.print("true");
                }
                else {
                    Serial.print("false");        
                }
                continue;
            }

        }
        Serial.print(*str);
    }
}

void NfcLog::print(uint8_t level, const char* str, const uint8_t buf[], uint32_t len)
{
    if (_level < level || buf == NULL || len == 0) {
        return;
    }
    Serial.print(F(str));
    Serial.print(F("0x"));
    Serial.print((uint32_t)buf);
    Serial.print(F("["));
    Serial.print(len);
    Serial.print(F("]:"));

    while (len--) {
        Serial.print(F(" 0x"));
        Serial.print(*buf++, HEX);
    }
    Serial.print(F("\n"));
}
