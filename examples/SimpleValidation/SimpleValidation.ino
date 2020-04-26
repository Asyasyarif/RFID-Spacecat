 
 /*******************************************************************
 *   
 *  Spacecat requires :
 *  RC522 :  https://github.com/miguelbalboa/rfid
 *  ArduinoJson : https://github.com/bblanchon/ArduinoJson
 *  - im use the latest version (Currently is 6.14.1)
 *   
 *  An simple example of how to auth the User :
 *  you can very easily manage user, placement, and can also add the rule for each placement
 *  
 *   Every you doing tap card or request to server, You definitely get a return value :
 *   101  Project not active          
 *   102  User not active             
 *   103  Placement not               
 *   104  SUCCEED                     
 *   105  UNSUCCESSFULL               
 *   106  Invalid Password            
 *   107  User Need Fill the Password 
 *   108  User reach the limit of parameter             
 *   109  Not Found
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
