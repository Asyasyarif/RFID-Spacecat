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


#include "Spacecat.h"
#include "Arduino.h"

Spacecat::Spacecat(String username, String password, String deviceName){

#if(username == NULL || password == NULL)
#warning Username or Password can not be empty
#endif  

    if(this->_debug){
        Serial.print("[DEBUG] Initialization : ");
    }
    this->_USERNAME_KEY = username;
    this->_PASSWORD_KEY = password;
    if(deviceName.isEmpty()){
        this->_DEVICE_NAME = this->getChipID();
    }
    this->_DEVICE_NAME  = deviceName;
}

Spacecat::Spacecat(){
      Serial.print("[DEBUG] Only Read the card : OK");
}

void Spacecat::intervalReading(int timer){
    if(timer && timer != 0){
        this->_INTERVAL_READING_CARD = timer;
    }else{
        this->_INTERVAL_READING_CARD = 1000; // defualt 1 Sec
    }
}

void Spacecat::setDebug(bool debug){
    _debug = debug;
}

void Spacecat::init(void){
   SPI.begin();
   this->intervalReading(_INTERVAL_READING_CARD);
   this->rc522.PCD_Init(_sdaPin, _resetPin);

#ifdef DEBUG_D
     Serial.print(F("\n[DEBUG] Library ver: "));
     Serial.println(F(lib_version));
#endif
}

String Spacecat::getLocalIP(){
    return WiFi.localIP().toString();
}

String Spacecat::getMacAddress(){
    return WiFi.macAddress();
}

String Spacecat::getWiFiSSID(){
    return WiFi.SSID();
}

uint8_t Spacecat::getChipID(){

#if defined(ARDUINO_ARCH_ESP8266)
    return ESP.getChipId();
#elif defined(ARDUINO_ARCH_ESP32)
    return ESP.getEfuseMac();
#endif

}

String Spacecat::getCoreVersion(){
    #if defined(ARDUINO_ARCH_ESP8266)
        return ESP.getCoreVersion();
    #elif defined(ARDUINO_ARCH_ESP32)
        return "-";
    #endif
}

uint8_t Spacecat::getBootVersion(){
#if defined(ARDUINO_ARCH_ESP8266)
    return ESP.getBootVersion();
#elif defined(ARDUINO_ARCH_ESP32)
    return 0;
#endif
}

uint8_t Spacecat::getBootMode(){
#if defined(ARDUINO_ARCH_ESP8266)
    return ESP.getBootMode();
#elif defined(ARDUINO_ARCH_ESP32)
    return 0;
#endif
}

int Spacecat::getWifiQuality(){
    int32_t dbm = WiFi.RSSI();
    if (dbm <= -100) {
      return 0;
    } else if (dbm >= -50) {
      return 100;
    } else {
      return 2 * (dbm + 100);
    }
}

String Spacecat::upTimeDevice(){
    long Day=0;
    int Hour =0;
    int Minute=0;
    int Second=0;
    int HighMillis=0;
    int Rollover=0;

    if(millis()>=3000000000){ 
        HighMillis=1;
    }
  
    if(millis()<=100000&&HighMillis==1){
        Rollover++;
        HighMillis=0;
    }

    long secsUp = millis()/1000;
    Second = secsUp%60;
    Minute = (secsUp/60)%60;
    Hour = (secsUp/(60*60))%24;
    Day = (Rollover*50)+(secsUp/(60*60*24)); 

    return (String)Day +" Day "+ (String)Hour +" Hour, " + (String)Minute + " Min, " + (String)Second + " Sec";
}


void Spacecat::parseJSON(String payload){
    const size_t capacity = JSON_OBJECT_SIZE(5) + 256;
    DynamicJsonDocument doc(capacity);
    doc.clear();
    DeserializationError error = deserializeJson(doc, payload);
    if(error){
        this->_isreadCard = false;
        if(this->_debug){
            Serial.print(F("[DEBUG] deserializeJson() failed: "));
        }
        Serial.println(error.c_str());
    }else{
        this->userData._username  = doc["name"].as<String>();
        this->userData._codeState = doc["code"].as<int>();
        this->userData._traceID   = doc["reference"].as<String>();
        this->userData._messages  = doc["messages"].as<String>();
        if(this->userData._codeState == 107){
            _isHavePassword = true;
        }
        if(_callbackEvent != NULL)
            this->_isreadCard = false;
            _callbackEvent(this->userData._codeState, this->userData._traceID, this->userData._username, this->userData._messages);
    }
}

