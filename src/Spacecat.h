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

#define lib_version "1.4.0"

#ifndef _Spacecat_h_
#define _Spacecat_h_

#include "Arduino.h"
#include <SPI.h>
//using the latest version 6.xx
#include <ArduinoJson.h>
//get from https://github.com/miguelbalboa/rfid
#include <MFRC522.h>
#include <memory>

#if defined(ARDUINO_ARCH_ESP8266)
    #include <Hash.h>
    #include <ESP8266WiFi.h>
	#include <ESP8266HTTPClient.h>
    #include <WiFiClientSecure.h>
    #include <WiFiClientSecureBearSSL.h>
    #define BOARD_ESP8266
extern "C" {
#include "Esp.h"
}

#elif defined(ARDUINO_ARCH_ESP32)
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <WiFiClientSecure.h>
    #include <Hash.h>
    #define BOARD_ESP32
extern "C" {
    #include "Esp.h"
}
#endif
// #define DEBUG_D


//uncomnet USE_HTTPS if you want use http://
#define USE_HTTPS
#ifndef  USE_HTTPS
    #define _protocol  "http://"
#else
    #define _protocol "https://"
#endif

#define HOST "rfid.asyarif.net"
#define PORT 80
#define PORT_S 443
#define PROJECT_N_ACTIVE        101
#define USER_N_ACTIVE           102
#define PLACEMENT_N_ACTIVE      103
#define SUCCEED                 104
#define UNSUCCESSFULL           105
#define ERR_INVALID_PASSOWORD   106
#define FILL_THE_PASSWORD       107
#define REACH_THE_LIMIT         108
#define ERR_NOT_FOUND           109
#define PASSWORD_CORRECT        111
#define PARAM_CONFIRM           112

typedef void (*Callback_Event)(bool isUserFound);

    struct Users{
        uint16_t statusCode;
        int referenceID;
        bool isMaster;
        bool status;
        bool havePassword;
        String rfid;
        char* messages;
        char* name;
        char* email;
        char* phoneNumber;
        int amount;
    };

    // this rule amount only show before processed (last amount value)
    struct RuleParam{
       int amount;
    };
    
    struct placementParam{
        char* Name1;
        int  value1;
        char* Name2;
        int  value2;
        char* Name3;
        int  value3;
        char* Name4;
        int  value4;
    };

    typedef struct {
        uint8_t data[64];
        uint32_t datalen;
        uint32_t state[8];
        unsigned long long bitlen;
    } SHA256_CTX;

    static const uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };


class Spacecat{
    bool _debug = true;
private:
    Callback_Event _callbackEvent;
    bool isReading, Auth, _requesting, _isSuccessInit, _isHavePassword, _doLoop, _isSuccessPushed, _isdoRegister, isSuccessRegister = false;
    int _httpCode;
    String _rfid;
    unsigned int _port = PORT;
    uint8_t _sdaPin;
    uint8_t _resetPin;
    uint8_t timer;
    uint8_t timeTimeout = 3; // default is three seconds
    #define _TIMER_TIMEOUT_WAITING_HTTP_REQUEST 5000
    #define _INTERVAL_UPDATE_DEVICE_STAUS 60000
    int _TIMER_INSERT_PASSWORD = 5000; //default
    unsigned long _PREVIOUS_TIME_READING_CARD, _TIMER_READING_CARD, _PREVIOUS_TIME_WAITING_HTTP_REQUEST, _PREVIOUS_INTERVAL_DEVICE_STATUS, _PREVIOUS_TIME_INSERT_PASSWORD = 0;
    unsigned long currentMillis;
    String  _payload;
    String _USERNAME_KEY;
    String _PASSWORD_KEY;
    String _DEVICE_NAME;
    String _DEVICE_TYPE = "RFID-READER";
    uint8_t validate();
    String  getLocalIP();
    String  getMacAddress();
    String  getWiFiSSID();
    uint8_t getChipID();
    String  getCoreVersion();
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
    void readCard();
    bool createRequest();
    bool initializeDevice();
    HTTPClient _http;
    WiFiClientSecure client;
    MFRC522 rc522;

   

    const char* _certificate = "EE:1A:A8:59:25:F2:67:6B:4D:D2:BB:45:DE:AE:37:C2:CF:4C:97:A1";
    //hash
    char hex[256];
    uint8_t data[256];
    uint8_t hash[32];
    #define SHA256_BLOCK_SIZE 32
    void sha256_init(SHA256_CTX *ctx);
    void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len);
    void sha256_final(SHA256_CTX *ctx, uint8_t hash[]);
    void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]);
    String SHA256(String data);
    #define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
    #define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
    #define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
    #define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
    #define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
    #define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
    #define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
    #define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))


