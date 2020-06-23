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

    if(username != NULL && password != NULL){
        Auth = true;
    }

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

void Spacecat::setIntervalReading(int timer){
    if(timer && timer != 0){
        this->_PREVIOUS_TIME_READING_CARD = timer;
    }else{
        this->_PREVIOUS_TIME_READING_CARD = 1000; // defualt 1 Sec
    }
}

void Spacecat::setTimeoutPassword(int timer){
    if( timer && timer != 0){
        _TIMER_INSERT_PASSWORD = timer;
    }
}

void Spacecat::setDebug(bool debug){
    _debug = debug;
}

void Spacecat::setCallback(Callback_Event callback){
   _callbackEvent = callback;
}

void Spacecat::init(void){
    SPI.begin();
    // client.setCertificate(test_ca_cert);
    _callbackEvent = NULL;
    this->setIntervalReading(_PREVIOUS_TIME_READING_CARD);
    this->rc522.PCD_Init(_sdaPin, _resetPin);
    this->isReading = false;
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

bool Spacecat::begin(uint8_t SS_PIN){

    if(this->_debug){
        Serial.print("[DEBUG] Initialization : ");
    }
    this->_sdaPin = SS_PIN;
    this->init();
    if(!Auth){
        return true;
    }else{
        if(WiFi.status() == WL_CONNECTED){
            if(this->initializeDevice()){
                if(this->_debug){
                    Serial.println(F("Done "));
                }
                return true;
            }
            return false;
        }
        return false;
    }
}

bool Spacecat::begin(uint8_t SS_PIN, uint8_t RESET_PIN){
    if(this->_debug){
        Serial.print("[DEBUG] Initialization : ");
    }

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
    // return sha1(id);
    return SHA256(id);
}

void Spacecat::sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
  uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

  for (i = 0, j = 0; i < 16; ++i, j += 4)
    m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) | ((uint32_t)data[j + 2] << 8) | ((uint32_t)data[j + 3]);
  for ( ; i < 64; ++i)
    m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

  a = ctx->state[0];
  b = ctx->state[1];
  c = ctx->state[2];
  d = ctx->state[3];
  e = ctx->state[4];
  f = ctx->state[5];
  g = ctx->state[6];
  h = ctx->state[7];

  for (i = 0; i < 64; ++i) {
    t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
    t2 = EP0(a) + MAJ(a,b,c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  ctx->state[0] += a;
  ctx->state[1] += b;
  ctx->state[2] += c;
  ctx->state[3] += d;
  ctx->state[4] += e;
  ctx->state[5] += f;
  ctx->state[6] += g;
  ctx->state[7] += h;
}

void Spacecat::sha256_init(SHA256_CTX *ctx){
  ctx->datalen = 0;
  ctx->bitlen = 0;
  ctx->state[0] = 0x6a09e667;
  ctx->state[1] = 0xbb67ae85;
  ctx->state[2] = 0x3c6ef372;
  ctx->state[3] = 0xa54ff53a;
  ctx->state[4] = 0x510e527f;
  ctx->state[5] = 0x9b05688c;
  ctx->state[6] = 0x1f83d9ab;
  ctx->state[7] = 0x5be0cd19;
}

void Spacecat::sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {

  for (uint32_t i = 0; i < len; ++i) {
    ctx->data[ctx->datalen] = data[i];
    ctx->datalen++;
    if (ctx->datalen == 64) {
      sha256_transform(ctx, ctx->data);
      ctx->bitlen += 512;
      ctx->datalen = 0;
    }
  }
}

void Spacecat::sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {

  uint32_t i = ctx->datalen;

  // Pad whatever data is left in the buffer.
  if (ctx->datalen < 56) {
    ctx->data[i++] = 0x80;
    while (i < 56)
      ctx->data[i++] = 0x00;
  } else {
    ctx->data[i++] = 0x80;
    while (i < 64)
      ctx->data[i++] = 0x00;
    this->sha256_transform(ctx, ctx->data);
    memset(ctx->data, 0, 56);
  }

  // Append to the padding the total message's length in bits and transform.
  ctx->bitlen += ctx->datalen * 8;
  ctx->data[63] = ctx->bitlen;
  ctx->data[62] = ctx->bitlen >> 8;
  ctx->data[61] = ctx->bitlen >> 16;
  ctx->data[60] = ctx->bitlen >> 24;
  ctx->data[59] = ctx->bitlen >> 32;
  ctx->data[58] = ctx->bitlen >> 40;
  ctx->data[57] = ctx->bitlen >> 48;
  ctx->data[56] = ctx->bitlen >> 56;
  sha256_transform(ctx, ctx->data);

  // Since this implementation uses little endian byte ordering and SHA uses big endian,
  // reverse all the bytes when copying the final state to the output hash.
  for (i = 0; i < 4; ++i) {
    hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
    hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
    hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
    hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
    hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
    hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
    hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
    hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
  }
}

char *btoh(char *dest, uint8_t *src, int len) {
  char *d = dest;
  while( len-- ) sprintf(d, "%02x", (unsigned char)*src++), d += 2;
  return dest;
}

String Spacecat::SHA256(String data) {
  uint8_t data_buffer[data.length()];
  
  for(int i=0; i<data.length(); i++){
    data_buffer[i] = (uint8_t)data.charAt(i);
  }
  
  SHA256_CTX ctx;
  ctx.datalen = 0;
  ctx.bitlen = 512;

  sha256_init(&ctx);
  sha256_update(&ctx, data_buffer, data.length());
  sha256_final(&ctx, hash);
  return(btoh(hex, hash, 32));
}

String Spacecat::createUriGetAccess(){
    String uri;
    uri += _protocol;
    uri += HOST;
    uri += "/public/api/v1/spacecat/acc/";
    uri += _PASSWORD_KEY;
    uri += "/";
    uri += this->doHash(this->_rfid);
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
#ifdef BOARD_ESP8266    
    this->client.setInsecure();
    // this->client.setFingerprint(_certificate);
#endif
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
            int end = millis();
            if(this->_debug){
                Serial.println(F("Done"));
                Serial.println(String("[DEBUG] Response Time: ") + (end-start) + "ms");
            }
            if (this->_httpCode > 0) {
                if(_httpCode == HTTP_CODE_CREATED){
                    isSuccessRegister = true;
                    return;
                }else if(_httpCode == HTTP_CODE_CONFLICT){
                    isSuccessRegister = false;
                    if(this->_debug){
                        Serial.println(" failed registrering ID");
                    }
                }else{
                    isSuccessRegister = false;
                    if(this->_debug){
                        Serial.println(" failed registrering ID");
                    }
                }
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

bool Spacecat::pushValue(const char *value){
    if(this->_debug){
       Serial.print(F("\n[DEBUG] Pushing the value "));
       Serial.print(F("\n[DEBUG] Requesting: "));
    }
    int start= currentMillis;
    if(WiFi.status() == WL_CONNECTED && !_isSuccessPushed){
        // String _data = "param_value=" + value +
        //                 "&id_reference=" + this->userData._traceID;

        // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
#ifdef BOARD_ESP8266    
    this->client.setFingerprint(_certificate);
#endif
#ifdef  USE_HTTPS 
        if(this->_http.begin(client, this->createUriPushValue())){
#else
        if(this->_http->begin(this->createUriPushValue()){
#endif
            this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            this->_http.addHeader("cache-control","no-cache");
            this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
            // this->_httpCode   = _http.POST(_data);
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
            // this->parseJSON(_payload);
            return true;
        } else {
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


bool Spacecat::enteredPassword(String userPassword){

    if(this->_debug){
        Serial.print(F("\n[DEBUG] User inserted password "));
        Serial.print(F("\n[DEBUG] Requesting: "));
    }
    _requesting = true;
    int start= currentMillis;
    String _data = 
                "refid=" + (String) this->user.referenceID +
                "&userpwd=" + this->doHash(userPassword) +
                "&mac_address=" + this->getMacAddress() + 
                "&local_ip_address=" + this->getLocalIP() +
                "&ssid=" + this->getWiFiSSID();

#ifdef BOARD_ESP8266   
    // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    // this->client.setFingerprint(_certificate);
     this->client.setInsecure();
#endif
       
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
   
    int end =millis();
    this->_http.end();
    _requesting = false;

    if(this->_debug){
        Serial.println(F("Done"));
        Serial.println(String("[DEBUG] Response Time: ") + (end-start) + "ms");
        Serial.println(F("[DEBUG] Result:"));
    }
    if (this->_httpCode > 0) {
        _isHavePassword = false;
#ifdef DEBUG_D
Serial.printf("[HTTPS] POST... code: %d\n", _httpCode);
#endif
        if(this->_httpCode == HTTP_CODE_OK){
            return true;
        } else {
            return false;
        }
        return false;
    }
    _isHavePassword = false;
#ifdef DEBUG_D   
        Serial.printf("[HTTPsS] GET... failed, error: %s", _http.errorToString(_httpCode).c_str());
#endif
    }
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
        "&local_ip_address=" + this->getLocalIP() + 
        "&esp_chip_id=" + this->getChipID() + 
        "&core_version=" + this->getCoreVersion() + 
        "&sdk_version=" + ESP.getSdkVersion() + 
        "&boot_version=" + this->getBootVersion() + 
        "&boot_mode=" + this->getBootMode() + 
        "&uptime=" + this->upTimeDevice() + 
        "&rc522_chip_version=" + "none";

#ifdef BOARD_ESP8266   
    // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    // this->client.setFingerprint(_certificate);
     this->client.setInsecure();
    
#endif
#ifdef BOARD_ESP32
    // this->client.setCACert(test_ca_cert);
    // client.setCertificate(test_ca_cert);
#endif
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
    this->_http.end();

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
                Serial.println(F("[DEBUG] Upss.. Failed To Initialization Device"));
                Serial.println(F("[DEBUG] Please restart..."));
            }
            return false;
        }
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
            Serial.println(F("[DEBUG] Upss.. Unable to connect to server"));
            Serial.println(F("[DEBUG] Please restart..."));
        }
    }
    return false;
}


uint8_t Spacecat::validate(){

        if(this->_debug){
            Serial.print(F("\n[DEBUG] User tapping card "));
            Serial.print(F("\n[DEBUG] Requesting: "));
        }
        int start = currentMillis;
#ifdef  USE_HTTPS
        // this->_http.begin(this->createUriGetAccess(),this->_certificate); 
#ifdef BOARD_ESP8266   
    // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    // this->client.setFingerprint(_certificate);
        this->client.setInsecure();
#endif
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

            int end = millis();
            if(this->_debug){
                Serial.println(F("Done"));
                Serial.print(String("[DEBUG] Response Time: ") + (end-start) + "ms\n");
                Serial.println(String("[DEBUG] Result: "));
            }
           
            if(this->_httpCode == HTTP_CODE_OK){
                // this->parseJSON(_payload);
                this->_requesting = false;
                const size_t capacity = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(1) + 5*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(7) + 300;
                DynamicJsonDocument doc(capacity);
                DeserializationError error = deserializeJson(doc, this->_payload);
                    if (!error) {
                        //parse user information
                        JsonObject profile      =   doc["profile"];
                        this->user.statusCode   =   doc["code"];
                        this->user.referenceID  =   doc["reference"];
                        this->user.messages     =   (char *) doc["messages"].as<char *>();
                        this->user.name         =   (char *) profile["username"].as<char *>();
                        this->user.isMaster     =   profile["master"].as<bool>();
                        this->user.status       =   profile["status"].as<bool>();
                        this->user.havePassword =   profile["password"].as<bool>();
                        this->user.email        =   (char *) profile["email"].as<char *>();
                        this->user.phoneNumber  =   (char *) profile["phone_number"].as<char *>();
                        this->user.amount       =   profile["amount"];
                        // parse rule parameter 
                        this->ruleParameter.amount   =   doc["parameter"]["rule"]["amount"];
                        // parse placement parameters if available
                        JsonArray parameter_placement        =   doc["parameter"]["placement"].as<JsonArray>();
                        if(parameter_placement.size() > 0){
                            this->placementParameter.Name1       =   (char *) parameter_placement[0]["name"].as<char *>();
                            this->placementParameter.value1      =   parameter_placement[0]["value"];
                            this->placementParameter.Name2       =   (char *) parameter_placement[1]["name"].as<char *>();
                            this->placementParameter.value2      =   parameter_placement[1]["value"];
                            this->placementParameter.Name3       =   (char *) parameter_placement[2]["name"].as<char *>();
                            this->placementParameter.value3      =   parameter_placement[2]["value"];
                            this->placementParameter.Name4       =   (char *) parameter_placement[3]["name"].as<char *>();
                            this->placementParameter.value4      =   parameter_placement[3]["value"];
                        } else {
                            this->placementParameter.Name1       =   NULL;
                            this->placementParameter.value1      =   0;
                            this->placementParameter.Name2       =   NULL;
                            this->placementParameter.value2      =   0;
                            this->placementParameter.Name3       =   NULL;
                            this->placementParameter.value3      =   0;
                            this->placementParameter.Name4       =   NULL;
                            this->placementParameter.value4      =   0;
                        }
                        
                        if(this->user.statusCode == FILL_THE_PASSWORD){
                            this->_isHavePassword = true;
                            _PREVIOUS_TIME_INSERT_PASSWORD = currentMillis;
                        }
                        _callbackEvent(true);
                    } else {
                        this->_requesting = false;
                        Serial.print(F("deserializeJson() failed with code "));
                        Serial.println(error.c_str());
                        _callbackEvent(false);
                        return false;
                    }

            }else if(_httpCode == HTTP_CODE_NOT_FOUND){
                if(this->_debug){
                  Serial.print(F("[DEBUG] Not Found \n"));
                }
                _callbackEvent(false);
#ifdef DEBUG_D
    Serial.println(F("[DEBUG] Data not found!"));
#endif              
                this->_requesting = false;
                return HTTP_CODE_NOT_FOUND;
            }else if(_httpCode == HTTP_CODE_TOO_MANY_REQUESTS){
                if(this->_debug){
                  Serial.print(F("[DEBUG] Wait a sec, to many request \n"));
                }
                _callbackEvent(false);
                this->_requesting = false;
                return HTTP_CODE_TOO_MANY_REQUESTS;
            }
            this->_http.end();
        }
    }
    return false;
}

void Spacecat::readCard(){

    currentMillis = millis();
    if(!this->rc522.PICC_IsNewCardPresent() || !this->rc522.PICC_ReadCardSerial()){
        return;
    }
    delay(1);

    if(currentMillis > _TIMER_READING_CARD + _PREVIOUS_TIME_READING_CARD && !this->isReading && !_isHavePassword){
        this->_TIMER_READING_CARD = currentMillis;
        this->_rfid = "";
        this->isReading = true;

        for(byte i = 0; i < rc522.uid.size; i++){
            this->_rfid.concat(String(this->rc522.uid.uidByte[i] < 0x10 ? "0" : ""));
            this->_rfid.concat(String(this->rc522.uid.uidByte[i]));
        }

        // this->user.rfid.toUpperCase();
        this->rc522.PICC_HaltA();
        this->rc522.PCD_StopCrypto1();
        isReading = false;
 
        if(_doLoop){
            // if(this->_debug){
            //     Serial.print(String("\n[DEBUG] RFID: ") + (this->doHash(this->_rfid.substring(0))));
            // }
            this->validate();
            return;
        }else if(this->_isdoRegister && !_doLoop){
            if(this->_debug){
                Serial.print(String("\n[DEBUG] RFID: ") + (this->doHash(this->_rfid.substring(0))));
            }
            this->doRegister(this->doHash(this->_rfid.substring(0)));
        } else {
            this->user.rfid = this->doHash(this->_rfid.substring(0));
        }
    }
}

bool Spacecat::confirm(){
#ifdef BOARD_ESP8266   
    // std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    // this->client.setFingerprint(_certificate);
     this->client.setInsecure();
#endif
    if(this->_http.begin(client, _protocol+(String) HOST+"/public/api/v1/spacecat/doact/"+ _PASSWORD_KEY+"?")){
        String _data = "id_reference=" +(String) this->user.referenceID;
        this->_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        this->_http.addHeader("cache-control","no-cache");
        this->_http.addHeader("SECRET-KEY", _USERNAME_KEY);
        this->_httpCode   = _http.POST(_data);
        this->_payload    = _http.getString();
          this->_http.end();
        if(_httpCode > 0){
            if(_httpCode == HTTP_CODE_OK){
                 return true;
            }
            return false;
        }
        return true;
    }
    return false;
}


void Spacecat::loop(){

    this->_doLoop = true;
    this->currentMillis = millis();

    if(this->_isSuccessInit){
        this->readCard();
    }

    // if user have a password, start timer password.
    if(this->_isHavePassword){
        if(this->currentMillis - _PREVIOUS_TIME_INSERT_PASSWORD >= _TIMER_INSERT_PASSWORD){
            _PREVIOUS_TIME_INSERT_PASSWORD = this->currentMillis;
            this->_isHavePassword = false; 
            if(this->_debug){
                Serial.println(F("[DEBUG] Password timeout"));
            }
        }
    }
    
    if(this->_requesting){ // set timeout if server not responding
        if(currentMillis - _PREVIOUS_TIME_WAITING_HTTP_REQUEST >= _TIMER_TIMEOUT_WAITING_HTTP_REQUEST){
            this->timer++;
            if(this->timer > timeTimeout){
                this->isReading = false;
                this->_requesting = false;
                this->timer = 0;

        if(this->_debug){
            Serial.println(F("[DEBUG] Request Time Out!"));
        }

#ifdef DEBUG_D
        Serial.println(F("[DEBUG] Request is timeout!"));
#endif 
            }
            _PREVIOUS_TIME_WAITING_HTTP_REQUEST = currentMillis;
        }
    }

    if(currentMillis - _PREVIOUS_INTERVAL_DEVICE_STATUS >= _INTERVAL_UPDATE_DEVICE_STAUS){

        this->initializeDevice();
        _PREVIOUS_INTERVAL_DEVICE_STATUS = currentMillis;

#ifdef DEBUG_D   
        Serial.println("[DEBUG] Sending device status");
#endif
    }
}

bool Spacecat::onlyReadCard(){
    if(!_doLoop){
        readCard();
    }
}

void Spacecat::cancel(){
    _isHavePassword = false;
    isReading       = false;
    
    if(this->_debug){
        Serial.println(F("[DEBUG] Canceled!"));
    }
}
