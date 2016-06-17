/*
 * NfcHw.h
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

#ifndef __NFC_HW__
#define __NFC_HW__

#include <Arduino.h>
#include "log/NfcLog.h"

class NfcHw
{
    public:
        NfcHw(NfcLog& log) : _log(log) {;}
        virtual void init(void) = 0;
        virtual uint8_t write(uint8_t buf[], uint32_t len) = 0;
        virtual uint8_t read(uint8_t buf[], uint32_t len) = 0;
        virtual void wait(void) = 0;

    protected:
        NfcLog& _log;
};

#endif /* __NFC_HW__ */
