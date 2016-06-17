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

#include "NfcNci.h"

// callback object
tNCI_CB _cb;

// reset response object
tNCI_RESET _reset;

// RF interface notification object
tNCI_RF_INTF _rf_intf;

// deactivate oject
tNCI_DEACTIVATE _deactivate;

#define getRxBuffer()   (_rx_buf)
#define getTxBuffer()   (_tx_buf)

NfcNci::NfcNci(NfcLog& log, NfcHw& hw) :
        _state(NCI_STATE_NONE), _log(log), _hw(hw)
{
    _cb.p_cb = NULL;
    _cb.data = NULL;
}

void NfcNci::registerCb(pNCI_CB p_cb)
{
    _cb.p_cb = p_cb;
    _cb.data = NULL;
}

uint32_t NfcNci::waitForEvent(uint8_t buf[])
{
    uint32_t len, ret;

    // read header
    ret = _hw.read(buf, NCI_MSG_HDR_SIZE);
    if (ret <= 0) {
        goto end;
    }

    // check length
    len = buf[NCI_OFFSET_LEN];

    // read payload
    ret = _hw.read(&buf[NCI_MSG_HDR_SIZE], len);
    if (ret <= 0) {
        goto end;
    }
    ret = len + NCI_MSG_HDR_SIZE;

end:
    return ret;
}

void NfcNci::handleEvent(void)
{
    uint8_t *p, *buf;
    uint8_t mt, pbf, gid, oid, status;
    uint32_t len;

    // wait for event
    oid = mt = 0;
    buf = getRxBuffer();
    len = waitForEvent(buf);
    if (len <= 0) {
        _log.e("NCI error: null event received\n");
        status = NCI_STATUS_FAILED;
        goto do_callback;
    }

    // read event header
    p = buf;
    NCI_MSG_PRS_HDR0(p, mt, pbf, gid);
    NCI_MSG_PRS_HDR1(p, oid);

    // check HDR0
    // FIXME: segmentation and reassembly message is not handled
    // FIXME: data packet message not handled
    if (pbf != NCI_PBF_NO_OR_LAST) {
        _log.e("NCI error: segmentation and reassembly messages not handled yet\n");
        status = NCI_STATUS_SYNTAX_ERROR;
        goto do_callback;
    }

    // broadcast to the right handler
    switch(gid) {
        case NCI_GID_CORE:
            status = handleCoreEvent(buf, len);
            break;
        case NCI_GID_RF_MANAGE:
            status = handleRfEvent(buf, len);
            break;
        case NCI_GID_EE_MANAGE:
        case NCI_GID_PROP:
        default:
            status = NCI_STATUS_UNKNOWN_GID;
            break;
    }

do_callback:
    if (_cb.p_cb != NULL) {
        (*_cb.p_cb)(status, UINT16_ID(mt, oid), _cb.data);
    }
}

uint8_t NfcNci::handleCoreEvent(uint8_t buf[], uint32_t len)
{
    uint8_t *p;
    uint8_t mt, pbf, gid, oid, status;

    // init
    p = buf;

    // get event header
    NCI_MSG_PRS_HDR0(p, mt, pbf, gid);
    NCI_MSG_PRS_HDR1(p, oid);

    // FIXME: only response message are handled at the moment
    if (mt != NCI_MT_RSP) {
        _log.e("NCI error: only response messages are handled by core now\n");
        status = NCI_STATUS_SYNTAX_ERROR;
        goto bail;
    }

    // FIXME: check event length = header + data length?
    // process event
    switch (oid) {
        case NCI_MSG_CORE_RESET:
            status = rspCoreReset(p);
            break;
        case NCI_MSG_CORE_INIT:
            status = rspCoreInit(p);
            break;
        default:
            _log.e("NCI error: unhandled core event oid = %d\n", oid);
            status = NCI_STATUS_UNKNOWN_OID;
            break;
    }

bail:
    return status;
}

