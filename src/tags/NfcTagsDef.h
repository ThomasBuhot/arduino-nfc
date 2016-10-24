/*
 * NfcTagsDef.h
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

#ifndef __NFC_TAGS_DEF_H__
#define __NFC_TAGS_DEF_H__

// status definition
enum {
    TAGS_STATUS_OK = 0,
    TAGS_STATUS_REJECTED,
    TAGS_STATUS_FAILED,
    TAGS_STATUS_MESSAGE_CORRUPTED
};

// tag type definition
enum {
    TAGS_TYPE_1 = 1,
    TAGS_TYPE_2,
    TAGS_TYPE_3,
    TAGS_TYPE_4,
    TAGS_TYPE_MIFARE
};

// NCI response type definition
typedef struct {
    void *data;
    uint16_t id;
    uint8_t status;
} tTAGS_NCI_RSP;

// NCI response type definition
typedef struct {
    uint8_t *buf;
    uint8_t len;
    uint8_t more;
} tTAGS_DUMP;

// Interface identifier
enum {
    TAGS_ID_NONE = 0,
    TAGS_ID_RESET,
    TAGS_ID_DISCOVER,
    TAGS_ID_DISCOVER_ACTIVATED,
    TAGS_ID_DEACTIVATE,
    TAGS_ID_DUMP
};

#endif // __NFC_TAGS_DEF_H__
