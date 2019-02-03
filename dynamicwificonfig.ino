#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

int button_pin=D1;               

WiFiManager wifiManager;

void setup() {
    Serial.begin(115200);
    pinMode(button_pin,INPUT);
    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("ENERGY SAVING MODULE");       //Leave this blank if you want the ESP to auto-generate the Access Point's name
    Serial.print("Successfully connected to ");      //if you get here you have connected to the WiFi
    Serial.println(WiFi.SSID());
}

void loop() {
    if(digitalRead(button_pin)==HIGH){ 
      Serial.println("Button Clicked");//If the button is pressed
      digitalWrite(LED_BUILTIN,LOW);
      WiFi.disconnect(true);
      delay(2000);
      Serial.println("Clearing EEPROM");
      wifiManager.resetSettings();
      for (int i = 0 ; i < EEPROM.length() ; i++)  //Clear every byte of the EEPROM, a pre-defined length is not used to allow portability to other boards
      {
        EEPROM.write(i, 0);
      }
      Serial.println();
      Serial.println();
      Serial.println("EEPROM Cleared");
      digitalWrite(LED_BUILTIN,HIGH);
      Serial.println("Press RST on NodeMCU");
      } 
    }
