 
 /*******************************************************************
 *   
 *  Spacecat requires :
 *  RC522 :  https://github.com/miguelbalboa/rfid
 *  ArduinoJson : https://github.com/bblanchon/ArduinoJson
 *  - im use the latest version (Currently is 6.14.1)
 *   
 *  Makes sure the user entered the password
 *         
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

void handleCallback(bool userFound){

  if(userFound){
    Serial.print("hello ");
    Serial.println(cat.user.name);
    Serial.println(cat.user.messages);

    /*
      - Make sure the user have a password
      - the password must be 6 digits and numeric
      - In this case For example I'd with simple password and hard-coded "123456"
    */
    if(cat.user.statusCode == FILL_THE_PASSWORD){
      if(cat.enteredPassword("123456")){
        Serial.println("The Password correct");
      }else{
        Serial.println("The Password incorrect");
      }
    }
  }

}


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

  cat.setTimeoutPassword(5000); // 5 seconds
  cat.begin(RC522_SS_PIN);
  cat.setCallback(&handleCallback);
}

void loop() {
  
  if(WiFi.status() == WL_CONNECTED){
    cat.loop();
  }

}