b![](https://github.com/AsyaSyarif/RFID-Spacecat/blob/master/header.jpg)
# Introduction
Spacecat is an Arduino library for ESP8266/ESP32 to makes things easier that requires authentication with an RFID card.

[![Build Status](https://travis-ci.org/AsyaSyarif/RFID-Spacecat.svg?branch=master)](https://travis-ci.org/AsyaSyarif/RFID-Spacecat)
[![arduino-library-badge](https://www.ardu-badge.com/badge/Spacecat.svg?)](https://www.ardu-badge.com/Spacecat)

## Getting Started

### Hardware Requirements
>Spacecat is primary built for ESP8266 Chip. but for now ESP32 is a bit of a problem, now still in progress :)
- Any board with ESP8266/ESP32 chips (Wemos, NodeMCU board, etc).
- RC522 RFID board can be found [here](https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20200411031748&SearchText=rc522) or [here.](https://www.tokopedia.com/search?st=product&q=rc522)
- Keypad (optional).

### Installing Library
You need download some libraries that we need, 
- Spacecat library from [Github](https://github.com/AsyaSyarif/RFID-Spacecat). 
- RC522 library from [Github](https://github.com/miguelbalboa/rfid)
- ArduinoJson from [Github](https://github.com/bblanchon/ArduinoJson)<br>
 **Note : for ArduinoJson libraries i'm use the latest version (6.14.1)**

### Preparation Basic Wiring
Now grab the hardware and some jumper wire and connect it like image shown below:
![](https://github.com/AsyaSyarif/RFID-Spacecat/blob/master/circuits/basic.PNG)

#### Wiring : 
| ESP8266       | RC522         | 
| ------------- |:-------------:|
| 3V3      | 3.3V          | 
| GND     | GND           | 
| D8 (GPIO15) | SDA / SS      | 
| D5 (GPIO14) | SCK      | 
| D7 (GPIO13) | MOSI      | 
| D6 (GPIO12) | MISO      | 
| -| IRQ      | 
| GND | RST      | 

### Simple Usage or Open this [Examples](https://github.com/AsyaSyarif/RFID-Spacecat/tree/master/examples) folder.
```
#include "Spacecat.h"

const char WIFI_SSID[]     = "";   // your network SSID (name) 
const char WIFI_PASSWORD[] = "";   // your network password

const char DEVICE_NAME[] = "";
const char SPACECAT_USERNAME[] = "";
const char SPACECAT_PASSWORD[] = "";

#define RC522_SS_PIN 15

Spacecat cat(SPACECAT_USERNAME, SPACECAT_PASSWORD, DEVICE_NAME);

void handleCallback(int code, String refrenceID, String name, String messages){
  Serial.print("Code : ");
  Serial.println(code);

  Serial.print("Ref : ");
  Serial.println(refrenceID);

  Serial.print("Name : ");
  Serial.println(name);

  Serial.print("Messages : ");
  Serial.println(messages);
}


void setup() {
  //for debuging purpose
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  cat.begin(RC522_SS_PIN);
  cat.setSimpleCallback(&handleCallback);
}


void loop() {
  
  if(WiFi.status() == WL_CONNECTED){
    cat.loop();
  }

}

```

### Get The Authentication Key
+ **Please pay attention to accessing our website, please use desktop version instead mobile version [only temporary]**
- Sign Up [here](https://rfid.asyarif.net/login)
- Create the Project.
- Create the Placement.
- After done then open placement  and go to **Options -> Credentials** menu.
- Copy and paste the Username & Password 
- Put it to here in your code :
```
const char SPACECAT_USERNAME[] = "";
const char SPACECAT_PASSWORD[] = "";
```

### Notes
 Every request to the server has limitations, only **300 requests per minute**, 
less than that will be accepted and the rest will be rejected and wait a few moments


### To Do
* Write clean code
* Write better documentaiton
