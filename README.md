![](https://github.com/AsyaSyarif/RFID-Spacecat/blob/master/header.jpg)
# Introduction
Spacecat is an Arduino library for ESP8266/ESP32 to makes things easier that requires authentication with an RFID card.
With this library you can easily manage the users with RFID card which the data stored on the cloud and also you can manage by schedule or even with special parameters.

[![Build Status](https://travis-ci.org/AsyaSyarif/RFID-Spacecat.svg?branch=master)](https://travis-ci.org/AsyaSyarif/RFID-Spacecat)
[![arduino-library-badge](https://www.ardu-badge.com/badge/Spacecat.svg?)](https://www.ardu-badge.com/Spacecat)
<a href="./LICENSE.md"><img src="https://img.shields.io/badge/license-MIT-green.svg"></a>

## Getting Started

### Quick Guide

Youtube Videos (Bahasa) [here](https://youtu.be/dyNRkAb60So)

Youtube Videos (English) *Soon*

## Hardware Requirements
>Spacecat is primary built for ESP8266 Chip. but for now ESP32 is a bit of a problem, now still in progress :)
- Any board with ESP8266/ESP32 chips (Wemos, NodeMCU board, etc).
- RC522 RFID board can be found [here](https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20200411031748&SearchText=rc522) or [here.](https://www.tokopedia.com/search?st=product&q=rc522)
- Keypad (optional).

## Environment Setup
1. Download and install the [Arduino IDE](https://www.arduino.cc/en/Main/Software).
2. If using an ESP module, install the Board Package.
   - For **ESP8266**:
      1. Under **File -> Preferences** add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to the **Additional Boards Manager URLs** field.
      2. Install the **esp8266** platform from **Tools -> Board -> Boards Manager**.
   - For **ESP32**, manually install the board package by following the instructions here: https://github.com/espressif/arduino-esp32/blob/master/README.md#installation-instructions.
4. Connect your device to your computer using the appropriate USB cable or USB to Serial FTDI cable.
5. Select your Arduino board or ESP module and and the correct port from the **Tools** menu.

## Installing Library
You need download some libraries that we need, 
- Spacecat library from [Github](https://github.com/AsyaSyarif/RFID-Spacecat). 
- RC522 library from [Github](https://github.com/miguelbalboa/rfid)
- ArduinoJson from [Github](https://github.com/bblanchon/ArduinoJson)<br>
 **Note : for ArduinoJson libraries i'm use the latest version (6.14.1)**

### Preparation Basic Wiring
Now grab the hardware and some jumper wire and connect it like image shown below:
![](https://github.com/AsyaSyarif/RFID-Spacecat/blob/master/circuits/basic.PNG)

#### Wiring
|ESP32  | ESP8266       | RC522         | 
| --- | ------------- |:-------------:|
| - | 3V3      | 3.3V          | 
| - | GND     | GND           | 
| - | D8 (GPIO15) | SDA / SS      | 
| - | D5 (GPIO14) | SCK      | 
| - | D7 (GPIO13) | MOSI      | 
| - | D6 (GPIO12) | MISO      | 
| - | -| IRQ      | 
| - | GND / X | RST      | 


## Get The Authentication Key
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
## Usage
 Please check [Example's](https://github.com/Asyasyarif/RFID-Spacecat/tree/master/examples) folder

## API Docs
Basically every we doing request to the server that will return the data, which will be explained below

| Value  | Description |
| ------------- | ------------- |
| **Code**  | ``[101]`` Project not active|
|      | `` [102] ``  User not active|
|      | `` [103] ``  Placement not active|
|      | `` [104] ``  Success|
|      | `` [106] ``  Invalid password|
|      | `` [107] ``  Waiting user password|
|      | `` [108] ``  User reach the limit of parameters|
|      | `` [109] ``  Not found|
|      | `` [110] ``  User parameter updated|
|      | `` [111] ``  Password OK|
|      | `` [112] ``  Password Confirmation|

## Structures 
We'll separate the code structures into 3 parts : `.user`, `.ruleParameter`, `.placementParameter`

| Struct        | Data | Data Type  | Description |
| ------------- | ------------- | --------- | --------- | 
| `{objectName}.user` | `.name` | `String` | Username |
|                     | `.rfid` | `String` | User RFID |
|                     | `.email` | `char` | Email |
|                     | `.isMaster` | `char` | `true` if user has set to Master  |
|                     | `.messages` | `char` | Message of every request |
|                     | `.phoneNumber` | `char` | Phone Number |
|                     | `.amount` | `int` | Amount of default parameter |
|                     | `.referenceID` | `int` | Unique ID for every requests |
|                     | `.havePassword` | `bool` | If user have password that's will be `true` |
|                     | `.status` | `bool` | User status |
|                     | `.statusCode` | `uint16_t` | Return code |
|`{objectName}.ruleParameter`         | `.amount` | `int` | Amount of rule parameter |
|`{objectName}.placementParameter`  | `.Name1` | `char` | Name of Placement parameter 1 |
|  | `.value1` | `int` | Amount of rule parameter  1|
|  | `.Name2` | `char` | Name of Placement parameter 2 |
|  | `.value2` | `int` | Amount of rule parameter  2|
|  | `.Name3` | `char` | Name of Placement parameter 3 |
|  | `.value3` | `int` | Amount of rule parameter  3|
|  | `.Name4` | `char` | Name of Placement parameter 4 |
|  | `.value4` | `int` | Amount of rule parameter  4|

## Functions
| Function  | Default |  Description |
| ------------- | ------------- | --------- | 
| ``cancel()`` |  |   Cancel data and proccess | 
| ``confirm()`` |  |   Confirm for changes parameters | 
| ``onlyReadingCard()`` | |   Just only reading RFID Card  | 
| ``registerID()`` |  |  Registering the ID and added to waiting list |
| ``setDebug(bool debug)`` | ``true ``  | Debugging purpose |
| ``setIntervalReading(int time)`` |   ``1000ms`` |  Delay for every reading the card
| `` setTimeoutPassword(int time)`` |   ``10000ms`` |  Timer for user inserting the password
| ``begin(uint8_t SS_PIN)`` | |  Initialization RC522 Module SS Pin and Reset to GND  | 
| ``begin(uint8_t SS_PIN, uint8_t RESET_PIN)``  | | Initialization RC522 Module SS Pin & Reset Pin | 
| ``enteredPassword(String password)`` |  |  User inserting password| 
|`` Spacecat(String Username, String Password, String DeviceName)`` | |  Initialization your credentials | 
|`` setCallback(&handleCallback);`` |  | Receive all the data feedback on any requests | 



## Contribute
There are several ways to contribute to Spacecat development. You can contribute to the repository by doing:
- Pull requests
- Create the issues

## To Do
- Write clean code / Refactoring
- Write API's
- Write Documentations

<sup>  [just contact me if you had any questions](https://www.instagram.com/asya_syarif/) </sup>
