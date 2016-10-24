/*
 * NfcNci.h
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
 *
 * The NCI names and definitions in this file are partially derived
 * from nci_defs.h used in Android whose copyright is copied below.
 *
 * Copyright (C) 1999-2014 Broadcom Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __NFC_NCI_H__
#define __NFC_NCI_H__

#include <Arduino.h>
#include <functional>
#include "log/NfcLog.h"
#include "hw/NfcHw.h"

/* NCI packet size */
#define NCI_PACKET_SIZE     258
#define NCI_MSG_HDR_SIZE    3   /* per NCI spec */

/* NCI length field offset */
#define NCI_OFFSET_LEN      2

/* NCI Command and Notification Format:
 * 3 byte message header:
 * byte 0: MT PBF GID
 * byte 1: OID
 * byte 2: Message Length */
/* MT: Message Type (byte 0) */
#define NCI_MT_MASK         0xE0
#define NCI_MT_SHIFT        5
#define NCI_MT_DATA         0x00
#define NCI_MT_CMD          1   /* (NCI_MT_CMD << NCI_MT_SHIFT) = 0x20 */
#define NCI_MT_RSP          2   /* (NCI_MT_RSP << NCI_MT_SHIFT) = 0x40 */
#define NCI_MT_NTF          3   /* (NCI_MT_NTF << NCI_MT_SHIFT) = 0x60 */
#define NCI_MT_CFG          4   /* (NCI_MT_CFG << NCI_MT_SHIFT) = 0x80 */

/* PBF: Packet Boundary Flag (byte 0) */
#define NCI_PBF_MASK        0x10
#define NCI_PBF_SHIFT       4
#define NCI_PBF_NO_OR_LAST  0x00    /* not fragmented or last fragment */
#define NCI_PBF_ST_CONT     0x10    /* start or continuing fragment */

/* GID: Group Identifier (byte 0) */
#define NCI_GID_MASK        0x0F
#define NCI_GID_SHIFT       0
#define NCI_GID_CORE        0x00    /* 0000b NCI Core group */
#define NCI_GID_RF_MANAGE   0x01    /* 0001b RF Management group */
#define NCI_GID_EE_MANAGE   0x02    /* 0010b NFCEE Management group */
#define NCI_GID_PROP        0x0F    /* 1111b Proprietary */
/* 0111b - 1110b RFU */

/* OID: Opcode Identifier (byte 1) */
#define NCI_OID_MASK        0x3F
#define NCI_OID_SHIFT       0

/* NCI Data Format:
 * byte 0: MT(0) PBF CID
 * byte 1: RFU
 * byte 2: Data Length */
/* CID: Connection Identifier (byte 0) 1-0xF Dynamically assigned (by NFCC), 0 is predefined  */
#define NCI_CID_MASK        0x0F
#define NCI_CID_RF_STATIC   0x00

/* builds byte0 of NCI Command and Notification packet */
#define NCI_MSG_BLD_HDR0(p, mt, gid) \
    *(p)++ = (uint8_t) (((mt) << NCI_MT_SHIFT) | (gid));

/* builds byte1 of NCI Command and Notification packet */
#define NCI_MSG_BLD_HDR1(p, oid) \
    *(p)++ = (uint8_t) (((oid) << NCI_OID_SHIFT));

/* parse byte0 of NCI packet */
#define NCI_MSG_PRS_HDR0(p, mt, pbf, gid) \
    mt = (*(p) & NCI_MT_MASK) >> NCI_MT_SHIFT; \
    pbf = (*(p) & NCI_PBF_MASK) >> NCI_PBF_SHIFT; \
    gid = *(p)++ & NCI_GID_MASK;

/* parse MT and PBF bits of NCI packet */
#define NCI_MSG_PRS_MT_PBF(p, mt, pbf) \
    mt = (*(p) & NCI_MT_MASK) >> NCI_MT_SHIFT; \
    pbf = (*(p) & NCI_PBF_MASK) >> NCI_PBF_SHIFT;

/* parse byte1 of NCI Cmd/Ntf */
#define NCI_MSG_PRS_HDR1(p, oid) \
    oid = (*(p) & NCI_OID_MASK); (p)++;

/* builds 3-byte message header of NCI Data packet */
#define NCI_DATA_BLD_HDR(p, cid, len) \
    *(p)++ = (uint8_t) (cid); *(p)++ = 0; *(p)++ = (uint8_t) (len);

#define NCI_DATA_PBLD_HDR(p, pbf, cid, len) \
    *(p)++ = (uint8_t) (((pbf) << NCI_PBF_SHIFT) | (cid)); *(p)++=0; *(p)++ = (len);

#define NCI_DATA_PRS_HDR(p, pbf, cid, len) \
    (pbf) = (*(p) & NCI_PBF_MASK) >> NCI_PBF_SHIFT; (cid) = (*(p) & NCI_CID_MASK); p++; p++; (len) = *(p)++;