bool Spacecat::begin(uint8_t SS_PIN){
    if(this->_debug){
        Serial.print("[DEBUG] Initialization : ");
    }
     _callbackEvent = NULL;
    this->_sdaPin = SS_PIN;
    this->init();
    if(WiFi.status() == WL_CONNECTED){
        if(this->initializeDevice()){
            if(this->_debug){
                Serial.println(F("Done "));
            }
            return true;
        }
    }
    return false;
}

bool Spacecat::begin(uint8_t SS_PIN, uint8_t RESET_PIN){
    if(this->_debug){
        Serial.print("[DEBUG] Initialization : ");
    }
     _callbackEvent = NULL;
    this->_sdaPin   = SS_PIN;
    this->_resetPin = RESET_PIN;
    this->init();
    if(WiFi.status() == WL_CONNECTED){
        if(this->initializeDevice()){
            if(this->_debug){
                Serial.println(F("Done "));
            }
            return true;
        }
    }
    return false;
}


String Spacecat::doHash(String id){
    return sha1(id);
}

String Spacecat::createUriGetAccess(){
    String uri;
    uri += _protocol;
    uri += HOST;
    uri += "/public/api/v1/spacecat/acc/";
    uri += _PASSWORD_KEY;
    uri += "/";
    uri += userData._rfid;
    uri += "/";
    uri += this->getMacAddress();
    uri += "/";
    uri += this->getLocalIP();
    uri += "/";
    uri += this->getWiFiSSID();
    return uri;
}

String Spacecat::createUriDeviceStatus(){
    String uri;
    uri += _protocol;
    uri += HOST;
    uri += "/public/api/v1/spacecat/dvc/";
    uri += _PASSWORD_KEY;
    uri += "?";
    return uri;
}

String Spacecat::createUriPassword(){
    String uri;
    uri += _protocol;
    uri += HOST;
    uri += "/public/api/v1/spacecat/pwd/";
    uri += _PASSWORD_KEY;
    uri += "?";
    return uri;
}

String Spacecat::createUriPushValue(){
    String uri;
    uri += _protocol;
    uri += HOST;
    uri += "/public/api/v1/spacecat/usrpshprm/";
    uri += _PASSWORD_KEY;
    uri += "?";
    return uri;
}

String Spacecat::createUriRegisterID(){
    String uri;
    uri += _protocol;
    uri += HOST;
    uri += "/public/api/v1/spacecat/lveusradd";
    uri += "?";
    return uri;
}


