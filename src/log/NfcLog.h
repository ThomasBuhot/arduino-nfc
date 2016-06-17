/*
 * NfcLog.h
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

#ifndef __NFC_LOG_H__
#define __NFC_LOG_H__

#include <Arduino.h>
#include <stdarg.h>

#define NFC_LOG_LEVEL_OFF     0 
#define NFC_LOG_LEVEL_ERROR   1
#define NFC_LOG_LEVEL_INFO    2
#define NFC_LOG_LEVEL_DEBUG   3
#define NFC_LOG_LEVEL_VERBOSE 4

class NfcLog {
    public:
        NfcLog(uint8_t level) : _level(level) {;}
        void init(uint32_t baudRate) {Serial.begin(baudRate);}
        void v(const char* str, ...) {va_list args; va_start(args, F(str)); print(NFC_LOG_LEVEL_VERBOSE, str, args);}
        void d(const char* str, ...) {va_list args; va_start(args, F(str)); print(NFC_LOG_LEVEL_DEBUG, str, args);}
        void i(const char* str, ...) {va_list args; va_start(args, F(str)); print(NFC_LOG_LEVEL_INFO, str, args);}
        void e(const char* str, ...) {va_list args; va_start(args, F(str)); print(NFC_LOG_LEVEL_ERROR, str, args);}
        void bv(const char* str, const uint8_t buf[], uint32_t len) {print(NFC_LOG_LEVEL_VERBOSE, str, buf, len);}
        void bi(const char* str, const uint8_t buf[], uint32_t len) {print(NFC_LOG_LEVEL_INFO, str, buf, len);}

    private:
        NfcLog() : _level(NFC_LOG_LEVEL_OFF) {;}
        void print(uint8_t level, const char* str, va_list args);
        void print(uint8_t level, const char* str, const uint8_t buf[], uint32_t len);

    private:
        uint8_t _level;
};

#endif /* __NFC_LOG_H__ */