#define UINT8_TO_STREAM(p, u8) \
    *(p)++ = (uint8_t)(u8);

/* build 2 bytes with GID / OID */
#define UINT16_ID(mt, oid) \
    ((uint16_t)((mt) << (8 + NCI_MT_SHIFT) | (oid)) & \
     (uint16_t)(NCI_MT_MASK << 8 | NCI_OID_MASK))

/* Status Codes */
#define NCI_STATUS_OK                   0x00
#define NCI_STATUS_REJECTED             0x01
#define NCI_STATUS_MESSAGE_CORRUPTED    0x02
#define NCI_STATUS_BUFFER_FULL          0xE0
#define NCI_STATUS_FAILED               0x03
#define NCI_STATUS_NOT_INITIALIZED      0x04
#define NCI_STATUS_SYNTAX_ERROR         0x05
#define NCI_STATUS_SEMANTIC_ERROR       0x06
#define NCI_STATUS_UNKNOWN_GID          0x07
#define NCI_STATUS_UNKNOWN_OID          0x08
#define NCI_STATUS_INVALID_PARAM        0x09
#define NCI_STATUS_MSG_SIZE_TOO_BIG     0x0A
/* discovery */
#define NCI_STATUS_ALREADY_STARTED      0xA0
#define NCI_STATUS_ACTIVATION_FAILED    0xA1
#define NCI_STATUS_TEAR_DOWN            0xA2
/* RF Interface */
#define NCI_STATUS_RF_TRANSMISSION_ERR  0xB0
#define NCI_STATUS_RF_PROTOCOL_ERR      0xB1
#define NCI_STATUS_TIMEOUT              0xB2
/* NFCEE Interface */
#define NCI_STATUS_EE_INTF_ACTIVE_FAIL  0xC0
#define NCI_STATUS_EE_TRANSMISSION_ERR  0xC1
#define NCI_STATUS_EE_PROTOCOL_ERR      0xC2
#define NCI_STATUS_EE_TIMEOUT           0xC3

/* NCI Core Group Opcode - 0 */
#define NCI_MSG_CORE_RESET              0
#define NCI_MSG_CORE_INIT               1
#define NCI_MSG_CORE_SET_CONFIG         2
#define NCI_MSG_CORE_GET_CONFIG         3
#define NCI_MSG_CORE_CONN_CREATE        4
#define NCI_MSG_CORE_CONN_CLOSE         5
#define NCI_MSG_CORE_CONN_CREDITS       6
#define NCI_MSG_CORE_GEN_ERR_STATUS     7
#define NCI_MSG_CORE_INTF_ERR_STATUS    8

/* RF MANAGEMENT Group Opcode - 1 */
#define NCI_MSG_RF_DISCOVER_MAP         0
#define NCI_MSG_RF_DISCOVER             3
#define NCI_MSG_RF_DISCOVER_SELECT      4
#define NCI_MSG_RF_INTF_ACTIVATED       5
#define NCI_MSG_RF_DEACTIVATE           6
#define NCI_MSG_RF_FIELD                7
#define NCI_MSG_RF_T3T_POLLING          8
#define NCI_MSG_RF_EE_ACTION            9
#define NCI_MSG_RF_EE_DISCOVERY_REQ     10
#define NCI_MSG_RF_PARAMETER_UPDATE     11

/* Response and notification IDs */
#define NCI_ID_RSP_CORE_RESET           UINT16_ID(NCI_MT_RSP, NCI_MSG_CORE_RESET)
#define NCI_ID_RSP_CORE_INIT            UINT16_ID(NCI_MT_RSP, NCI_MSG_CORE_INIT)
#define NCI_ID_RSP_RF_DISCOVER_MAP      UINT16_ID(NCI_MT_RSP, NCI_MSG_RF_DISCOVER_MAP)
#define NCI_ID_RSP_RF_DISCOVER          UINT16_ID(NCI_MT_RSP, NCI_MSG_RF_DISCOVER)
#define NCI_ID_NTF_RF_INTF_ACTIVATED    UINT16_ID(NCI_MT_NTF, NCI_MSG_RF_INTF_ACTIVATED)
#define NCI_ID_RSP_RF_DEACTIVATE        UINT16_ID(NCI_MT_RSP, NCI_MSG_RF_DEACTIVATE)
#define NCI_ID_NTF_RF_DEACTIVATE        UINT16_ID(NCI_MT_NTF, NCI_MSG_RF_DEACTIVATE)