uint8_t NfcNci::handleRfEvent(uint8_t buf[], uint32_t len)
{
    uint8_t *p;
    uint8_t mt, pbf, gid, oid, status;

    // init
    p = buf;

    // get event header
    NCI_MSG_PRS_HDR0(p, mt, pbf, gid);
    NCI_MSG_PRS_HDR1(p, oid);

    // process message type
    // FIXME: check event length = header + data length?
    switch(mt) {
        // response message
        case NCI_MT_RSP:
            switch (oid) {
                case NCI_MSG_RF_DISCOVER_MAP:
                    status = rspRfDiscoverMap(p);
                    break;
                case NCI_MSG_RF_DISCOVER:
                    status = rspRfDiscover(p);
                    break;
                case NCI_MSG_RF_DEACTIVATE:
                    status = rspRfDeactivate(p);
                    break;
                default:
                    _log.e("NCI error: unhandled rf event oid = %d\n", oid);
                    status = NCI_STATUS_UNKNOWN_OID;
                    break;
            }
            break;
        // notification message
        case NCI_MT_NTF:
            switch(oid) {
                case NCI_MSG_RF_INTF_ACTIVATED:
                    status = ntfRfIntfActivated(p);
                    break;
                case NCI_MSG_RF_DEACTIVATE:
                    status = ntfRfDeactivate(p);
                    break;
                default:
                    _log.e("NCI error: unhandled rf event oid = %d\n", oid);
                    status = NCI_STATUS_UNKNOWN_OID;
                    break;
            }
            break;
        default:
            _log.e("NCI error: unhandled rf event mt = %d\n", mt);
            status = NCI_STATUS_SYNTAX_ERROR;
            break;
    }

    return status;
}

uint8_t NfcNci::cmdCoreReset(pNCI_CB p_cb, uint8_t type)
{
    uint8_t *p, *buf;
    uint8_t status, ret;

    // _log NCI message
    _log.d("NCI_CMD: NCI_MSG_CORE_RESET\n");

    // check state, parameters
    if (_state != NCI_STATE_NONE) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }
    if (p_cb == NULL) {
        status = NCI_STATUS_INVALID_PARAM;
        goto end;
    }
    switch(type) {
        case NCI_RESET_TYPE_KEEP_CFG:
        case NCI_RESET_TYPE_RESET_CFG:
            break;
        default:
            status = NCI_STATUS_INVALID_PARAM;
            goto end;
    }

    // register callback
    registerCb(p_cb);

    // get RX buffer
    buf = getTxBuffer();
    p = buf;

    // format reset command
    NCI_MSG_BLD_HDR0(p, NCI_MT_CMD, NCI_GID_CORE);
    NCI_MSG_BLD_HDR1(p, NCI_MSG_CORE_RESET);
    UINT8_TO_STREAM(p, NCI_CORE_PARAM_SIZE_RESET);
    UINT8_TO_STREAM(p, type);

    // send command
    ret =_hw.write(buf, NCI_MSG_HDR_SIZE + NCI_CORE_PARAM_SIZE_RESET);
    status = ret == (NCI_MSG_HDR_SIZE + NCI_CORE_PARAM_SIZE_RESET) ? NCI_STATUS_OK : NCI_STATUS_FAILED;

end:
    return status;
}

uint8_t NfcNci::rspCoreReset(uint8_t buf[])
{
    uint8_t *p = buf;
    uint8_t len, status;

    // _log NCI message
    _log.d("NCI_RSP: NCI_MSG_CORE_RESET\n");

    // check state
    if (_state != NCI_STATE_NONE) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }

    // check length
    len = *p++;
    if (len != NCI_CORE_PARAM_SIZE_RESET_RSP) {
        status = NCI_STATUS_SYNTAX_ERROR;
        goto end;
    }

    // check command status
    status = *p++;
    if (status != NCI_STATUS_OK) {
        goto end;
    }

    // reset command response: NCI version | configuration 
    _reset.version = *p++;
    _reset.status = *p;
    _cb.data = (void *)&_reset;

    // set state
    _state = NCI_STATE_RFST_RESET;

