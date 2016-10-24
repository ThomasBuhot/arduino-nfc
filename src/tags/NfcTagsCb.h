/*
 * NfcTagsCb.h
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

#ifndef __NFC_TAGS_CB_H__
#define __NFC_TAGS_CB_H__

// Callback object that clients have to implement
// to be notified on response or event
class NfcTagsCb
{
    public:
        NfcTagsCb(void) {;}
        // Reset response callback function for cmdReset()
        virtual void cbReset(uint8_t status, uint16_t id, void *data) = 0;
        // Discover response callback function for cmdDiscover()
        virtual void cbDiscover(uint8_t status, uint16_t id, void *data) = 0;
        // Tag detection callback notification function for cmdDiscover()
        virtual void cbDiscoverNtf(uint8_t status, uint16_t id, void *data) = 0;
        // Deactivation response callback function for cmdDeactivate()
        virtual void cbDeactivate(uint8_t status, uint16_t id, void *data) = 0;
        // Dump response callback function for cmdDump()
        virtual void cbDump(uint8_t status, uint16_t id, void *data) = 0;
};

#endif // __NFC_TAGS_CB_H__