/* NCI CORE_RESET_CMD */
#define NCI_CORE_PARAM_SIZE_RESET       0x01
#define NCI_CORE_PARAM_SIZE_RESET_RSP   0x03
#define NCI_CORE_PARAM_SIZE_RESET_NTF   0x02
#define NCI_RESET_TYPE_KEEP_CFG         0x00  /* Keep the NCI configuration (if possible) and perform NCI initialization. */
#define NCI_RESET_TYPE_RESET_CFG        0x01  /* Reset the NCI configuration, and perform NCI initialization. */

/* NCI CORE_INIT_CMD */
#define NCI_CORE_PARAM_SIZE_INIT            0x00 /* no payload */
#define NCI_CORE_INIT_RSP_OFFSET_NUM_INTF   0x05
#define NCI_CORE_PARAM_SIZE_INIT_RSP        0x11

/* NCI RF_DISCOVER_MAP_CMD */
#define NCI_RF_PARAM_SIZE_DISCOVER_MAP_RSP  0x01

/* NCI RF_DISCOVER_CMD */
#define NCI_RF_PARAM_SIZE_DISCOVER_RSP  0x01

/* NCI RF_INTF_ACTIVATED_NTF */
#define NCI_RF_PARAM_SIZE_INTF_ACTIVATED_NTF    0x0B

/* NCI RF_DEACTIVATE_CMD */
#define NCI_RF_PARAM_SIZE_DEACTIVATE        0x01
#define NCI_RF_PARAM_SIZE_DEACTIVATE_RSP    0x01
#define NCI_RF_PARAM_SIZE_DEACTIVATE_NTF    0x02

/* Supported Protocols */
#define NCI_PROTOCOL_UNKNOWN            0x00
#define NCI_PROTOCOL_T1T                0x01
#define NCI_PROTOCOL_T2T                0x02
#define NCI_PROTOCOL_T3T                0x03
#define NCI_PROTOCOL_ISO_DEP            0x04
#define NCI_PROTOCOL_NFC_DEP            0x05

/* Discovery Types/Detected Technology and Mode */
#define NCI_DISCOVERY_TYPE_POLL_A               0x00
#define NCI_DISCOVERY_TYPE_POLL_B               0x01
#define NCI_DISCOVERY_TYPE_POLL_F               0x02
#define NCI_DISCOVERY_TYPE_POLL_A_ACTIVE        0x03
#define NCI_DISCOVERY_TYPE_POLL_F_ACTIVE        0x05
#define NCI_DISCOVERY_TYPE_LISTEN_A             0x80
#define NCI_DISCOVERY_TYPE_LISTEN_B             0x81
#define NCI_DISCOVERY_TYPE_LISTEN_F             0x82
#define NCI_DISCOVERY_TYPE_LISTEN_A_ACTIVE      0x83
#define NCI_DISCOVERY_TYPE_LISTEN_F_ACTIVE      0x85
#define NCI_DISCOVERY_TYPE_POLL_ISO15693        0x06
#define NCI_DISCOVERY_TYPE_LISTEN_ISO15693      0x86
#define NCI_DISCOVERY_TYPE_MAX  NCI_DISCOVERY_TYPE_LISTEN_ISO15693

/* Discovery frequency */
#define NCI_DISCOVERY_FREQUENCY_ALWAYS          0x01

/* NCI Deactivation Type */
#define NCI_DEACTIVATE_TYPE_IDLE        0   /* Idle Mode     */
#define NCI_DEACTIVATE_TYPE_SLEEP       1   /* Sleep Mode    */
#define NCI_DEACTIVATE_TYPE_SLEEP_AF    2   /* Sleep_AF Mode */
#define NCI_DEACTIVATE_TYPE_DISCOVERY   3   /* Discovery     */

typedef struct
{
    uint8_t mode;
    uint8_t freq;
} tNCI_DISCOVER_CONFS;

/* NCI Interface Types */
#define NCI_INTERFACE_EE_DIRECT_RF      0
#define NCI_INTERFACE_FRAME             1
#define NCI_INTERFACE_ISO_DEP           2
#define NCI_INTERFACE_NFC_DEP           3
#define NCI_INTERFACE_MAX               NCI_INTERFACE_NFC_DEP
#define NCI_INTERFACE_FIRST_VS          0x80
typedef uint8_t tNCI_INTF_TYPE;

/* NCI Interface Mode */
#define NCI_INTERFACE_MODE_POLL             1
#define NCI_INTERFACE_MODE_LISTEN           2
#define NCI_INTERFACE_MODE_POLL_N_LISTEN    3

typedef struct
{
    uint8_t version;
    uint8_t status;
} tNCI_RESET;

typedef struct
{
    uint8_t type;
    uint8_t reason;
} tNCI_DEACTIVATE;

typedef struct
{
    uint8_t protocol;
    uint8_t mode;
    uint8_t intf_type;
} tNCI_DISCOVER_MAPS;

#define NCI_RF_PA_SENS_RES_LENGTH  2
#define NCI_RF_PA_NFCID_LENGTH     10

