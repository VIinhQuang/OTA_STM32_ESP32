#include <WiFi.h>
#include <WiFiClient.h>
//#include <Arduino_JSON.h>
#include <TBPubSubClient.h>
#include <ESP32httpUpdate.h>
#include <WiFiClient.h>
#include <math.h>
#include <string>
#include <FS.h>
#include "stm32ota.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>


unsigned long lastTime;
char link_bin[100];
const char* ssid = "1111";
const char* password = "mothaibabonnamsau";
#define ARDUINOJSON_USE_LONG_LONG 1
stm32ota STM32(5, 4, 2);  //For use with libray STM32OTA

const char* link_Updt  = "you Link";

boolean MandatoryUpdate = false;
//----------------------------------------------------------------------------------
const int buttonPin = 9;
const int ledPin = 2;
boolean aux = false;

int button = true;
void wifiConnect() {
  Serial.println("");
  WiFi.disconnect(true);  
  WiFi.mode(WIFI_STA);
  delay(2000);  //Aguarda a estabilizaçao do modulo.
  WiFi.begin(ssid, password);
  byte b = 0;
  while (WiFi.status() != WL_CONNECTED && b < 60) {  //Tempo de tentativa de conecxao - 60 segundos
    b++;
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP:");
  Serial.println(WiFi.localIP());
}

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
    Serial.println(s);  //usar apenas no debug
  }


  if (httpCode != HTTP_CODE_OK) {
    return;
  }

  StaticJsonDocument<800> doc;
  deserializeJson(doc, s);
  strlcpy(link_bin, doc["link"] | "", sizeof(link_bin));
  MandatoryUpdate = doc["mandatory"] | false;
  Serial.println(link_bin);  //For debug only
  //Debug.println(MandatoryUpdate);                   //For debug only
}
void setup() {
  Serial.begin(9600);
  Serial.println("DEBUG SOFTWARESERIAL");
  SPIFFS.begin();
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  delay(200);
  wifiConnect(); 
  delay(200);
  //STM32.RunMode();
  
  checkupdt();
  Serial.println("END OF INITIALIZATION");
}
void loop() {

  button = digitalRead(buttonPin);
  if (!button) {
    digitalWrite(ledPin, HIGH);
    Serial.println("START UPDATE");
    delay(2000);
    checkupdt(false);
    String myString = String(link_bin);
    Serial.println(STM32.otaUpdate(myString));  //For debug only
    Serial.println("END OF UPDT");              //For debug only
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