/*
 * NfcHw_pn7120.cpp
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

#include <Wire.h>
#include "NfcHw_pn7120.h"

void NfcHw_pn7120::init(void)
{
    // initialize interrupt and reset lines
    pinMode(_irq, INPUT);
    pinMode(_reset, OUTPUT);
    
    // VEN (reset) has to be HIGH
    digitalWrite(_reset, HIGH);
    delay(10);
    
    // join i2c bus
    Wire.begin();
}

uint8_t NfcHw_pn7120::write(uint8_t buf[], uint32_t len)
{
    // print buffer
    _log.bv("NCI_TX: ", buf, len);

    // i2c transfer starts with slave address (7 uppder bytes only),
    // then transmit the NCI packet
    Wire.beginTransmission(_address);
    while (len--) {
        Wire.write(*buf++);
    }
    Wire.endTransmission();

    return len;
}

uint8_t NfcHw_pn7120::read(uint8_t buf[], uint32_t len)
{
    // wait for response to be ready
    wait();

    // read response
    Wire.requestFrom(_address, len);
    do {
        *buf++ = Wire.read();
    } while (Wire.available());
    
    // print response
    _log.bv("NCI_RX: ", (uint8_t *)(buf-len), len);

    return len;
}

void NfcHw_pn7120::wait(void)
{
    /* poll irq */
    while (!digitalRead(_irq)) {
        delay(10);
    }
}