typedef struct
{
    uint8_t sens_res[NCI_RF_PA_SENS_RES_LENGTH];
    uint8_t nfcid_len;
    uint8_t nfcid[NCI_RF_PA_NFCID_LENGTH];
    uint8_t sel_res_len;
    uint8_t sel_res;
} tNCI_RF_PARAMS_PA;

typedef struct
{
    uint8_t type;
    union
    {
        tNCI_RF_PARAMS_PA poll_a;
    } params;
} tNCI_RF_PARAMS;

typedef struct
{
    uint8_t type;
    void    *params = NULL; // FIXME: to be defined, not supported now
} tNCI_ACT_PARAMS;

typedef struct
{
    uint8_t id;
    uint8_t interface;
    uint8_t protocol;
    uint8_t activation_mode;
    uint8_t max_payload_size;
    uint8_t credits;
    tNCI_RF_PARAMS specific;
    uint8_t exchange_mode;
    uint8_t tx_bitrate;
    uint8_t rx_bitrate;
    tNCI_ACT_PARAMS activation;
} tNCI_RF_INTF;

enum
{
    NCI_STATE_NONE = 0,
    NCI_STATE_RFST_RESET,
    NCI_STATE_RFST_IDLE,
    NCI_STATE_RFST_DISCOVERY,
    NCI_STATE_RFST_W4_ALL_DISCOVERIES,
    NCI_STATE_RFST_POLL_ACTIVE,
    NCI_STATE_RFST_W4_HOST_SELECT,
    NCI_STATE_RFST_LISTEN_SLEEP,
    NCI_STATE_RFST_LISTEN_ACTIVE
};
typedef uint8_t tNFC_STATE;

// Callback object that clients have to implement
// to be notified on response or event
class NfcNciCb 
{
    public:
        NfcNciCb(void) {;}
        virtual void cbCoreReset(uint8_t status, uint16_t id, void *data) = 0;  
        virtual void cbCoreInit(uint8_t status, uint16_t id, void *data) = 0;  
        virtual void cbRfDiscoverMap(uint8_t status, uint16_t id, void *data) = 0;  
        virtual void cbRfDiscover(uint8_t status, uint16_t id, void *data) = 0;  
        virtual void cbRfDiscoverNtf(uint8_t status, uint16_t id, void *data) = 0;  
        virtual void cbRfDeactivate(uint8_t status, uint16_t id, void *data) = 0;  
        virtual void cbRfDeactivateNtf(uint8_t status, uint16_t id, void *data) = 0;
        virtual void cbData(uint8_t status, uint16_t id, void *data) = 0;
        virtual void cbError(uint8_t status, uint16_t id, void *data) = 0;  
};

// NCI API definition which implements the NCI interface
// as defined by the NFC Forum to drive NFC controller
class NfcNci
{
    public:
        NfcNci(NfcLog& log, NfcHw& hw);
        void init(NfcNciCb *cb) {_cb = cb;}
        void handleEvent(void);
        uint8_t cmdCoreReset(uint8_t type);
        uint8_t cmdCoreInit(void);
        uint8_t cmdRfDiscoverMap(uint8_t num, tNCI_DISCOVER_MAPS* p_maps);
        uint8_t cmdRfDiscover(uint8_t num, tNCI_DISCOVER_CONFS* p_confs);
        uint8_t cmdRfDeactivate(uint8_t type);
        uint8_t dataSend(uint8_t cid, uint8_t buf[], uint32_t len);

    private:
        uint32_t waitForEvent(uint8_t buf[]);
        void handleDataEvent(uint8_t buf[], uint32_t len);
        void handleCoreEvent(uint8_t buf[], uint32_t len);
        void handleRfEvent(uint8_t buf[], uint32_t len);
        uint8_t rspCoreReset(uint8_t buf[]);
        uint8_t rspCoreInit(uint8_t buf[]);
        uint8_t rspRfDiscoverMap(uint8_t buf[]);
        uint8_t rspRfDiscover(uint8_t buf[]);
        uint8_t ntfRfIntfActivated(uint8_t buf[]);
        uint8_t rspRfDeactivate(uint8_t buf[]);
        uint8_t ntfRfDeactivate(uint8_t buf[]);

    private:
        uint8_t _rx_buf[NCI_PACKET_SIZE];
        uint8_t _tx_buf[NCI_PACKET_SIZE];
        tNFC_STATE _state;
        NfcLog& _log;
        NfcHw& _hw;
        NfcNciCb *_cb;
        void *_data;
        tNCI_RESET _reset;              // reset response
        tNCI_RF_INTF _rf_intf;          // RF interface
        tNCI_DEACTIVATE _deactivate;    // deactivate
};

#endif /* __NFC_NCI_H__ */
