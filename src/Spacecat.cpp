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


#include "Spacecat.h"
#include "Arduino.h"

Spacecat::Spacecat(String username, String password, String deviceName){
    if(this->_debug){
        Serial.print("Initialization : ");
    }
    this->_USERNAME_KEY = username;
    this->_PASSWORD_KEY = password;
    if(deviceName.isEmpty()){
        this->_DEVICE_NAME = this->getChipID();
    }
    this->_DEVICE_NAME  = deviceName;
}

Spacecat::Spacecat(){
      Serial.print("Only Read the card : OK");
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
       Serial.println(lib_version);
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

uint8_t Spacecat::getWifiQuality(){
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


bool Spacecat::begin(uint8_t SS_PIN){
    if(this->_debug){
        Serial.print("Initialization : ");
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
    Serial.print("Initialization : ");
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

bool Spacecat::enteredPassword(String Password){
    Serial.println(this->doHash(Password));
    if(WiFi.status() == WL_CONNECTED && _isHavePassword){
        String _data = 
            "traceid=" + this->userData._traceID +
            "&pwd=" + this->doHash(Password) +
            "&rfid=" + this->userData._rfid + 
            "&mac_address=" + this->getMacAddress() + 
            "&local_ip_address=" + this->getLocalIP()+
            "&ssid=" + this->getWiFiSSID();
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(_certificate);

#ifdef  USE_HTTPS 
    if(this->_http.begin(*client, this->createUriPassword())){
#else
    if(this->_http->begin(this->createUriPassword()){
#endif
    // this->_http.addHeader("content-type","application/json");
    this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    this->_http.addHeader("cache-control","no-cache");
    this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
    this->_httpCode   = _http.POST(_data);
    this->_payload    = _http.getString();
  #ifdef DEBUG_D   
        Serial.printf("[HTTPS] GET... failed, error: %s", _http.errorToString(_httpCode).c_str());
#endif
#ifdef DEBUG_PATH_PASSWORD
    Serial.print(F("Requesting : "));
    Serial.println(this->createUriDeviceStatus());
#endif

#ifdef DEBUG_HTTP_CODE_PASSWORD
    Serial.print(F("[HTTPS] GET...\n"));
    Serial.print(F("Http Code : "));
    Serial.println(_httpCode);
#endif

#ifdef DEBUG_PAYLOAD_PASSWORD
    Serial.print(F("Payload : "));
    Serial.println(_payload);
#endif
      if (this->_httpCode > 0) {

#ifdef DEBUG_D
Serial.printf("[HTTPS] GET... code: %d\n", _httpCode);
#endif
        if(this->_httpCode == HTTP_CODE_OK){
            return true;
        }
            this->_http.end();
            return false;
        }
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

        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        client->setFingerprint(_certificate);
#ifdef  USE_HTTPS 
    if(this->_http.begin(*client, this->createUriDeviceStatus())){
#else
    this->_http.begin(this->createUriDeviceStatus());
#endif
    this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    this->_http.addHeader("cache-control","no-cache");
    this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
    // _http.addHeader("Content-Length", _data.length());
 
    this->_httpCode   = _http.POST(_data);
    this->_payload    = _http.getString();
  
#ifdef DEBUG_D   
        Serial.printf("[HTTPS] GET... failed, error: %s\n", _http.errorToString(_httpCode).c_str());
#endif

#ifdef DEBUG_PATH_DEVICE_INIT
    Serial.print(F("Requesting : "));
    Serial.print(this->createUriDeviceStatus());
    Serial.println(_data);
#endif

#ifdef DEBUG_HTTP_CODE_DEVICE_INIT
    Serial.print(F("Http Code : "));
    Serial.println(_httpCode);
#endif

#ifdef DEBUG_PAYLOAD_DEVICE_INIT
    Serial.print(F("Payload : "));
    Serial.println(_payload);
#endif
    if (this->_httpCode > 0) {
#ifdef DEBUG_D
Serial.printf("[HTTPS] GET... code: %d\n", this->_httpCode);
#endif
        if(this->_httpCode == HTTP_CODE_CREATED){
            this->_isSuccessInit = true;
            return true;
        }else{
            this->_isSuccessInit = false;
            if(this->_debug){
                Serial.println(F("Upss.. Failed To Initialization Device"));
                Serial.println(F("Please restart..."));
            }
            return false;
            }
        this->_http.end();
        }
    }
    this->_isSuccessInit = false;
#ifdef DEBUG_D
     Serial.printf("[HTTPS] Unable to connect\n");
#endif
    if(this->_debug){
        Serial.println(F("Upss.. Unable to connect to server"));
    }
    }
    return false;
}

uint8_t Spacecat::validate(String idcard){ 
      if(this->_debug){
        Serial.print(F("Requesting: "));
      }
        this->userData._rfid = idcard; 
        int start= currentMillis;
#ifdef  USE_HTTPS
        // this->_http.begin(this->createUriGetAccess(),this->_certificate); 
        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        client->setFingerprint(_certificate);
        if(this->_http.begin(*client, this->createUriGetAccess())){
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
  
#ifdef DEBUG_D   
            Serial.printf("[HTTPS] GET... failed, error: %s\n", _http.errorToString(_httpCode).c_str());
#endif
        
#ifdef DEBUG_PATH_REQUEST_ACCESS
            Serial.print("[HTTPS] GET...\n");
            Serial.println(this->createUriGetAccess());
#endif

#ifdef DEBUG_HTTP_CODE_REQUEST_ACCESS
            Serial.print(F("Http Code : "));
            Serial.println(_httpCode);
#endif

#ifdef DEBUG_HTTP_CODE_REQUEST_ACCESS
            Serial.print(F("Payload : "));
            Serial.println(_payload);
#endif       
        if (this->_httpCode > 0) {
#ifdef DEBUG_D
Serial.printf("[HTTPS] GET... code: %d\n", this->_httpCode);
#endif
            int end =millis();
            if(this->_debug){
                Serial.println(F("Done"));
                Serial.println(String("Response Time: ") + (end-start) + "ms\n");
            }
            if(this->_httpCode == HTTP_CODE_OK){
                const size_t capacity = JSON_OBJECT_SIZE(5) + 256;
                DynamicJsonDocument doc(capacity);
                doc.clear();
                DeserializationError error = deserializeJson(doc, _payload);
                if(error){
                    this->_isreadCard = false;
                    if(this->_debug){
                        Serial.print(F("deserializeJson() failed: "));
                    }
                    Serial.println(error.c_str());
                    return false;
                }else{
                    this->userData._username  = doc["name"].as<String>();
                    this->userData._codeState = doc["code"].as<int>();
                    this->userData._traceID   = doc["reference"].as<String>();
                    this->userData._messages  = doc["messages"].as<String>();
                    if(_callbackEvent != NULL)
                        this->_isreadCard = false;
                        _callbackEvent(this->userData._codeState, this->userData._traceID, this->userData._username, this->userData._messages);
                }
           
            }else if(_httpCode == HTTP_CODE_NOT_FOUND){
#ifdef DEBUG_D
                Serial.println(F("Data not found!"));
#endif
        if(this->_debug){
            Serial.println(F("*Done"));
        }
                    if(_callbackEvent != NULL)
                        this->_isreadCard = false;
                        this->_requesting = false;
                        _callbackEvent(109, "-" , "-", "Not Found");
                return HTTP_CODE_NOT_FOUND;
            }else if(_httpCode == HTTP_CODE_TOO_MANY_REQUESTS){
#ifdef DEBUG_D
            Serial.println(F("Wait a sec, to many request!"));
#endif
        if(this->_debug){
            Serial.println(F("*Done"));
        }
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

   if(!this->rc522.PICC_IsNewCardPresent()){
        return;
    }
    if(!this->rc522.PICC_ReadCardSerial()){
        return;
    }
   
    if(currentMillis > _TIMER_READING_CARD + _INTERVAL_READING_CARD && !this->_isreadCard){
        this->_TIMER_READING_CARD = currentMillis;
        this->_isreadCard = true;
        this->_content = "";

        for(byte i = 0; i < rc522.uid.size; i++){
            this->_content.concat(String(this->rc522.uid.uidByte[i] < 0x10 ? "0" : ""));
            this->_content.concat(String(this->rc522.uid.uidByte[i]));
        }

        this->rc522.PICC_HaltA();
        this->rc522.PCD_StopCrypto1();

        if(_doLoop){
        if(this->_debug){
            Serial.println(String("\nRFID: ") + (this->doHash(_content.substring(0))));
        }
            this->validate(this->doHash(_content.substring(0)));
            return;
        }
         _isreadCard = false;
         Serial.println(this->doHash(_content.substring(0)));
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
        Serial.println(F("Request is timeout!"));
#endif 
            if(this->_debug){
                Serial.println(F("Timeout"));
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
        Serial.println("Sending device status");
#endif
        this->initializeDevice();
        _PREVIOUS_INTERVAL_DEVICE_STATUS = currentMillis;
    }

    if(_isSuccessInit){
        this->userData._rfid = "";
        this->readCard();
    } 
}






