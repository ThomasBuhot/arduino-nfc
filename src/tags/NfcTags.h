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
#include "log/NfcLog.h"
#include "nci/NfcNci.h"
#include "nci/NfcNci.h"

// State definition
enum {
    TAGS_STATE_NONE = 0,
    // reset command states
    TAGS_STATE_INIT_RESET,
    TAGS_STATE_INIT_INIT,
    TAGS_STATE_INIT_DONE,
    // discover command states
    TAGS_STATE_DISCOVER_MAP,
    TAGS_STATE_DISCOVER,
    TAGS_STATE_DISCOVER_NTF,
    TAGS_STATE_DISCOVER_ACTIVATED,
    // disconnect command states
    TAGS_STATE_DEACTIVATE,
    TAGS_STATE_DEACTIVATE_RSP,
    TAGS_STATE_DEACTIVATE_NTF
};
typedef uint8_t tTAGS_STATE;

// Interface identifier
enum {
    TAGS_ID_NONE = 0,
    TAGS_ID_RESET,
    TAGS_ID_DISCOVER,
    TAGS_ID_DISCOVER_ACTIVATED,
    TAGS_ID_DEACTIVATE
};
typedef uint16_t tTAGS_ID;

// Status definition
enum {
    TAGS_STATUS_OK = 0,
    TAGS_STATUS_REJECTED,
    TAGS_STATUS_FAILED
};

// NCI response type definition
typedef struct {
    void *data;
    uint16_t id;
    uint8_t status;
} tTAGS_NCI_RSP;

// Callback object that clients have to implement
// to be notified on response or event
class NfcTagsCb
{
    public:
        NfcTagsCb(void) {;}
        // Reset response
        virtual void cbReset(uint8_t status, uint16_t id, void *data) = 0;
        // Discover response
        virtual void cbDiscover(uint8_t status, uint16_t id, void *data) = 0;
        // Tag detection notification
        virtual void cbDiscoverNtf(uint8_t status, uint16_t id, void *data) = 0;
        // Deactivation response
        virtual void cbDeactivate(uint8_t status, uint16_t id, void *data) = 0;
};

// Tag API object definition which interfaces with the NCI
// and implements its callback to be notified on NCI response
// or event
class NfcTags : public NfcNciCb
{
    public:
        NfcTags(NfcLog& log, NfcNci& nci);
        void init(NfcTagsCb *cb) {_cb = cb;}
        void handleEvent(void);

    public:
        // Reset stack and hardware
        uint8_t reset(void);
        // Configure RF and start the discovering loop
        uint8_t discover(void);
        // Deactivate an activated tag and re-start
        // the discovering loop
        uint8_t deactivate(void);

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
        // error
        void cbError(uint8_t status, uint16_t id, void *data);
        // internal stuff
        void setNciResponse(uint8_t status, uint16_t id, void *data);
        uint8_t translateNciStatus(uint8_t nci_status);

    private:
        tTAGS_STATE _state;     // internal state
        tTAGS_ID _id;           // command or event identifier
        NfcLog& _log;           // logging interface
        NfcNci& _nci;           // NCI interface
        NfcTagsCb *_cb;         // Callback object
        void *_data;            // application data
        tTAGS_NCI_RSP _nciRsp;  // NCI response
};

#endif /* __NFC_TAGS_H__ */