void Spacecat::doRegister(String rfid){
    if(this->_debug){
       Serial.print(F("\n[DEBUG] Regsitering ID : "));
    }
    int start= currentMillis;
    this->client.setFingerprint(_certificate);
    String _data = "rfid=" + rfid;

#ifdef  USE_HTTPS 
        if(this->_http.begin(client, this->createUriRegisterID())){
#else
        if(this->_http->begin(this->createUriRegisterID()){
#endif
            this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            this->_http.addHeader("cache-control","no-cache");
            this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
            this->_httpCode   = _http.POST(_data);
            this->_http.end();
#ifdef DEBUG_D
    Serial.printf("[HTTPS] POST... code: %d\n", _httpCode);
#endif
            if (this->_httpCode > 0) {
                if(_httpCode == HTTP_CODE_CREATED){
                    int end = millis();
                    Serial.println(F("Done"));
                    Serial.println(String("[DEBUG] Response Time: ") + (end-start) + "ms");
                    isSuccessRegister = true;
                    return;
                }
               
            }
            isSuccessRegister = false;
            if(this->_debug){
                Serial.println(" failed registrering ID");
            }
#ifdef DEBUG_D   
    Serial.printf("[HTTPS] POST... failed, error: %s\n", _http.errorToString(_httpCode).c_str());
#endif
        }
}

bool Spacecat::registerID(){
    _isdoRegister = true;
    if(_isdoRegister){
        this->readCard();
    }
    while (isSuccessRegister){
        isSuccessRegister = false;
        return true;
    }
    return false;
}

bool Spacecat::pushValue(String value){
    if(this->_debug){
       Serial.print(F("\n[DEBUG] Pushing the value "));
       Serial.print(F("\n[DEBUG] Requesting: "));
    }
    int start= currentMillis;
    if(WiFi.status() == WL_CONNECTED && !_isSuccessPushed){
        String _data = "param_value=" + value +
                        "&id_reference=" + this->userData._traceID;

        // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        this->client.setFingerprint(_certificate);
#ifdef  USE_HTTPS 
        if(this->_http.begin(client, this->createUriPushValue())){
#else
        if(this->_http->begin(this->createUriPushValue()){
#endif
            this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            this->_http.addHeader("cache-control","no-cache");
            this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
            this->_httpCode   = _http.POST(_data);
            this->_payload    = _http.getString();
            _isSuccessPushed = true;
#ifdef DEBUG_D
    Serial.printf("[HTTPS] POST... code: %d\n", _httpCode);
#endif
#ifdef DEBUG_PUSH_VALUE
    Serial.print(F("[DEBUG] Requesting : "));
    Serial.print(this->createUriPushValue());
    Serial.println(_data);
#endif

#ifdef DEBUG_HTTP_CODE_PUSH_VALUE
    Serial.print(F("[HTTPS] POST..."));
    Serial.println(_httpCode);
#endif

#ifdef DEBUG_PAYLOAD_PUSH_VALUE
    Serial.print(F("[DEBUG] Payload : "));
    Serial.println(_payload);
#endif
        this->_http.end();
        if (this->_httpCode > 0) {
            _isSuccessPushed = false;
            int end =millis();
            if(this->_debug){
                Serial.println(F("Done"));
                Serial.println(String("[DEBUG] Response Time: ") + (end-start) + "ms");
                Serial.println(F("[DEBUG] Result:"));
            }
            this->parseJSON(_payload);
            return true;
        }else{
        if(this->_debug){
            Serial.println("[DEBUG] Failed pushing value");
        }
#ifdef DEBUG_D   
    Serial.printf("[HTTPS] POST... failed, error: %s\n", _http.errorToString(_httpCode).c_str());
#endif
        return false;
        }
    return false;
    }
    }
}


bool Spacecat::enteredPassword(String Password){
    if(this->_debug){
        Serial.print(F("\n[DEBUG] User inserted password "));
        Serial.print(F("\n[DEBUG] Requesting: "));
    }
    if(WiFi.status() == WL_CONNECTED && _isHavePassword){
        int start= currentMillis;
        String _data = 
            "refid=" + this->userData._traceID +
            "&userpwd=" + this->doHash(Password) +
            "&mac_address=" + this->getMacAddress() + 
            "&local_ip_address=" + this->getLocalIP()+
            "&ssid=" + this->getWiFiSSID();
    // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    this->client.setFingerprint(_certificate);

#ifdef  USE_HTTPS 
    if(this->_http.begin(this->client, this->createUriPassword())){
#else
    if(this->_http->begin(this->createUriPassword()){
#endif
    // this->_http.addHeader("content-type","application/json");
    this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    this->_http.addHeader("cache-control","no-cache");
    this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
    this->_httpCode   = _http.POST(_data);
    this->_payload    = _http.getString();

#ifdef DEBUG_PATH_PASSWORD
    Serial.print(F("[DEBUG] Requesting : "));
    Serial.println(this->createUriPassword());
#endif

#ifdef DEBUG_HTTP_CODE_PASSWORD
    Serial.print(F("[HTTPS] GET...\n"));
    Serial.print(F("Http Code : "));
    Serial.println(_httpCode);
#endif

#ifdef DEBUG_PAYLOAD_PASSWORD
    Serial.print(F("[DEBUG] Payload : "));
    Serial.println(_payload);
#endif
    if (this->_httpCode > 0) {
        _isHavePassword = false;
        _isreadCard = false;
        this->_http.end();
        int end =millis();
        if(this->_debug){
            Serial.println(F("Done"));
            Serial.println(String("[DEBUG] Response Time: ") + (end-start) + "ms");
            Serial.println(F("[DEBUG] Result:"));
        }
#ifdef DEBUG_D
Serial.printf("[HTTPS] POST... code: %d\n", _httpCode);
#endif
        if(this->_httpCode == HTTP_CODE_OK){
            this->parseJSON(_payload);
            return true;
        }
        return false;
    }
        
#ifdef DEBUG_D   
        Serial.printf("[HTTPsS] GET... failed, error: %s", _http.errorToString(_httpCode).c_str());
#endif
        }
    }
    return false;
}

bool Spacecat::initializeDevice(){
    if(WiFi.status() == WL_CONNECTED){  
     String _data = 
        "device_name=" + _DEVICE_NAME +
        "&type_devices=" + _DEVICE_TYPE + 
        "&firmware_version=" + lib_version + 
        "&ssid=" + this->getWiFiSSID() + 
        "&wifi_strength=" + this->getWifiQuality() + 
        "&mac_address=" + this->getMacAddress() + 
        "&local_ip_address=" + this->getLocalIP()+ 
        "&esp_chip_id=" + this->getChipID() + 
        "&core_version=" + this->getCoreVersion() + 
        "&sdk_version=" + ESP.getSdkVersion() + 
        "&boot_version=" + this->getBootVersion() + 
        "&boot_mode=" + this->getBootMode() + 
        "&uptime=" + this->upTimeDevice() + 
        "&rc522_chip_version=" + "none";

        // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        this->client.setFingerprint(_certificate);
#ifdef  USE_HTTPS 
    if(this->_http.begin(client, this->createUriDeviceStatus())){
#else
    this->_http.begin(this->createUriDeviceStatus());
#endif
    this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    this->_http.addHeader("cache-control","no-cache");
    this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
    // _http.addHeader("Content-Length", _data.length());
 
    this->_httpCode   = _http.POST(_data);
    this->_payload    = _http.getString();

#ifdef DEBUG_PATH_DEVICE_INIT
    Serial.print(F("[DEBUG] Requesting : "));
    Serial.print(this->createUriDeviceStatus());
    Serial.println(_data);
#endif

#ifdef DEBUG_HTTP_CODE_DEVICE_INIT
    Serial.print(F("DEBUG] Http Code : "));
    Serial.println(_httpCode);
#endif

#ifdef DEBUG_PAYLOAD_DEVICE_INIT
    Serial.print(F("DEBUG] Payload : "));
    Serial.println(_payload);
#endif
    if (this->_httpCode > 0) {
        if(this->_httpCode == HTTP_CODE_CREATED){
            this->_isSuccessInit = true;
            return true;
        }else{
            this->_isSuccessInit = false;
            if(this->_debug){
                Serial.println(F("Upss.. Failed To Initialization Device"));
                Serial.println(F("[DEBUG] Please restart..."));
            }
            return false;
            }
        this->_http.end();
        }
#ifdef DEBUG_D   
        Serial.printf("[HTTPS] POST...: %s\n", _http.errorToString(_httpCode).c_str());
#endif
    }
    this->_isSuccessInit = false;
#ifdef DEBUG_D
     Serial.printf("[HTTPS] Unable to connect\n");
#endif
    if(this->_debug){
        Serial.println(F("Upss.. Unable to connect to server"));
        Serial.println(F("[DEBUG] Please restart..."));
    }
    }
    return false;
}


uint8_t Spacecat::validate(String idcard){ 
      if(this->_debug){
        Serial.print(F("\n[DEBUG] User tapping card "));
        Serial.print(F("\n[DEBUG] Requesting: "));
      }
        this->userData._rfid = idcard; 
        int start= currentMillis;
#ifdef  USE_HTTPS
        // this->_http.begin(this->createUriGetAccess(),this->_certificate); 
        // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        this->client.setFingerprint(_certificate);
        if(this->_http.begin(client, this->createUriGetAccess())){
            _requesting = true;
#else
            this->_http->begin(this->createUriGetAccess()); 
#endif       
            this->_http.setTimeout(10000);
            this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            this->_http.addHeader("cache-control","no-cache");
            this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
            this->_httpCode = _http.GET();
            this->_payload = _http.getString();
  
#ifdef DEBUG_PATH_REQUEST_ACCESS
            Serial.print("[HTTPS] GET...\n");
            Serial.println(this->createUriGetAccess());
#endif

#ifdef DEBUG_HTTP_CODE_REQUEST_ACCESS
            Serial.print(F("[DEBUG] Http Code : "));
            Serial.println(_httpCode);
#endif

#ifdef DEBUG_HTTP_CODE_REQUEST_ACCESS
            Serial.print(F("[DEBUG] Payload : "));
            Serial.println(_payload);
#endif       
        if (this->_httpCode > 0) {
#ifdef DEBUG_D
Serial.printf("[HTTPS] GET... code: %d\n", this->_httpCode);
#endif
            int end =millis();
            if(this->_debug){
                Serial.println(F("Done"));
                Serial.print(String("[DEBUG] Response Time: ") + (end-start) + "ms\n");
                Serial.println(String("[DEBUG] Result: "));
            }
            if(this->_httpCode == HTTP_CODE_OK){
                this->parseJSON(_payload);
            }else if(_httpCode == HTTP_CODE_NOT_FOUND){
#ifdef DEBUG_D
    Serial.println(F("[DEBUG] Data not found!"));
#endif
                    if(_callbackEvent != NULL)
                        this->_isreadCard = false;
                        this->_requesting = false;
                        _callbackEvent(109, "-" , "-", "Not Found");
                return HTTP_CODE_NOT_FOUND;
            }else if(_httpCode == HTTP_CODE_TOO_MANY_REQUESTS){
#ifdef DEBUG_D
    Serial.println(F("Wait a sec, to many request!"));
#endif
            if(_callbackEvent != NULL)
                         this->_isreadCard = false;
                        this->_requesting = false;
                    _callbackEvent(109, "-" , "-", "Not Found");
                return HTTP_CODE_TOO_MANY_REQUESTS;
            }
            this->_http.end();
        }
    } 
    return false;
}

void Spacecat::setSimpleCallback(Callback_Event callback){
   _callbackEvent = callback;
}

void Spacecat::readCard(){
    currentMillis = millis();

   if(!this->rc522.PICC_IsNewCardPresent() || !this->rc522.PICC_ReadCardSerial()){
        return;
    }
   
    if(currentMillis > _TIMER_READING_CARD + _INTERVAL_READING_CARD && !this->_isreadCard && !_isHavePassword){
        this->_TIMER_READING_CARD = currentMillis;
        this->_isreadCard = true;
        this->_content = "";

        for(byte i = 0; i < rc522.uid.size; i++){
            this->_content.concat(String(this->rc522.uid.uidByte[i] < 0x10 ? "0" : ""));
            this->_content.concat(String(this->rc522.uid.uidByte[i]));
        }
        _content.toUpperCase();
        this->rc522.PICC_HaltA();
        this->rc522.PCD_StopCrypto1();

        if(_doLoop){
            if(this->_debug){
                Serial.print(String("\n[DEBUG] RFID: ") + (this->doHash(_content.substring(0))));
            }
            this->validate(this->doHash(_content.substring(0)));
            return;
        }else if(this->_isdoRegister){
            this->doRegister(this->doHash(_content.substring(0)));
        }
         _isreadCard = false;
    }   
}

void Spacecat::loop(){

    _doLoop = true;
    currentMillis = millis();
    if(_requesting && _isreadCard){
        if(currentMillis - _PREVIOUS_TIME_WAITING_HTTP_REQUEST >= _TIMER_TIMEOUT_WAITING_HTTP_REQUEST){
            timer++;
            if(timer > timeTimeout){
#ifdef DEBUG_D
        Serial.println(F("[DEBUG] Request is timeout!"));
#endif 
            if(this->_debug){
                Serial.println(F("[DEBUG] Timeout"));
            }
                _callbackEvent(105, "-" , "-", "Request timeout");
                _isreadCard = false;
                _requesting = false;
                timer = 0;
            }
            _PREVIOUS_TIME_WAITING_HTTP_REQUEST = currentMillis;
        }
    }

    if(currentMillis - _PREVIOUS_INTERVAL_DEVICE_STATUS >= _INTERVAL_UPDATE_DEVICE_STAUS){
#ifdef DEBUG_D   
        Serial.println("[DEBUG] Sending device status");
#endif
        this->initializeDevice();
        _PREVIOUS_INTERVAL_DEVICE_STATUS = currentMillis;
    }

    if(_isSuccessInit){
        this->userData._rfid = "";
        this->readCard();
    } 
}

void Spacecat::clear(){
    _isHavePassword     = false;
    _isreadCard         = false;
    userData._codeState = 0;
    userData._traceID   = "";
    userData._username  = "";
    userData._messages  = "";

    if(this->_debug){
        Serial.println(F("[DEBUG] Cleared!"));
    }
}





