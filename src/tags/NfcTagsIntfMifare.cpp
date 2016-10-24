/*
 * NfcTagsIntfMifare.cpp
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
    TAGS_INTF_MIFARE_STATE_NONE = 0
};

// state strings
const char *nfcTagsIntfMifare[] = {
    "TAGS_INTF_MIFARE_STATE_NONE"
};

// event definition
enum {
    TAGS_INTF_MIFARE_ID_NONE
};

NfcTagsIntfMifare::NfcTagsIntfMifare(NfcLog& log, NfcNci& nci) :
    NfcTagsIntf(log, nci)
{
    _state = TAGS_INTF_MIFARE_STATE_NONE;
}

uint8_t NfcTagsIntfMifare::getType(void)
{
    return TAGS_TYPE_MIFARE;
}

uint8_t NfcTagsIntfMifare::getNfcidLen(void)
{
    uint8_t len = 0;

    if(_p_rf != NULL) {
        len = ((tNCI_RF_INTF *)_p_rf)->specific.params.poll_a.nfcid_len;
    }

    return len;
}

uint8_t* NfcTagsIntfMifare::getNfcidBuf(void)
{
    uint8_t *buf = NULL;

    if(_p_rf != NULL) {
        buf = ((tNCI_RF_INTF *)_p_rf)->specific.params.poll_a.nfcid;
    }

    return buf;
}

uint8_t NfcTagsIntfMifare::cmdDump(void)
{
    uint8_t status;

    _log.d("NfcTagsIntfMifare: %s state = %s\n", __func__, nfcTagsIntfMifare[_state]);

    // FIXME: not implemented yet
    return TAGS_STATUS_REJECTED;
}

uint8_t NfcTagsIntfMifare::handleDump(void)
{
    _log.d("NfcTagsIntfMifare: %s state = %s\n", __func__, nfcTagsIntfMifare[_state]);

    // FIXME: not implemented yet
    return TAGS_STATUS_REJECTED;
}

void NfcTagsIntfMifare::handleData(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTagsIntfMifare: %s status = %d id = %d\n", __func__, status, id);

    // FIXME: not implemented yet
}

