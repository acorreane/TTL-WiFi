#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPUpdateServer.h>

char CHIP_ID[7] = ""; const char* mqtt_clientid  = itoa(ESP.getChipId(), CHIP_ID, DEC);

const byte pin_config = 0;
const byte led_wifi   = 1;

#define MAX_SRV_CLIENTS 2

WiFiServer server(502);
WiFiClient serverClients[MAX_SRV_CLIENTS];

ESP8266WebServer httpServer(1088);
ESP8266HTTPUpdateServer httpUpdater;

void setup() 
{
  pinMode(led_wifi, OUTPUT); digitalWrite(led_wifi, 0); delay(200);
  
  MDNS.begin(CHIP_ID);
  
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  
  server.begin();
  server.setNoDelay(true);

  MDNS.addService("http", "tcp", 1088);

  check_reset(); 
  setup_wifi();
  manager_call();

  Serial.begin(19200, SERIAL_8N2);
}

void manager_call()
{
  if ((digitalRead(pin_config) == LOW)) 
  {
   digitalWrite(led_wifi, HIGH); delay(1000); led_format();
  
   WiFiManager wifiManager;
 
   if (!wifiManager.startConfigPortal()) {    } else {    }
   ESP.reset();   
  }
}

void setup_wifi() 
{
  delay(100);
  
  if (WiFi.SSID() == "") {  } else { WiFi.mode(WIFI_STA); }

  while (WiFi.status() != WL_CONNECTED) 
  {
    digitalWrite(led_wifi, HIGH); delay(100);
    digitalWrite(led_wifi, LOW); delay(100);
  }
  randomSeed(micros()); delay(2000);
}

void check_reset()
{
  if (!digitalRead(pin_config))
  {
   delay(3000);
   manager_call();
  }
}



void loop() {
  httpServer.handleClient();

  if (!digitalRead(pin_config)) { check_reset(); }
  
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        //Serial.print("New client: "); Serial.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available())
      { 
        delay(10);
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
      } else { delay(25); }
    }
  }
  
  //check UART for data
  if(Serial.available()){ delay(10);
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].write(sbuf, len);
        //delay(1);
      }
    }
  }
}

void led_format()
{
 digitalWrite(led_wifi, 0); delay(50);
 digitalWrite(led_wifi, 1); delay(50);
 digitalWrite(led_wifi, 0); delay(50);
 digitalWrite(led_wifi, 1); delay(50);
 digitalWrite(led_wifi, 0); delay(50);
 digitalWrite(led_wifi, 1); delay(50);
 digitalWrite(led_wifi, 0); delay(50);
 digitalWrite(led_wifi, 1); delay(50);
}