end:
    return status;
}

uint8_t NfcNci::cmdCoreInit(pNCI_CB p_cb)
{
    uint8_t *p, *buf;
    uint8_t len, ret, status;

    // _log NCI message
    _log.d("NCI_CMD: NCI_MSG_CORE_INIT\n");

    // check state, parameters
    if (_state != NCI_STATE_RFST_RESET) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }
    if (p_cb == NULL) {
        status = NCI_STATUS_INVALID_PARAM;
        goto end;
    }

    // register callback
    registerCb(p_cb);

    // get TX buffer
    buf = getTxBuffer();
    p = buf;

    // format command
    len = NCI_MSG_HDR_SIZE + NCI_CORE_PARAM_SIZE_INIT;
    NCI_MSG_BLD_HDR0(p, NCI_MT_CMD, NCI_GID_CORE);
    NCI_MSG_BLD_HDR1(p, NCI_MSG_CORE_INIT);
    UINT8_TO_STREAM(p, NCI_CORE_PARAM_SIZE_INIT);
    
    // send command
    ret = _hw.write(buf, len);
    status = ret == len ? NCI_STATUS_OK : NCI_STATUS_FAILED;

end:
    return status;
}

uint8_t NfcNci::rspCoreInit(uint8_t buf[])
{
    uint8_t *p = buf;
    uint8_t len, status;

    // _log NCI message
    _log.d("NCI_RSP: NCI_MSG_CORE_INIT\n");

    // check state
    if (_state != NCI_STATE_RFST_RESET) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }

    // packet length
    len = *p++;
    if (len < NCI_CORE_PARAM_SIZE_INIT_RSP) {
        status = NCI_STATUS_SYNTAX_ERROR;
        goto end;
    }

    // check status
    status = *p;
    if (status != NCI_STATUS_OK) {
        goto end;
    }

    // FIXME: initialize init response structure
    // set state
    _state = NCI_STATE_RFST_IDLE;

end:
    return status;
}

uint8_t NfcNci::cmdRfDiscoverMap(pNCI_CB p_cb, uint8_t num, tNCI_DISCOVER_MAPS *p_maps)
{
    uint8_t *p, *buf, *p_size, *p_start;
    uint8_t len, status, ret;

    // _log NCI message
    _log.d("NCI_CMD: NCI_MSG_RF_DISCOVER_MAP\n");

    // check state, parameters
    if (_state != NCI_STATE_RFST_IDLE) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }
    if (p_cb == NULL || p_maps == NULL) {
        status = NCI_STATUS_INVALID_PARAM;
        goto end;
    }

    // register callback
    registerCb(p_cb);

    // get TX buffer
    buf = getTxBuffer();
    p = buf;

    // format command header
    NCI_MSG_BLD_HDR0(p, NCI_MT_CMD, NCI_GID_RF_MANAGE);
    NCI_MSG_BLD_HDR1(p, NCI_MSG_RF_DISCOVER_MAP);
    p_size = p;
    p++;
    p_start = p;

    // format command payload
    UINT8_TO_STREAM(p, num);
    while (num--) {
        UINT8_TO_STREAM(p, p_maps[num].protocol);
        UINT8_TO_STREAM(p, p_maps[num].mode);
        UINT8_TO_STREAM(p, p_maps[num].intf_type);
    }
    *p_size = (uint8_t)(p - p_start);
    len = NCI_MSG_HDR_SIZE + *p_size;

    // send command
    ret = _hw.write(buf, len);
    status = ret == len ? NCI_STATUS_OK : NCI_STATUS_FAILED;

end:
    return status;
}

uint8_t NfcNci::rspRfDiscoverMap(uint8_t buf[])
{
    uint8_t *p = buf;
    uint8_t len, status;

    // _log NCI message
    _log.d("NCI_RSP: NCI_MSG_RF_DISCOVER_MAP\n");

    // check state
    if (_state != NCI_STATE_RFST_IDLE) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }

    // packet length
    len = *p++;
    if (len != NCI_RF_PARAM_SIZE_DISCOVER_MAP_RSP) {
        status = NCI_STATUS_SYNTAX_ERROR;
        goto end;
    }

    // read status
    status = *p;

