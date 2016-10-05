/*
 * NfcNci.cpp
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

#include "NfcTags.h"

// NCI RF configuration for tag detection
// Discover tag type 1, 2 and 3 in polling mode
// with frame interface.
// Poll tag detection RF mode A and F.
tNCI_DISCOVER_MAPS discover_maps[] =
{
    // T1T + poll mode + frame RF interface
    {
        NCI_PROTOCOL_T1T,
        NCI_INTERFACE_MODE_POLL,
        NCI_INTERFACE_FRAME
    },
    // T2T + poll mode + frame RF interface
    {
        NCI_PROTOCOL_T2T,
        NCI_INTERFACE_MODE_POLL,
        NCI_INTERFACE_FRAME
    },
    // T3T + poll mode + frame RF interface
    {
        NCI_PROTOCOL_T3T,
        NCI_INTERFACE_MODE_POLL,
        NCI_INTERFACE_FRAME
    }
};

tNCI_DISCOVER_CONFS discover_confs[] =
{
    // poll A + always
    {
        NCI_DISCOVERY_TYPE_POLL_A,
        NCI_DISCOVERY_FREQUENCY_ALWAYS
    },
    // poll F + always
    {
        NCI_DISCOVERY_TYPE_POLL_F,
        NCI_DISCOVERY_FREQUENCY_ALWAYS
    }
};

// State strings
const char *nfcTagsStateToStr[] = {
    "TAGS_STATE_NONE",
    // reset command states
    "TAGS_STATE_INIT_RESET",
    "TAGS_STATE_INIT_INIT",
    "TAGS_STATE_INIT_DONE",
    // discover command states
    "TAGS_STATE_DISCOVER_MAP",
    "TAGS_STATE_DISCOVER",
    "TAGS_STATE_DISCOVER_NTF",
    "TAGS_STATE_DISCOVER_ACTIVATED",
    // disconnect command states
    "TAGS_STATE_DEACTIVATE",
    "TAGS_STATE_DEACTIVATE_RSP",
    "TAGS_STATE_DEACTIVATE_NTF"
};

NfcTags::NfcTags(NfcLog& log, NfcNci& nci) :
         _state(TAGS_STATE_NONE), _id(TAGS_ID_NONE),
         _log(log), _nci(nci)
{
    _cb = NULL;
    _data = NULL;
}

void NfcTags::setNciResponse(uint8_t status, uint16_t id, void *data)
{
    _nciRsp.data = data;
    _nciRsp.id = id;
    _nciRsp.status = status;
}

uint8_t NfcTags::translateNciStatus(uint8_t nci_status)
{
    uint8_t status;

    switch(nci_status) {
        case NCI_STATUS_OK:
            status = TAGS_STATUS_OK;
            break;
        case NCI_STATUS_REJECTED:
            status = TAGS_STATUS_REJECTED;
            break;
        default:
            status = TAGS_STATUS_FAILED;
            break;
    }

    return status;
}

void NfcTags::cbError(uint8_t status, uint16_t id, void *data)
{
    _log.e("NfcTags: %s %u %u", __func__, status, _id);
}

void NfcTags::handleEvent(void)
{
    // check callback object
    if (_cb == NULL) {
        _log.e("NfcTags: %s no callback object registered\n", __func__);
        return;
    }

    // process event per command
    switch(_id) {
        case TAGS_ID_RESET:
            handleReset();
            break;
        case TAGS_ID_DISCOVER:
            handleDiscover();
            break;
        case TAGS_ID_DEACTIVATE:
            handleDeactivate();
            break;
        default:
            _log.e("NfcTags: %s ignore unknown event %d\n", __func__, _id);
            break;
    }
}

uint8_t NfcTags::reset(void)
{
    // reset command migth be sent at anytime
    // reset state accordingly
    _log.d("NfcTags: %s state = %s\n", __func__, nfcTagsStateToStr[_state]);
    _state = TAGS_STATE_INIT_RESET;
    _id = TAGS_ID_RESET;
    return TAGS_STATUS_OK;
}

void NfcTags::handleReset(void)
{
    uint8_t status;

    _log.d("NfcTags: %s state = %s\n", __func__, nfcTagsStateToStr[_state]);

    // reset command state machine
    switch (_state) {
        case TAGS_STATE_INIT_RESET:
            // send NCI reset command
            status = _nci.cmdCoreReset(NCI_RESET_TYPE_KEEP_CFG);
            break;
        case TAGS_STATE_INIT_INIT:
            // send NCI init command
            status = _nci.cmdCoreInit();
            break;
        default:
            // unhandled state
            status = NCI_STATUS_REJECTED;
            break;
    }

    // check status and notify
    if (status != NCI_STATUS_OK) {
        status = translateNciStatus(status);
        _log.e("NfcTags: %s state = %s error status = %d\n", __func__, nfcTagsStateToStr[_state], status);
        _cb->cbReset(status, TAGS_ID_RESET, NULL);
    }
} 

void NfcTags::cbCoreReset(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTags: %s status = %d id = %d\n", __func__, status, id);
    setNciResponse(status, id, data);

    if (status == NCI_STATUS_OK) {
        if (id == NCI_ID_RSP_CORE_RESET) {
            _log.i("NfcTags: NFC controller reseted\n");
            _state = TAGS_STATE_INIT_INIT;
            return;
        }
        else {
            _log.e("NfcTags: %s incorrect id = %d\n, __func__, id");
        }
    }

    status = translateNciStatus(status);
    _cb->cbDeactivate(status, TAGS_ID_RESET, NULL);
}

void NfcTags::cbCoreInit(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTags: %s status = %d id = %d\n", __func__, status, id);
    setNciResponse(status, id, data);

    if (status == NCI_STATUS_OK) {
        if (id == NCI_ID_RSP_CORE_INIT) {
            _log.i("NfcTags: NFC controller initialized\n");
            status = TAGS_STATUS_OK;
            _state = TAGS_STATE_INIT_DONE;
        }
        else {
            _log.e("NfcTags: %s incorrect id = %d\n, __func__, id");
            status = TAGS_STATUS_FAILED;
        }
    }
    else {
        status = translateNciStatus(status);
    }

    _cb->cbReset(status, TAGS_ID_RESET, NULL);
}

uint8_t NfcTags::discover(void)
{
    uint8_t status;

    _log.d("NfcTags: %s state = %s\n", __func__, nfcTagsStateToStr[_state]);

    // check state
    if (_state != TAGS_STATE_INIT_DONE) {
        status = TAGS_STATUS_REJECTED;
        goto bail;
    }

    // prepare state machine
    _state = TAGS_STATE_DISCOVER_MAP;
    _id = TAGS_ID_DISCOVER;
    status = TAGS_STATUS_OK;

bail:
    return status;
}

void NfcTags::handleDiscover(void)
{
    uint8_t status;
    
    _log.d("NfcTags: %s state = %s\n", __func__, nfcTagsStateToStr[_state]);

    // discover command state machine
    switch(_state) {
        case TAGS_STATE_DISCOVER_MAP:
            // send NCI discover map command
            status = _nci.cmdRfDiscoverMap(sizeof(discover_maps)/sizeof(tNCI_DISCOVER_MAPS), discover_maps);
            break;
        case TAGS_STATE_DISCOVER:
            // send NCI discover command to activate polling
            status = _nci.cmdRfDiscover(sizeof(discover_confs)/sizeof(tNCI_DISCOVER_CONFS), discover_confs);
            break;
        case TAGS_STATE_DISCOVER_NTF:
            // wait for tag detection
            status = NCI_STATUS_OK;
            break;
        case TAGS_STATE_DISCOVER_ACTIVATED:
            // tags activated
            status = NCI_STATUS_OK;
            break;
        default:
            // unhandled state
            status = NCI_STATUS_REJECTED;
            break;
    }

    // check status and notify
    if (status != NCI_STATUS_OK) {
        _log.e("NfcTags: %s state = %s error status = %d\n", __func__, nfcTagsStateToStr[_state], status);
        status = translateNciStatus(status);
        _cb->cbDiscover(status, TAGS_ID_DISCOVER, NULL);
    }
}

void NfcTags::cbRfDiscoverMap(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTags: %s status = %d id = %d\n", __func__, status, id);
    setNciResponse(status, id, data);

    if (status == NCI_STATUS_OK) {
        if (id == NCI_ID_RSP_RF_DISCOVER_MAP) {
            _log.i("NfcTags: RF discovering mode configured\n");
            _state = TAGS_STATE_DISCOVER;
            return;
        }
        else {
            _log.e("NfcTags: %s incorrect id = %d\n, __func__, id");
        }
    }

    status = translateNciStatus(status);
    _cb->cbDeactivate(status, TAGS_ID_DISCOVER, NULL);
}

void NfcTags::cbRfDiscover(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTags: %s status = %d id = %d\n", __func__, status, id);
    setNciResponse(status, id, data);

    if (status == NCI_STATUS_OK) {
        if (id == NCI_ID_RSP_RF_DISCOVER) {
            _log.i("NfcTags: tag detection started\n");
            status = TAGS_STATUS_OK;
            _state = TAGS_STATE_DISCOVER_NTF;
        }
        else {
            _log.e("NfcTags: %s incorrect id = %d\n, __func__, id");
            status = TAGS_STATUS_FAILED;
        }
    }
    else {
        status = translateNciStatus(status);
    }

    _cb->cbDiscover(status, TAGS_ID_DISCOVER, NULL);
}

void NfcTags::cbRfDiscoverNtf(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTags: %s status = %d id = %d\n", __func__, status, id);
    setNciResponse(status, id, data);

    if (status == NCI_STATUS_OK) {
        if (id == NCI_ID_NTF_RF_INTF_ACTIVATED) {
            _log.i("NfcTags: tag activated\n");
            status = TAGS_STATUS_OK;
            _state = TAGS_STATE_DISCOVER_ACTIVATED;
        }
        else {
            _log.e("NfcTags: %s incorrect id = %d\n, __func__, id");
            status = TAGS_STATUS_FAILED;
        }
    }
    else {
        status = translateNciStatus(status);
    }

    _cb->cbDiscoverNtf(status, TAGS_ID_DISCOVER_ACTIVATED, _nciRsp.data);
}

uint8_t NfcTags::deactivate(void)
{
    uint8_t status;

    _log.d("NfcTags: %s state = %s\n", __func__, nfcTagsStateToStr[_state]);

    // check state
    if (_state != TAGS_STATE_DISCOVER_ACTIVATED) {
        status = TAGS_STATUS_REJECTED;
        goto bail;
    }

    // prepare state machine
    _state = TAGS_STATE_DEACTIVATE;
    _id = TAGS_ID_DEACTIVATE;
    status = TAGS_STATUS_OK;

bail:
    return status;
}

void NfcTags::handleDeactivate(void)
{
    uint8_t status;

    _log.d("NfcTags: %s state = %s\n", __func__, nfcTagsStateToStr[_state]);

    // discover command state machine
    switch(_state) {
        case TAGS_STATE_DEACTIVATE:
            // send NCI discover and restart discovering
            status = _nci.cmdRfDeactivate(NCI_DEACTIVATE_TYPE_DISCOVERY);
            break;
        case TAGS_STATE_DEACTIVATE_RSP:
            // do nothing, wait for notification
            status = NCI_STATUS_OK;
            break;
        case TAGS_STATE_DEACTIVATE_NTF:
            // change handler, switch to discover
            _id = TAGS_ID_DISCOVER;
            _state = TAGS_STATE_DISCOVER_NTF;
            status = NCI_STATUS_OK;
            break;
        default:
            // unhandled state
            status = NCI_STATUS_REJECTED;
            break;
    }

    // check status and notify
    if (status != TAGS_STATUS_OK) {
        _log.e("NfcTags: %s state = %s error status = %d\n", __func__, nfcTagsStateToStr[_state], status);
        status = translateNciStatus(status);
        _cb->cbDeactivate(status, TAGS_ID_DEACTIVATE, NULL);
    }
}

void NfcTags::cbRfDeactivate(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTags: %s status = %d id = %d\n", __func__, status, id);
    setNciResponse(status, id, data);

    if (status == NCI_STATUS_OK) {
        if (id == NCI_ID_RSP_RF_DEACTIVATE) {
            _state = TAGS_STATE_DEACTIVATE_RSP;
            return;
        }
        else {
            _log.e("NfcTags: %s incorrect id = %d\n, __func__, id");
        }
    }

    status = translateNciStatus(status);
    _cb->cbDeactivate(status, TAGS_ID_DEACTIVATE, NULL);
}

void NfcTags::cbRfDeactivateNtf(uint8_t status, uint16_t id, void *data)
{
    _log.d("NfcTags: %s status = %d id = %d\n", __func__, status, id);
    setNciResponse(status, id, data);

    if (status == NCI_STATUS_OK) {
        if (id == NCI_ID_NTF_RF_DEACTIVATE) {
            _log.i("NfcTags: tag deactivated\n");
            // re-start discover loop
            // change handler, switch to discover
            _id = TAGS_ID_DISCOVER;
            _state = TAGS_STATE_DISCOVER_NTF;
            status = TAGS_STATUS_OK;
        }
        else {
            _log.e("NfcTags: %s incorrect id = %d\n, __func__, id");
            status = TAGS_STATUS_FAILED;
        }
    }
    else {
        status = translateNciStatus(status);
    }

    _cb->cbDeactivate(status, TAGS_ID_DEACTIVATE, NULL);
}

