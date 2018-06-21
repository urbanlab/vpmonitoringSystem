// --------------------------------------------------------------------------------------
// OTA functions to Update Program via Wifi.
// --------------------------------------------------------------------------------------
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
int OTAport = 8266;

void setupOTA(String dnsName, const char* pwd) {
  //char myDNSName[50] = dnsName;
  // Port defaults to 8266
  ArduinoOTA.setPort(OTAport);

  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setHostname((const char *)myDNSName);

  // No authentication by default
  ArduinoOTA.setPassword((const char *)pwd);

  ArduinoOTA.onStart([]() {
    Serial.println("Start updating software");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd of software update.");
    Serial.println("Reseting Feather...");
    ESP.restart();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("\nReady for OTA updates.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Port: ");
  Serial.println(OTAport);
}

