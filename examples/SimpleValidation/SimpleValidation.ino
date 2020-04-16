 
 /*******************************************************************
 *   
 *  Spacecat requires :
 *  RC522 :  https://github.com/miguelbalboa/rfid
 *  ArduinoJson : https://github.com/bblanchon/ArduinoJson
 *  - use the latest version (Currently is 6.14.1)
 *   
 *  An simple example of how to auth the User
 *  - SUCCEED
 *  - FILL_THE_PASSWORD
 *  - ERR_NOT_FOUND
 *  - UNSUCCESSFULL
 * 
 * -- SUCCEED
 * if Users registered and don't have a Password,
 * it will automatically accepted
 *
 * -- FILL_THE_PASSWORD
 * for example i'm use hard code for password : "12345678",
 * if the User Password is match, you'll get a messages on serial Monitor "Password Correct"
 * and if Wrong you'll get "Wrong Password"
 * for real purpose it must have to use keypad,
 * and put User Password into *.enterpassword attributes.
 * 
 * -- UNSUCCESSFULL
 * this case if User, Placment or the Rule does not meet the conditions 
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

const char DEVICE_NAME[] = "Device 1";
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

      //Password 12345678 is just example, change it with your password
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
