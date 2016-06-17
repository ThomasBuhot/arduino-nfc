/*
 * TagDetect.ino
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

/**********************************************
 *           Purpose of this sketch
 *
 * The purpose of this sketch is to provide
 * an example of NFC tag detection with an
 * NFC conftoller driven by the NFC NCI
 * (NFC Controller Interface) as defined by
 * the NFC Forum.
 *
 * It configures the NFC Controller to
 * detect tag of types 1, 2 or 3 as per
 * the NFC Forum specifications.
 *
 * This sketch:
 * 1. initializes the NFC controller
 * 2. configures the RF discovery parameters
 * 3. prints NFCID of the detected tag
 * 4. restart the tag detection back to step 2
 *
 * The HW configuration used to test that sketch
 * is: Intel Arduino 101 with NXP PN7120 SBC kit.
 **********************************************/

#include <Nfc.h>

/**********************************************
 *      NFC controller hardware configuration
 *
 * - NXP PN7120 NFC chipset
 * - Connected with I2C + IRQ + RESET
 *********************************************/

#define PN7120_IRQ          2  // pin 2 configured as input for IRQ
#define PN7120_RESET        4  // pin 4 configured as input for VEN (reset)
#define PN7120_I2C_ADDRESS  40 // 0x28

/**********************************************
 *           NCI RF configuration
 *
 * Discover tag type 1, 2 and 3 in polling mode
 * with frame interface.
 * Poll tag detection RF mode A and F.
 **********************************************/

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

/**********************************************
 *           Sketch state machine
 *********************************************/

enum
{
    STATE_RESET = 0,
    STATE_INIT,
    STATE_DISCOVER_MAP,
    STATE_DISCOVER,
    STATE_DISCOVERING,
    STATE_CONNECTED,
    STATE_ERROR,
    STATE_END
};

const char *stateToStr [] = {
    "STATE_RESET",
    "STATE_INIT",
    "STATE_DISCOVER_MAP",
    "STATE_DISCOVER",
    "STATE_DISCOVERING",
    "STATE_CONNECTED",
    "STATE_ERROR",
    "STATE_END"
};

/**********************************************
 *           Initialization
 *
 * _log: logger (serial)
 * _pn7120: NXP PN7120 NFC chipset
 * _nci: NFC Connection Interface (NFC Forum)
 * _state: internal state
 * _nci_cb: NCI callback object
 **********************************************/

NfcLog _log(NFC_LOG_LEVEL_INFO);
NfcHw_pn7120 _pn7120(_log, PN7120_IRQ, PN7120_RESET, PN7120_I2C_ADDRESS);
NfcNci _nci(_log, _pn7120);
uint8_t _state;

/**********************************************
 *           State machine callbacks
 *
 * Each callback is called upon NFC controller
 * response or event received.
 * The callbacks are used to:
 * - check the reponse / event status and data,
 * - change the current state accordingly.
 **********************************************/

void cbCoreReset(uint8_t status, uint16_t id, void *data)
{
    _log.d("TagDetect: %s status = %d id = %d\n", __func__, status, id);

    if (status != NCI_STATUS_OK || id != NCI_ID_RSP_CORE_RESET) {
        _state = STATE_ERROR;
    }
    else {
        _log.i("TagDetect: NFC controller reseted\n");
        _state = STATE_INIT;
    }
}

void cbCoreInit(uint8_t status, uint16_t id, void *data)
{
    _log.d("TagDetect: %s status = %d id = %d\n", __func__, status, id);

    if (status != NCI_STATUS_OK || id != NCI_ID_RSP_CORE_INIT) {
        _state = STATE_ERROR;
    }
    else {
        _log.i("TagDetect: NFC controller initialized\n");
        _state = STATE_DISCOVER_MAP;
    }
}

void cbRfDiscoverMap(uint8_t status, uint16_t id, void *data)
{
    _log.d("TagDetect: %s status = %d id = %d\n", __func__, status, id);

    if (status != NCI_STATUS_OK || id != NCI_ID_RSP_RF_DISCOVER_MAP) {
        _state = STATE_ERROR;
    }
    else {
        _log.i("TagDetect: RF polling mode configured\n");
        _state = STATE_DISCOVER;
    }
}

