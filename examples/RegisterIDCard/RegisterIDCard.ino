 
 /*******************************************************************
 *   
 *  Spacecat requires :
 *  RC522 :  https://github.com/miguelbalboa/rfid
 *  ArduinoJson : https://github.com/bblanchon/ArduinoJson
 *  - im use the latest version (Currently is 6.14.1)
 *   
 *  If you don't want to copy an dpaste the ID card, 
 *  you can use this example and automatically the ID card will appear on your project
 * 
 *  Written by Arif @2019. 
 *  Bandung, Indonesia 
 *  https://rfid.asyarif.net
 *
 *******************************************************************/


#include "Spacecat.h"

const char WIFI_SSID[]     = "";   // your network SSID (name) 
const char WIFI_PASSWORD[] = "";   // your network password

const char DEVICE_NAME[] = "";
const char SPACECAT_USERNAME[] = "";
const char SPACECAT_PASSWORD[] = "";

#define RC522_SS_PIN 15 //D8

Spacecat cat(SPACECAT_USERNAME, SPACECAT_PASSWORD, DEVICE_NAME);

void setup() {
  //for debuging purpose
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  cat.begin(RC522_SS_PIN);
  
}


void loop() {
  
  if(WiFi.status() == WL_CONNECTED){
    if(cat.registerID()){
      Serial.println("Success.");
    }
  }

}