#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#define MAX_SRV_CLIENTS 2
const char* ssid = "your ssid";
const char* password = "your password";
byte ptt_pin = 0;

WiFiServer server(502);
WiFiClient serverClients[MAX_SRV_CLIENTS];

ESP8266WebServer httpServer(1088);
ESP8266HTTPUpdateServer httpUpdater;

void setup() {
  pinMode(ptt_pin, OUTPUT);
  Serial.begin(19200, SERIAL_8N2);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  
  server.begin();
  server.setNoDelay(true);

  //WiFi.softAP(ssid, password, 1, 1);
  WiFi.enableAP(0);
}

void loop() {
  httpServer.handleClient();
  
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
        digitalWrite(ptt_pin, HIGH); delay(10);
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
      } else { delay(25); digitalWrite(ptt_pin, LOW); }
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