void cbRfDiscover(uint8_t status, uint16_t id, void *data)
{
    _log.d("TagDetect: %s status = %d id = %d\n", __func__, status, id);

    if (status != NCI_STATUS_OK || id != NCI_ID_RSP_RF_DISCOVER) {
        _state = STATE_ERROR;
    }
    else {
        _log.i("TagDetect: tag detection started\n");
        _state = STATE_DISCOVERING;
    }
}

void cbRfDiscovering(uint8_t status, uint16_t id, void *data)
{
    _log.d("TagDetect: %s status = %d id = %d\n", __func__, status, id);

    if (status != NCI_STATUS_OK) {
        _state = STATE_ERROR;
        return;
    }

    // handle notifications
    switch (id) {
        case NCI_ID_NTF_RF_INTF_ACTIVATED:
            // tag detected
            _log.i("TagDetect: tag detected\n");
            // FIXME: also print for type F
            if (data != NULL && ((tNCI_RF_INTF *)data)->specific.type == NCI_DISCOVERY_TYPE_POLL_A) {
                _log.bi("TagDetect: tag NFCID = ", ((tNCI_RF_INTF *)data)->specific.params.poll_a.nfcid, ((tNCI_RF_INTF *)data)->specific.params.poll_a.nfcid_len);
            }
            _state = STATE_CONNECTED;
            break;
        case NCI_ID_NTF_RF_DEACTIVATE:
            // RF polling enabled
            _log.i("TagDetect: tag detection ready, present tag....\n");
            _state = STATE_DISCOVERING;
            break;
        default:
            _state = STATE_ERROR;
            break;
    }
}

void cbRfDeactivate(uint8_t status, uint16_t id, void *data)
{
    _log.d("TagDetect: %s status = %d id = %d\n", __func__, status, id);

    if (status != NCI_STATUS_OK || id != NCI_ID_RSP_RF_DEACTIVATE) {
        _state = STATE_ERROR;
    }
    else {
        _state = STATE_DISCOVERING;
    }
}

/**********************************************
 *           State machine main loop
 *
 * Sends commands to the NFC controller based
 * on current state.
 * The state machine implementation is aligned
 * on NFC Forum NCI standard.
 **********************************************/

void handleEvent(void)
{
    uint8_t status = NCI_STATUS_FAILED;

    _log.d("TagDetect: state = %s\n", stateToStr[_state]);

    switch(_state) {
        case STATE_RESET:
            // send reset command
            status = _nci.cmdCoreReset(cbCoreReset, NCI_RESET_TYPE_KEEP_CFG);
            break;
        case STATE_INIT:
            // send init command
            status = _nci.cmdCoreInit(cbCoreInit);
            break;
        case STATE_DISCOVER_MAP:
            // send discover map command
            status = _nci.cmdRfDiscoverMap(cbRfDiscoverMap, sizeof(discover_maps)/sizeof(tNCI_DISCOVER_MAPS), discover_maps);
            break;
        case STATE_DISCOVER:
            // send discover command to activate polling
            status = _nci.cmdRfDiscover(cbRfDiscover, sizeof(discover_confs)/sizeof(tNCI_DISCOVER_CONFS), discover_confs);
            break;
        case STATE_DISCOVERING:
            // do nothing, wait for a tag to be detected
            _nci.registerCb(cbRfDiscovering);
            status = NCI_STATUS_OK;
            break;
        case STATE_CONNECTED:
            // disconnect from tag and restart polling
            status = _nci.cmdRfDeactivate(cbRfDeactivate, NCI_DEACTIVATE_TYPE_DISCOVERY);
            break;
        case STATE_ERROR:
        case STATE_END:
        default:
            break;
    }

    // handle error
    if (status != NCI_STATUS_OK) {
        _state = STATE_ERROR;
    }
}

// the setup function runs once when you press reset or power the board
void setup(void)
{
    // add a delay for the serial bus to be mounted
    delay(2000);

    // init logger, hw, nci, and state machine
    _log.init(230400);
    _pn7120.init();
    _nci.init();
    _state = STATE_RESET;
}

// the loop function runs over and over again forever
void loop(void)
{
    // handle sketch events (state machine based)
    handleEvent();

    // handle NCI events (state machine based),
    // it may block waiting for NFC controller
    // response or event
    _nci.handleEvent();
}

