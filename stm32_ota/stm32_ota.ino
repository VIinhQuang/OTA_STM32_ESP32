#include "stm32ota.h"
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <FS.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include "ArduinoJson.h"  //Version 6.15.2
#define ARDUINOJSON_USE_LONG_LONG 1

SoftwareSerial Debug;  //For debug only
#define MYPORT_TX 14   //For debug only
#define MYPORT_RX 13   //For debug only

stm32ota STM32(5, 4, 2);  //For use with libray STM32OTA
const char* ssid = "HardwareLab";
const char* password = "Hayuongbidao";


const char* link_Updt = link_Updt = "https://raw.githubusercontent.com/VIinhQuang/OTA_STM32_ESP8266/main/LinkUpdt.txt";
char link_bin[100];
boolean MandatoryUpdate = false;
//----------------------------------------------------------------------------------
const int buttonPin = 12;
const int ledPin = 2;
boolean aux = false;
unsigned long lastTime;
int button = true;

//----------------------------------------------------------------------------------
void wifiConnect() {
  Debug.println("");
  WiFi.disconnect(true);  
  WiFi.mode(WIFI_STA);
  delay(2000);  //Aguarda a estabilizaçao do modulo.
  WiFi.begin(ssid, password);
  byte b = 0;
  while (WiFi.status() != WL_CONNECTED && b < 60) {  //Tempo de tentativa de conecxao - 60 segundos
    b++;
    Debug.print(".");
    delay(500);
  }
  Debug.println("");
  Debug.print("IP:");
  Debug.println(WiFi.localIP());
}
//----------------------------------------------------------------------------------
void checkupdt(boolean all = true) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, link_Updt);
  int httpCode = http.GET();
  String s = "";
  s = http.getString();
  http.end();
  s.trim();

  if (all) {
    Debug.println(s);  //usar apenas no debug
  }


  if (httpCode != HTTP_CODE_OK) {
    return;
  }

  StaticJsonDocument<800> doc;
  deserializeJson(doc, s);
  strlcpy(link_bin, doc["link"] | "", sizeof(link_bin));
  MandatoryUpdate = doc["mandatory"] | false;
  Debug.println(link_bin);  //For debug only
  //Debug.println(MandatoryUpdate);                   //For debug only
}

//----------------------------------------------------------------------------------
void setup() {
  Debug.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);  //For debug only
  Serial.begin(9600, SERIAL_8E1);
  Debug.println("DEBUG SOFTWARESERIAL");
  SPIFFS.begin();
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  delay(200);
  wifiConnect(); 
  delay(200);
  //STM32.RunMode();
  
  checkupdt();
  Debug.println("END OF INITIALIZATION");
}

void loop() {

  button = digitalRead(buttonPin);
  if (!button) {
    digitalWrite(ledPin, HIGH);
    Debug.println("START UPDATE");
    delay(2000);
    checkupdt(false);
    String myString = String(link_bin);
    Debug.println(STM32.otaUpdate(myString));  //For debug only
    Debug.println("END OF UPDT");              //For debug only
  }
  //_______________________________________________________________________

  if (millis() - lastTime > 500) {             //BLINK LED BULTIN
    if (aux) {
      aux = false;
    } else aux = true;
    lastTime = millis();
    digitalWrite(ledPin, aux);
  }
}
