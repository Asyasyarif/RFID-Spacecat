![](https://github.com/AsyaSyarif/RFID-Spacecat/blob/master/header.jpg)
# Introduction
Spacecat is an Arduino library for ESP8266/ESP32 to makes things easier that requires authentication with an RFID card.

[![Build Status](https://travis-ci.org/AsyaSyarif/RFID-Spacecat.svg?branch=master)](https://travis-ci.org/AsyaSyarif/RFID-Spacecat)
[![arduino-library-badge](https://www.ardu-badge.com/badge/Spacecat.svg?)](https://www.ardu-badge.com/Spacecat)
## Getting Started

### Hardware Requirements
>Spacecat is primary built for ESP8266 Chip. but for now ESP32 is a bit of a problem, now still in progress :)
- Any board with ESP8266 chips (Wemos, NodeMCU board, etc).
- RC522 RFID board can be found [here](https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20200411031748&SearchText=rc522) or [here.](https://www.tokopedia.com/search?st=product&q=rc522)
- Keypad (optional).

### Installing Library
You need download some libraries that we need, 
- Spacecat library from [Github](https://github.com/AsyaSyarif/RFID-Spacecat). 
- RC522 library from [Github](https://github.com/miguelbalboa/rfid)
- ArduinoJson from [Github](https://github.com/bblanchon/ArduinoJson)
 **Note : for ArduinoJson libraries i'm use the latest version (6.14.1)**

### Preparation
Now grab the hardware and some jumper wire and connect it like image shown below:
![](https://github.com/AsyaSyarif/RFID-Spacecat/blob/master/circuits/basic.PNG)

#### Table Wiring : 
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

### Simple Usage or Open [Examples](https://github.com/AsyaSyarif/RFID-Spacecat/tree/master/examples) folder.
```
#include "Spacecat.h"

const char WIFI_SSID[]     = "";   // your network SSID (name) 
const char WIFI_PASSWORD[] = "";   // your network password

const char DEVICE_NAME[] = "Reader-1";
const char SPACECAT_USERNAME[] = "";
const char SPACECAT_PASSWORD[] = "";

#define RC522_SS_PIN 15
Spacecat cat(SPACECAT_USERNAME, SPACECAT_PASSWORD, DEVICE_NAME);

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
	
  //the Reset pin of RC522 connected to Ground (GND)
  cat.RC522_PIN(RC522_SS_PIN);
  cat.begin();

}

void loop() {
  
  if(WiFi.status() == WL_CONNECTED){
  
    switch (cat.validate()){      

      case SUCCEED:
        Serial.printf("User Authenticated : %s ", cat.Name().c_str());
      break;  

      case FILL_THE_PASSWORD:
        Serial.println("Insert Password   :");
        if(cat.enteredPassword("123456")){
          Serial.println("Password Correct");
        }else{  
          Serial.println("Wrong Password");
        }
      break;

      case UNSUCCESSFULL:
         Serial.printf("Failed : %s", cat.Messages().c_str());
      break;

      default:
      break;
    }
  }
}
```

### Get The Authentication Key
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
