/*
 * NfcTagsIntf.h
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

#ifndef __NFC_TAGS_INTF_H__
#define __NFC_TAGS_INTF_H__

#include <Arduino.h>
#include "tags/NfcTagsDef.h"
#include "tags/NfcTagsCb.h"
#include "log/NfcLog.h"
#include "nci/NfcNci.h"

// Tag interface object to exchange with activated tags
class NfcTagsIntf
{
    public:
        NfcTagsIntf(NfcLog& log, NfcNci& nci) :
            _log(log), _nci(nci), _p_cb(NULL), _p_rf(NULL) {;}
        void init(NfcTagsCb *cb) {_p_cb = cb;}
        void initTag(tNCI_RF_INTF *rf) {_p_rf = rf;}

    // public API
    public:
        // get NFC Forum tag type
        virtual uint8_t getType(void) = 0;
        // get NFCID length
        virtual uint8_t getNfcidLen(void) = 0;
        // get NFCID buffer
        virtual uint8_t* getNfcidBuf(void) = 0;
        // command to dump an activated (found) tag
        // response is callback function cbDump()
        virtual uint8_t cmdDump(void) = 0;

    // internal stuff
    public:
        virtual uint8_t handleDump(void) = 0;
        virtual void handleData(uint8_t status, uint16_t id, void *data) = 0;

    // internal stuff
    protected:
        uint8_t translateNciStatus(uint8_t nci_status);

    // internal stuff
    protected:
        uint8_t _state;         // internal state
        uint8_t _id;            // command
        NfcLog& _log;           // logging interface
        NfcNci& _nci;           // NCI interface
        NfcTagsCb *_p_cb;       // callback object
        tNCI_RF_INTF *_p_rf;    // tag RF interface
        tTAGS_DUMP  _dump;      // dump structure
};

// Tag interface object to exchange with activated tags of type 2
// (see NFC Forum definition), this includes NXP Mifare Ultra Ligth
class NfcTagsIntfType2 : public NfcTagsIntf
{
    public:
        NfcTagsIntfType2(NfcLog& log, NfcNci& nci);

    // public API
    public:
        uint8_t getType(void);
        uint8_t getNfcidLen(void);
        uint8_t* getNfcidBuf(void);
        uint8_t cmdDump(void);

    // internal stuff
    public:
        uint8_t handleDump(void);
        void handleData(uint8_t status, uint16_t id, void *data);

    private:
        void handleDataDump(uint8_t status, uint16_t id, void *data);

    private:
        uint8_t _block;
};

// Tag interface object to exchange with activated tags of type Mifare
// (see NFC Forum definition), this includes NXP Mifare Classic and Plus
class NfcTagsIntfMifare : public NfcTagsIntf
{
    public:
        NfcTagsIntfMifare(NfcLog& log, NfcNci& nci);

    // public API
    public:
        uint8_t getType(void);
        uint8_t getNfcidLen(void);
        uint8_t* getNfcidBuf(void);
        uint8_t cmdDump(void);

    // internal stuff
    public:
        uint8_t handleDump(void);
        void handleData(uint8_t status, uint16_t id, void *data);
};

#endif // __NFC_TAGS_INTF__