public:
    Users user;
    RuleParam ruleParameter;
    placementParam placementParameter;
    Spacecat(String Username, String Password, String DeviceName);
    Spacecat();
    int getWifiQuality();
    void loop();
    void cancel();
    void setIntervalReading(int time);
    void setTimeoutPassword(int time);
    void setDebug(bool debug);
    void setCallback(Callback_Event callback);
    bool onlyReadCard();
    bool registerID();
    bool confirm();
    bool enteredPassword(String user_password);
    bool pushValue(const char *value);
    bool begin(uint8_t SS_PIN);
    bool begin(uint8_t SS_PIN, uint8_t RESET_PIN);
    
    const char* test_ca_cert = \
        "-----BEGIN CERTIFICATE-----\n" \
        "MIIFbTCCBFWgAwIBAgISBJIMiyUiBYnaGKpPTi+4arjUMA0GCSqGSIb3DQEBCwUA\n" \
        "MEoxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MSMwIQYDVQQD\n" \
        "ExpMZXQncyBFbmNyeXB0IEF1dGhvcml0eSBYMzAeFw0yMDA1MTgxNzQzMjlaFw0y\n" \
        "MDA4MTYxNzQzMjlaMBsxGTAXBgNVBAMTEHJmaWQuYXN5YXJpZi5uZXQwggEiMA0G\n" \
        "CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC1lsIAJd+ViMLCfXynxHyRO2tGiPzp\n" \
        "D+S0lZxoafUltXriZsZyNBmlavGO71EFABY7BzJ5cMLX1xVWY5CN55YJNmlp3I/U\n" \
        "QwQcv4O6r5sq34GA8yRFk/QdFPJecB9GiiI4vCFcCGhoqJVUmVAP4B5MndLqKRM1\n" \
        "Bg1DV3fCyD1AXtR7kV05ABLcQxyVtrE2ln2Dn7z+QMGKtSxcbGAIoU2ZAbr03tq4\n" \
        "PEOszruoLCZxjVcYerJfSFqppAkrjJT1wxz7m4ltf9BQ6RyePoSFcfF1a4xfNf9z\n" \
        "zq/x1YNh8dolS0yz+BrKWVaZzpep54V/xvW+q1lwRY6sdixBSeZsoQM3AgMBAAGj\n" \
        "ggJ6MIICdjAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsG\n" \
        "AQUFBwMCMAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFOLwUUbu1YrsrmjYTYybisrG\n" \
        "XPiIMB8GA1UdIwQYMBaAFKhKamMEfd265tE5t6ZFZe/zqOyhMG8GCCsGAQUFBwEB\n" \
        "BGMwYTAuBggrBgEFBQcwAYYiaHR0cDovL29jc3AuaW50LXgzLmxldHNlbmNyeXB0\n" \
        "Lm9yZzAvBggrBgEFBQcwAoYjaHR0cDovL2NlcnQuaW50LXgzLmxldHNlbmNyeXB0\n" \
        "Lm9yZy8wMQYDVR0RBCowKIIQcmZpZC5hc3lhcmlmLm5ldIIUd3d3LnJmaWQuYXN5\n" \
        "YXJpZi5uZXQwTAYDVR0gBEUwQzAIBgZngQwBAgEwNwYLKwYBBAGC3xMBAQEwKDAm\n" \
        "BggrBgEFBQcCARYaaHR0cDovL2Nwcy5sZXRzZW5jcnlwdC5vcmcwggEDBgorBgEE\n" \
        "AdZ5AgQCBIH0BIHxAO8AdQBep3P531bA57U2SH3QSeAyepGaDIShEhKEGHWWgXFF\n" \
        "WAAAAXIpGRd4AAAEAwBGMEQCIEyy598Hp+3QwWjR3BAXdrOhZUcI4Y70Y9h9zUw/\n" \
        "HsY8AiABiFAJI01HD6KfAOFEu/de8CJ4n52QbwzyA340yp3DAgB2ALIeBcyLos2K\n" \
        "IE6HZvkruYolIGdr2vpw57JJUy3vi5BeAAABcikZF2UAAAQDAEcwRQIhAMHOubrb\n" \
        "13evPJIplv81Ve+7G/tO7lgzTbJ5ZByMb8XrAiATjHcB1k5TfoZHIisgw1yQzR+Y\n" \
        "HIT62grhF297NkazAzANBgkqhkiG9w0BAQsFAAOCAQEAgtYcnbDcwdBx1+apkG86\n" \
        "wnrRj6zHjUO4vXlu0O4a/dRKUHjKZqj3G4+TALPczG/iS+eQVz+QtOCXfbzFQKkQ\n" \
        "Qtczi3HFzD3jGULmnpKDcjasJMFd0xNO3gS42bZpgiC82jx4pai5Nj5p3ZyEgXDD\n" \
        "j13vQ2F5cpYY6EMd1YDuHM84G1v1ouXm7fahRZnO1jKSuM8gSqCUsMQxIDb4JX5R\n" \
        "JDMXQJU+ju76XnvNRR4SktRHxkmBXOZrxSYIiRudFIV6oNzlrVR0RfLzhVpKc3HL\n" \
        "qarJmb7E67NqKIUhjbdV0GI/XGOpizZRgsG7bvkxSxCflOKUN1EmERI6dfI34GO1\n" \
        "SA==\n" \
        "-----END CERTIFICATE-----\n" \
    ;
};

#endif