/*
 * NfcTagsIntfType2.cpp
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

#include "tags/NfcTagsIntf.h"

// state definition
enum {
    TAGS_INTF_T2_STATE_NONE = 0,
    // dump command states
    TAGS_INTF_T2_STATE_DUMP,
    TAGS_INTF_T2_STATE_DUMP_RSP
};

// state strings
const char *nfcTagsIntfType2[] = {
    "TAGS_INTF_T2_STATE_NONE",
    // reset command states
    "TAGS_INTF_T2_STATE_DUMP",
    "TAGS_INTF_T2_STATE_DUMP_RSP"
};

// event definition
enum {
    TAGS_INTF_T2_ID_NONE,
    TAGS_INTF_T2_ID_DUMP
};

// tag type 2 commands
#define CMD_READ    0x30
#define CMD_WRITE   0xA2

// tag type 2 memory mapping definitions
#define MEMORY_BLOCK_SIZE_BYTES         4   // 4 bytes per block
#define MEMORY_READ_BLOCK               4   // read 4 blocks
#define MEMORY_FIRST_BLOCK              0   // 1st block
#define MEMORY_LAST_BLOCK               15  // last block for static memory mapping

NfcTagsIntfType2::NfcTagsIntfType2(NfcLog& log, NfcNci& nci) :
    NfcTagsIntf(log, nci)
{
    _state = TAGS_INTF_T2_STATE_NONE;
}

uint8_t NfcTagsIntfType2::getType(void)
{
    return TAGS_TYPE_2;
}

uint8_t NfcTagsIntfType2::getNfcidLen(void)
{
    uint8_t len = 0;

    if(_p_rf != NULL) {
        len = ((tNCI_RF_INTF *)_p_rf)->specific.params.poll_a.nfcid_len;
    }

    return len;
}

uint8_t* NfcTagsIntfType2::getNfcidBuf(void)
{
    uint8_t *buf = NULL;

    if(_p_rf != NULL) {
        buf = ((tNCI_RF_INTF *)_p_rf)->specific.params.poll_a.nfcid;
    }

    return buf;
}

uint8_t NfcTagsIntfType2::cmdDump(void)
{
    uint8_t status;

    _log.d("NfcTagsIntfType2: %s state = %s\n", __func__, nfcTagsIntfType2[_state]);

    // check state
    if (_state != TAGS_INTF_T2_STATE_NONE) {
        status = TAGS_STATUS_REJECTED;
        goto bail;
    }

    // prepare state machine, state is unchanged
    _id = TAGS_INTF_T2_ID_DUMP;
    _state = TAGS_INTF_T2_STATE_DUMP;
    status = TAGS_STATUS_OK;

    // reset block number
    _block = MEMORY_FIRST_BLOCK;

bail:
    return status;
}

uint8_t NfcTagsIntfType2::handleDump(void)
{
    uint8_t status;
    uint8_t buf[2];

    _log.d("NfcTags: %s state = %s\n", __func__, nfcTagsIntfType2[_state]);

    switch(_state) {
        case TAGS_INTF_T2_STATE_DUMP:
            // send NCI read command
            if (_block <= (MEMORY_LAST_BLOCK + 1 - MEMORY_READ_BLOCK)) {
                buf[0] = CMD_READ;
                buf[1] = _block;
                status = _nci.dataSend(NCI_CID_RF_STATIC, buf, sizeof(buf));
                _state = TAGS_INTF_T2_STATE_DUMP_RSP;
            }
            else {
                status = NCI_STATUS_REJECTED;
            }
            break;
        case TAGS_INTF_T2_STATE_DUMP_RSP:
            // do nothing, wait for response
            status = NCI_STATUS_OK;
            break;
    }

    // check status and notify
    if (status != NCI_STATUS_OK) {
        status = translateNciStatus(status);
    }

    return status;
}

void NfcTagsIntfType2::handleData(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTagsIntfType2: %s status = %d id = %d\n", __func__, status, id);

    switch(_id) {
        case TAGS_INTF_T2_ID_DUMP:
            handleDataDump(status, id, data);
            break;
        default:
            break;
    }
}

void NfcTagsIntfType2::handleDataDump(uint8_t status, uint16_t id, void *data)
{
    uint8_t *buf = (uint8_t*) data;

    _log.d("NfcTagsIntfType2: %s status = %d id = %d\n", __func__, status, id);
    status = translateNciStatus(status);

    // check message is not corrupted
    // and if dump is complete or not
    if (status == TAGS_STATUS_OK &&
        buf[0] == (MEMORY_READ_BLOCK * MEMORY_BLOCK_SIZE_BYTES + 1) &&
        buf[MEMORY_READ_BLOCK * MEMORY_BLOCK_SIZE_BYTES + 1] == 0) {
        // payload format is: len | data | status
        _dump.buf = &buf[1];
        _dump.len = buf[0] - 1;
        if (_block < (MEMORY_LAST_BLOCK + 1 - MEMORY_READ_BLOCK)) {
            _dump.more = 1;
            _state = TAGS_INTF_T2_STATE_DUMP;
            _block += MEMORY_READ_BLOCK;
        }
        else {
            _dump.more = 0;
            _state = TAGS_INTF_T2_STATE_NONE;
        }
    }
    else {
        _dump.buf = NULL;
        _dump.len = 0;
        _dump.more = 0;
        _state = TAGS_INTF_T2_STATE_NONE;
    }

    _p_cb->cbDump(status, TAGS_ID_DUMP, &_dump);
}