end:
    return status;
}

uint8_t NfcNci::cmdRfDiscover(pNCI_CB p_cb, uint8_t num, tNCI_DISCOVER_CONFS *p_confs)
{
    uint8_t *p, *buf, *p_size, *p_start;
    uint8_t len, status, ret;

    // _log NCI message
    _log.d("NCI_CMD: NCI_MSG_RF_DISCOVER\n");

    // check state, parameters
    if (_state != NCI_STATE_RFST_IDLE || p_cb == NULL) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }
    if (p_cb == NULL || num == 0 || p_confs == NULL) {
        status = NCI_STATUS_INVALID_PARAM;
        goto end;
    }

    // register callback
    registerCb(p_cb);

    // get TX buffer
    buf = getTxBuffer();
    p = buf;

    // format command header
    NCI_MSG_BLD_HDR0(p, NCI_MT_CMD, NCI_GID_RF_MANAGE);
    NCI_MSG_BLD_HDR1(p, NCI_MSG_RF_DISCOVER);
    p_size = p;
    p++;
    p_start = p;

    // format command payload
    UINT8_TO_STREAM(p, num);
    while (num--) {
        UINT8_TO_STREAM(p, p_confs[num].mode);
        UINT8_TO_STREAM(p, p_confs[num].freq);
    }
    *p_size = (uint8_t)(p - p_start);
    len = NCI_MSG_HDR_SIZE + *p_size;

    // send command
    ret = _hw.write(buf, len);
    status = ret == len ? NCI_STATUS_OK : NCI_STATUS_FAILED;

end:
    return status;
}

uint8_t NfcNci::rspRfDiscover(uint8_t buf[])
{
    uint8_t *p = buf;
    uint8_t len, status;

    // _log NCI message
    _log.d("NCI_RSP: NCI_MSG_RF_DISCOVER\n");

    // check state
    if (_state != NCI_STATE_RFST_IDLE) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }

    // packet length
    len = *p++;
    if (len != NCI_RF_PARAM_SIZE_DISCOVER_RSP) {
        status = NCI_STATUS_SYNTAX_ERROR;
        goto end;
    }

    // check status
    status = *p;
    if (status != NCI_STATUS_OK) {
        goto end;
    }

    // set state
    _state = NCI_STATE_RFST_DISCOVERY;

end:
    return status;
}

void setRfTechSpecParams(uint8_t buf[], tNCI_RF_INTF *p_rf)
{
    uint8_t mode, len;

    mode = p_rf->activation_mode;
    p_rf->specific.type = mode;

    switch(mode) {
        case NCI_DISCOVERY_TYPE_POLL_A:
        {
            tNCI_RF_PARAMS_PA *p_poll_a = &p_rf->specific.params.poll_a;
            p_poll_a->sens_res[0] = *buf++;
            p_poll_a->sens_res[1] = *buf++;
            len = *buf++;
            p_poll_a->nfcid_len = len;
            memcpy(p_poll_a->nfcid, buf, len);
            buf += len;
            p_poll_a->sel_res_len = *buf++;
            if (p_poll_a->sel_res_len != 0) {
                p_poll_a->sel_res = *buf;
            }
        }
            break;
        // FIXME: implement other modes
        default:
            break;
    }
}

