#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <FirebaseArduino.h> 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
char watt[5];
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   
#define AIO_USERNAME    "360iot"
#define AIO_KEY         "cf0ac52579ae43039b28c99d4493c881"
/*FIREBASE****************************************************/
#define FIREBASE_HOST "test-5c528.firebaseio.com"                        //link of api
#define FIREBASE_AUTH "eQR35g0wmj4geDTYzcVlfeaWuHTIakcNY3PspIxq"
/*..........................................................*/
/**************pzem.........................................*/
#include <PZEM004T.h> //https://github.com/olehs/PZEM004T
PZEM004T pzem(&Serial);                                        /// use Serial
IPAddress ip(192,168,1,1);
/**************************************************************/
WiFiClient client;
int bill_amount = 0;   
unsigned int energyTariff = 8.0; 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Power = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Power");
Adafruit_MQTT_Publish bill = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/bill");

#include "ACS712.h"
ACS712 sensor(ACS712_30A, A0);
unsigned long last_time =0;
unsigned long current_time =0;
float Wh =0 ; 
void MQTT_connect();
/****************pzem************************************/
float vol=0;
float cur=0;
float powe=0;
float ene=0;
unsigned long lastMillis = 0;
/*************************************************************/
int button_pin=D1;               

WiFiManager wifiManager;

void setup() {
    //Serial.begin(115200);
    pinMode(button_pin,INPUT);
    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("ENERGY SAVING MODULE");       //Leave this blank if you want the ESP to auto-generate the Access Point's name
    Serial.print("Successfully connected to ");      //if you get here you have connected to the WiFi
    Serial.println(WiFi.SSID());
    /**********************************************************************/
      /*******************************pinmodes*/
   pinMode(D5,OUTPUT);  //define pinmodes
  digitalWrite(D5,LOW);
  /*****************************************/

  sensor.calibrate();
  
  /*************************firebase******************/
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
  delay(1000);
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
        MQTT_connect();

  //code copied
    
    String firebaseResult;
    firebaseResult=Firebase.getString("on");
    
  Serial.println(firebaseResult);
  delay(1000);
  if (firebaseResult=="1"){
      //code to happen if the status is ON
      Serial.println("LED IS ON"); 
      digitalWrite(D5,HIGH);  
      
  }else{
      //code to happen if the status is OFF
      Serial.println("LED IS OFF");
      digitalWrite(D5,LOW); 
    }


 float v = pzem.voltage(ip);          
   if(v >= 0.0){   vol =v; } //V
  
    float i = pzem.current(ip);
    if(i >= 0.0){ cur=i;    }  //A                                                                                                                      
    
    float p = pzem.power(ip);
    if(p >= 0.0){powe=p;       } //kW
    
    float e = pzem.energy(ip);          
    if(e >= 0.0){  ene =e;  } ///kWh

    delay(1000);
          if (millis() - lastMillis > 10000) {
            lastMillis = millis();
            Serial.println("voltage\n.....................");
            Serial.println(vol);
            Serial.println("current\n.....................");
            Serial.println(cur);
            Serial.println("power\n.....................");
            Serial.println(powe);
            Serial.println("energy\n.....................");
            Serial.println(ene);
            Serial.println(".........................................end.....................");
       
            
               

          }         
  


  Serial.print(F("\nSending Power val "));
  Serial.println(p);
  Serial.print("...");

  if (! Power.publish(p)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

   if (! bill.publish(e)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

/*if (e==9){
for (int i =0; i<=2; i++)
{
  bill.publish(e);
delay(5000);
}
bill_amount =6;
}*/

 Firebase.setFloat ("Current",powe);
 //Firebase.setFloat ("usage",vol);

delay(500);
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
    }
