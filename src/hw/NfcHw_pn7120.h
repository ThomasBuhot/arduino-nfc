/*
 * NfcHw_pn7120.h
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

#ifndef __PN7120_H__
#define __PN7120_H__

#include <Arduino.h>
#include "log/NfcLog.h"
#include "NfcHw.h"

class NfcHw_pn7120 : public NfcHw
{
    public:
        NfcHw_pn7120(NfcLog& log, uint8_t irq, uint8_t reset, uint8_t address) :
            NfcHw(log), _irq(irq), _reset(reset), _address(address) {;}
        void init(void);
        uint8_t write(uint8_t buf[], uint32_t len);
        uint8_t read(uint8_t buf[], uint32_t len);
        void wait(void);

    private:
        uint8_t _irq;
        uint8_t _reset;
        uint8_t _address;
};

#endif /* __PN7120_H__ */
