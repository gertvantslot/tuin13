#include <Arduino.h>
#include <WiFi.h>
#include <ezTime.h>
#include <myWiFi.h>
#include <SPIFFS.h>

#include <server.h>

#include "tuinWebServer.h"

const char* ssid = __WIFI_SSID__;
const char* password = __WIFI_PASW__;

Timezone CET;
tuinWebServer server(80);

void setup() {
    // Initialize and wait for serial
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Serial.println();
    Serial.println();
    Serial.println("==========================");
    Serial.println(" Tuin13 - 2020 - Starting");
    Serial.println("==========================");
    Serial.println();
    delay(1000);

    // WiFi connection
    Serial.println("==========================");
    Serial.println("Starting WiFi connection");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    WiFi.enableIpV6();
    delay(1000);
    Serial.println(WiFi.localIPv6());
    Serial.println();
    delay(1000);

    // Time sync
    Serial.println("==========================");
    Serial.println("Time sync");
    waitForSync();
    if (!CET.setCache(0)) CET.setLocation("Europe/Berlin");
    Serial.println("Time is set.");
    Serial.println(CET.dateTime());
    Serial.println();
    delay(1000);

    // WebServer startup
    Serial.println("==========================");
    Serial.println("Filesystem");
    SPIFFS.begin();
    Serial.println("Filesystem started");
    Serial.println();
    delay(1000);

    // WebServer startup
    Serial.println("==========================");
    Serial.println("Webserver");
    server.start(&CET, SPIFFS);
    Serial.println("Webserver started");
    Serial.println();
    delay(1000);

    // Hardware startup
    Serial.println("==========================");
    Serial.println("Hardware");

    // Read settings and status
    Serial.println("==========================");
    Serial.println("Configuration");

    Serial.println();
    Serial.println();
    Serial.println("setup ready");
}

void loop() {
    // put your main code here, to run repeatedly:
    events();
}