uint8_t NfcNci::ntfRfIntfActivated(uint8_t buf[])
{
    uint8_t *p = buf;
    uint8_t status, len;

    // _log NCI message
    _log.d("NCI_NTF: NCI_MSG_RF_INTF_ACTIVATED\n");

    // check state
    if (_state != NCI_STATE_RFST_DISCOVERY) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }

    // check length
    len = *p++;
    if (len < NCI_RF_PARAM_SIZE_INTF_ACTIVATED_NTF) {
        status = NCI_STATUS_SYNTAX_ERROR;
        goto end;
    }

    // set RF interface
    _rf_intf.id = *p++;
    _rf_intf.interface = *p++;
    _rf_intf.protocol = *p++;
    _rf_intf.activation_mode = *p++;
    _rf_intf.max_payload_size = *p++;
    _rf_intf.credits = *p++;
    len = *p++;
    if (len != 0) {
        setRfTechSpecParams(p, &_rf_intf);
        p += len;
    }
    _rf_intf.exchange_mode = *p++;
    _rf_intf.tx_bitrate = *p++;
    _rf_intf.rx_bitrate = *p++;
    len = *p++;

    // FIXME: implement activation
    _cb.data = (void *)&_rf_intf;
 
    // set state
    _state = NCI_STATE_RFST_POLL_ACTIVE;

    // no error
    status = NCI_STATUS_OK;

end:
    return status;
}

uint8_t NfcNci::cmdRfDeactivate(pNCI_CB p_cb, uint8_t type)
{
    uint8_t *p, *buf;
    uint8_t status, ret;

    // _log NCI message
    _log.d("NCI_CMD: NCI_MSG_RF_DEACTIVATE\n");

    // check state
    if (_state != NCI_STATE_RFST_POLL_ACTIVE || p_cb == NULL) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }
    if (p_cb == NULL) {
        status = NCI_STATUS_INVALID_PARAM;
        goto end;
    }

    // register callback
    registerCb(p_cb);

    // get RX buffer
    buf = getTxBuffer();
    p = buf;

    // format reset command
    NCI_MSG_BLD_HDR0(p, NCI_MT_CMD, NCI_GID_RF_MANAGE);
    NCI_MSG_BLD_HDR1(p, NCI_MSG_RF_DEACTIVATE);
    UINT8_TO_STREAM(p, NCI_RF_PARAM_SIZE_DEACTIVATE);
    UINT8_TO_STREAM(p, type);

    // send command
    ret = _hw.write(buf, NCI_MSG_HDR_SIZE + NCI_RF_PARAM_SIZE_DEACTIVATE);
    status = ret == NCI_MSG_HDR_SIZE + NCI_RF_PARAM_SIZE_DEACTIVATE ? NCI_STATUS_OK : NCI_STATUS_FAILED;

end:
    return status;
}

uint8_t NfcNci::rspRfDeactivate(uint8_t buf[])
{
    uint8_t *p = buf;
    uint8_t len, status;

    // _log NCI message
    _log.d("NCI_RSP: NCI_MSG_RF_DEACTIVATE\n");

    // check state
    if (_state != NCI_STATE_RFST_POLL_ACTIVE) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }

    // check length
    len = *p++;
    if (len != NCI_RF_PARAM_SIZE_DEACTIVATE_RSP) {
        status = NCI_STATUS_SYNTAX_ERROR;
        goto end;
    }

    // check status
    status = *p;
    if (status != NCI_STATUS_OK) {
        goto end;
    }

    // set state
    _state = NCI_STATE_RFST_DISCOVERY;

end:
    return status;
}

uint8_t NfcNci::ntfRfDeactivate(uint8_t buf[])
{
    uint8_t *p = buf;
    uint8_t len, status;

    // _log NCI message
    _log.d("NCI_NTF: NCI_MSG_RF_DEACTIVATE\n");

    // check state
    if (_state != NCI_STATE_RFST_DISCOVERY) {
        status = NCI_STATUS_REJECTED;
        goto end;
    }

    // check length
    len = *p++;
    if (len != NCI_RF_PARAM_SIZE_DEACTIVATE_NTF) {
        status = NCI_STATUS_SYNTAX_ERROR;
        goto end;
    }

    // set deactivation data
    _deactivate.type = *p++;
    _deactivate.reason = *p;
    _cb.data = (void *)&_deactivate;

    // no error
    status = NCI_STATUS_OK;

end:
    return status;
}
