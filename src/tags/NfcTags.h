/*
 * NfcTags.h
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

#ifndef __NFC_TAGS_H__
#define __NFC_TAGS_H__

#include <Arduino.h>
#include "tags/NfcTagsDef.h"
#include "tags/NfcTagsCb.h"
#include "tags/NfcTagsIntf.h"
#include "log/NfcLog.h"
#include "nci/NfcNci.h"

// Tag API object definition which interfaces with the NCI
// and implements its callback to be notified on NCI response
// or event
class NfcTags : public NfcNciCb
{
    public:
        NfcTags(NfcLog& log, NfcNci& nci);
        void init(NfcTagsCb *cb) {_p_cb = cb; _tag2.init(cb);}
        void handleEvent(void);

    // public API
    public:
        // reset command of stack and hardware
        // response is callback function cbReset()
        uint8_t cmdReset(void);
        // command to configure RF and start the discovering loop
        // response is callback function cbDiscover()
        // notification when tag is found is function cbDiscoverNtf()
        uint8_t cmdDiscover(void);
        // command to deactivate an activated tag (found tag) and re-start
        // the discovering loop
        // response is callback function cbDeactivate()
        uint8_t cmdDeactivate(void);
        // get tag interface object for low level commands
        NfcTagsIntf* getInterface(void) {return _p_tagIntf;}
        // command to dump an activated (found) tag
        // response is callback function cbDump()
        uint8_t cmdDump(void);

    private:
        // reset
        void handleReset(void);
        void cbCoreReset(uint8_t status, uint16_t id, void *data);
        void cbCoreInit(uint8_t status, uint16_t id, void *data);
        // discover
        void handleDiscover(void);
        void cbRfDiscoverMap(uint8_t status, uint16_t id, void *data);
        void cbRfDiscover(uint8_t status, uint16_t id, void *data);
        void cbRfDiscoverNtf(uint8_t status, uint16_t id, void *data);
        // deactivate
        void handleDeactivate(void);
        void cbRfDeactivate(uint8_t status, uint16_t id, void *data);
        void cbRfDeactivateNtf(uint8_t status, uint16_t id, void *data);
        // dump
        void handleDump(void);
        // Data exchange callback
        void cbData(uint8_t status, uint16_t id, void *data);
        // error
        void cbError(uint8_t status, uint16_t id, void *data);
        // internal stuff
        void setNciResponse(uint8_t status, uint16_t id, void *data);
        uint8_t translateNciStatus(uint8_t nci_status);
        void identifyTag(tNCI_RF_INTF *rf_intf);

    private:
        uint8_t _state;                 // internal state
        uint8_t _id;                    // command or event identifier
        NfcLog& _log;                   // logging interface
        NfcNci& _nci;                   // NCI interface
        NfcTagsCb *_p_cb;               // callback object
        void *_data;                    // application data
        tTAGS_NCI_RSP _nciRsp;          // NCI response
        NfcTagsIntfType2 _tag2;         // NFC Forum tag type 2
        NfcTagsIntfMifare _tagMifare;   // NXP Mifare classic / plus tag
        NfcTagsIntf *_p_tagIntf;        // current tag interface
};

#endif // __NFC_TAGS_H__
