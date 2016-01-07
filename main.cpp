/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include <string>
#include "BLEDevice.h"
#include "URIBeaconConfigService.h"
#include "DFUService.h"
#include "DeviceInformationService.h"
#include "ConfigParamsPersistence.h"

BLEDevice ble;
URIBeaconConfigService *uriBeaconConfig;

/**
 * URIBeaconConfig service can operate in two modes: a configuration mode which
 * allows a user to update settings over a connection; and normal URIBeacon mode
 * which involves advertising a URI. Constructing an object from URIBeaconConfig
 * service sets up advertisements for the configuration mode. It is then up to
 * the application to switch to URIBeacon mode based on some timeout.
 *
 * The following help with this switch.
 */
static const int CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS = 5;  // Duration after power-on that config service is available.
Ticker configAdvertisementTimeoutTicker;

/**
 * Stop advertising the UriBeaconConfig Service after a delay; and switch to normal URIBeacon.
 */
void timeout(void)
{
    Gap::GapState_t state;
    state = ble.getGapState();
    if (!state.connected) { /* don't switch if we're in a connected state. */
        uriBeaconConfig->setupURIBeaconAdvertisements();
        ble.startAdvertising();

        configAdvertisementTimeoutTicker.detach(); /* disable the callback from the timeout Ticker. */
    }
}

/**
 * Callback triggered upon a disconnection event. Needs to re-enable advertisements.
 */
void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason)
{
    ble.startAdvertising();
}

int main(void)
{
    ble.init();
    ble.onDisconnection(disconnectionCallback);

    uint8_t mac[6] = {};
    ble.getAddress(0,mac);

    printf("MAC Address = ");
    for(int x =0; x<7; x++) {
        printf("%x",mac[x]);
    }
    printf("\n\r");

//    string url = "http://snurl.com/ub?q=" ;
    string url = "http://snurl.com/u6?q=" ;
    url += ((mac[0] & 0xF0) >> 4) + 'A';
    url += ((mac[0] & 0x0F) >> 0) + 'A';
    url += ((mac[1] & 0xF0) >> 4) + 'A';
    url += ((mac[1] & 0x0F) >> 0) + 'A';
    url += ((mac[2] & 0xF0) >> 4) + 'A';
    url += ((mac[2] & 0x0F) >> 0) + 'A';
    url += ((mac[3] & 0xF0) >> 4) + 'A';
    url += ((mac[3] & 0x0F) >> 0) + 'A';
    url += ((mac[4] & 0xF0) >> 4) + 'A';
    url += ((mac[4] & 0x0F) >> 0) + 'A';
    
    for(int y=0; y<6; y++) {
        printf("\n\rmac[%d]=0x%x, d%d, c%c,modH=%c, modL=%c",y,mac[y],mac[y],mac[y],((mac[y] & 0xF0) >> 4) + 'A',((mac[y] & 0x0F) >> 0) + 'A');
    }
//    printf("\n\r0x%x->%c, 0x%x->%c\n\r%s\n\r",mac[4],(mac[4]%(122-32))+32,mac[5],(mac[5]%(122-32))+32,url);
    printf("\n\rurl = %s\n\r",url);

    /*
     * Load parameters from (platform specific) persistent storage. Parameters
     * can be set to non-default values while the URIBeacon is in configuration
     * mode (within the first 60 seconds of power-up). Thereafter, parameters
     * get copied out to persistent storage before switching to normal URIBeacon
     * operation.
     */
    URIBeaconConfigService::Params_t params;
    bool fetchedFromPersistentStorage = loadURIBeaconConfigParams(&params);

    /* Initialize a URIBeaconConfig service providing config params, default URI, and power levels. */
    static URIBeaconConfigService::PowerLevels_t defaultAdvPowerLevels = {-20, -4, 0, 10}; // Values for ADV packets related to firmware levels
    uriBeaconConfig = new URIBeaconConfigService(ble, params, !fetchedFromPersistentStorage, (const char*)&url[0], defaultAdvPowerLevels);
    if (!uriBeaconConfig->configuredSuccessfully()) {
        error("failed to accommodate URI");
    }
    configAdvertisementTimeoutTicker.attach(timeout, CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS);

    // Setup auxiliary services to allow over-the-air firmware updates, etc
    DFUService dfu(ble);
    DeviceInformationService deviceInfo(ble, "ARM", "UriBeacon", "SN1", "hw-rev1", "fw-rev1", "soft-rev1");

    ble.startAdvertising(); /* Set the whole thing in motion. After this call a GAP central can scan the URIBeaconConfig
                             * service. This can then be switched to the normal URIBeacon functionality after a timeout. */


    while (true) {
        ble.waitForEvent();
    }
}
