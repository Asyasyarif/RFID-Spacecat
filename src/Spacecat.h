/*****************************************************************************
    With this library you can easily  manage users with RFID card data stored in the cloud
    and also you can manage by schedule or even with special parameters.

    The MIT License (MIT)

    Copyright (c) 2018-2020 Asya Syarif

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*****************************************************************************/

#define lib_version "1.3.1"

#ifndef _Spacecat_h_
#define _Spacecat_h_

#include "Arduino.h"
#include <SPI.h>
//using the latest version 6.xx
#include <ArduinoJson.h>
//get from https://github.com/miguelbalboa/rfid
#include <MFRC522.h>
#include <memory>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
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

// #define DEBUG_PUSH_VALUE
// #define DEBUG_HTTP_CODE_PUSH_VALUE
// #define DEBUG_PAYLOAD_PUSH_VALUE

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
#define ERR_NOT_FOUND           109
typedef void (*Callback_Event)(int code, String refrence , String name, String messages);

class Spacecat{
    bool _debug = true;
private:
    
    bool _isreadCard ,_requesting,_isSuccessInit, _isHavePassword, _doLoop, _isSuccessPushed, _isdoRegister, isSuccessRegister = false;
    int _httpCode;
    unsigned int _port = PORT;
    uint8_t _sdaPin;
    uint8_t _resetPin;
    uint8_t timer;
    uint8_t timeTimeout = 3; // default is three seconds
    #define _TIMER_TIMEOUT_WAITING_HTTP_REQUEST 1000
    #define _INTERVAL_UPDATE_DEVICE_STAUS 60000
    #define _INTERVAL_INSERT_PASSWORD 5000
    unsigned long _INTERVAL_READING_CARD, _TIMER_READING_CARD, _TIMER_WAITING_INSERT_PASSWORD, _PREVIOUS_TIME_WAITING_HTTP_REQUEST, _PREVIOUS_INTERVAL_DEVICE_STATUS = 0;
    unsigned long currentMillis;
    String _content, _payload;
    String _USERNAME_KEY;
    String _PASSWORD_KEY;
    String _DEVICE_NAME;
    String _DEVICE_TYPE = "RFID-READER";
    uint8_t validate(String idcard);
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
    String createUriPushValue();
    String createUriRegisterID();
    String doHash(String _id);
    String upTimeDevice();
    void setPort(unsigned int port){ this->_port = port;}
    void _setLed(uint8_t _ledPin);
    void init();
    void doRegister(String rfid);
    void parseJSON(String payload);
    bool createRequest();
    bool initializeDevice();
    HTTPClient _http;
    WiFiClientSecure client;
    MFRC522 rc522;
    Callback_Event _callbackEvent;
    struct rfidData{
        int    _codeState;
        String _username;
        String _traceID;
        String _messages;
        String _rfid;
    } userData;

public:
    const char* _certificate = "EE:1A:A8:59:25:F2:67:6B:4D:D2:BB:45:DE:AE:37:C2:CF:4C:97:A1";
    void loop();
    void clear();
    void setSimpleCallback(Callback_Event callback);
    void setRawCallback();
    void intervalReading(int time);
    void readCard();
    void setDebug(bool debug);
    Spacecat(String Username, String Password, String DeviceName);
    Spacecat();
    bool registerID();
    bool enteredPassword(String user_password);
    bool pushValue(String value);
    bool begin(uint8_t SS_PIN);
    bool begin(uint8_t SS_PIN, uint8_t RESET_PIN);
    int getWifiQuality();
};

#endif