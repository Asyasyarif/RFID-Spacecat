/*
    Copyright (c) 2018 Arif. All right reserved.

    RFID-SPACECAT - Library to create your own RFID Reader
    ESP8266 or ESP32 on Arduino IDE.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
#define lib_version 1.0

#ifndef _Spacecat_h_
#define _Spacecat_h_

#include "Arduino.h"
#include <SPI.h>
//using the latest version 6.xx
#include <ArduinoJson.h>
//get from https://github.com/miguelbalboa/rfid
#include <MFRC522.h>
#include <memory>
//Uncomment ENABLE_DEBUG for enable debuging
// #define DEBUG_D

// Get Access User RFID
// #define DEBUG_PATH_REQUEST_ACCESS
// #define DEBUG_HTTP_CODE_REQUEST_ACCESS
// #define DEBUG_PAYLOAD_REQUEST_ACCESS

//Intialization Device at startup
// #define DEBUG_PATH_DEVICE_INIT
// #define DEBUG_HTTP_CODE_DEVICE_INIT
// #define DEBUG_PAYLOAD_DEVICE_INIT

//Password Validation
// #define DEBUG_PATH_PASSWORD
// #define DEBUG_HTTP_CODE_PASSWORD
// #define DEBUG_PAYLOAD_PASSWORD

#if defined(ARDUINO_ARCH_ESP8266)
    #include <Hash.h>
    #include <ESP8266WiFi.h>
	#include <ESP8266HTTPClient.h>
    #define BOARD_ESP8266
extern "C" {
#include "Esp.h"
}

#elif defined(ARDUINO_ARCH_ESP32)
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <Hash.h>
    #define BOARD_ESP32
extern "C" {
    #include "Esp.h"
}
#endif

//uncomnet USE_HTTPS if you want use http://
#define USE_HTTPS
#ifndef  USE_HTTPS
    #define _protocol  "http://"
#else
    #define _protocol "https://"
    
#endif

#define HOST "rfid.asyarif.net"
#define PORT 80
#define PROJECT_N_ACTIVE        101
#define USER_N_ACTIVE           102
#define PLACEMENT_N_ACTIVE      103
#define SUCCEED                 104
#define UNSUCCESSFULL           105
#define ERR_INVALID_PASSOWORD   106
#define FILL_THE_PASSWORD       107
#define REACH_THE_LIMIT         108
#define SUCCESS                 200
#define ERR_NOT_FOUND           404

class Spacecat{

private:
    bool _debug = false;
    bool _isreadCard = false;
    bool _isSuccess = false;
    bool _isHavePassword = false;
    int _httpCode;
    unsigned int _port = PORT;
    uint8_t _sdaPin;
    uint8_t _resetPin;
    #define _INTERVAL_READING_CARD 3000
    #define _INTERVAL_INSERT_PASSWORD 5000
    unsigned long _TIMER_READING_CARD = 0;
    unsigned long _TIMER_WAITING_INSERT_PASSWORD = 0;
    String _content, _payload;
    String _USERNAME_KEY;
    String _PASSWORD_KEY;
    String _DEVICE_NAME;
    String _DEVICE_TYPE = "RFID-READER";
    String getLocalIP();
    String getMacAddress();
    String getWiFiSSID();
    uint8_t getChipID();
    String getCoreVersion();
    uint8_t getBootVersion();
    uint8_t getBootMode();
    String createUriGetAccess();
    String createUriDeviceStatus();
    String createUriPassword();
    String readingCard();
    String doHash(String _id);
    String upTimeDevice();
    void setPort(unsigned int port){ this->_port = port;}
    void _setLed(uint8_t _ledPin);
    void init();
    bool createRequest();
    bool writeHeader();
    bool initializeDevice();
    HTTPClient _http;
    MFRC522 rc522;
    
    struct rfidData{
        int    _codeState;
        String _username;
        String _traceID;
        String _messages;
        String _rfid;
    } userData;

public:
    Spacecat(String Username, String Password, String DeviceName);
    void RC522_PIN(uint8_t SS_PIN);
    void RC522_PIN(uint8_t SS_PIN, uint8_t RESET_PIN);
    const char* _certificate = "EE:1A:A8:59:25:F2:67:6B:4D:D2:BB:45:DE:AE:37:C2:CF:4C:97:A1";
    uint8_t validate();
    uint8_t getWifiQuality();
    bool readCard();
    bool enteredPassword(String user_password);
    bool begin();
    uint8_t Code();
    String Name();
    String TraceID();
    String Messages();
    String RFID();
};

#endif