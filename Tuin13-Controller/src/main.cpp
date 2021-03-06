#include <Arduino.h>

#ifdef ESP32
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include <ezTime.h>
#include <myWiFi.h>

#include "pins.h"
#include "tuinWebServer.h"
#include "tuinLamp.h"
#include "Schedule.h"
#include "button.h"

const char* ssid = __WIFI_SSID__;
const char* password = __WIFI_PASW__;
const char* hostname = __WIFI_HOSTNAME__;

Timezone CET;
tuinWebServer server(80);

Dusk2Dawn sun(51.313363, 4.4954359, 0.0);
tuinLamp lamp;
Schedule lampMorning("Ochtend");
Schedule lampEvening("Avond");

button lampButton(PIN_TUIN_LAMP_BUTTON);

void initSun() {
    Serial.println("Setting sun position");
    sun = Dusk2Dawn(51.313363, 4.4954359, -CET.getOffset() / 60);
    lamp.setPosition(&sun);

    // Iedere dag om 03:05 -> Zet de zon opnieuw voor opvangen wijziging zomer/wintertijd
    // (offset wijzigt dan namelijk)

    // Zet event voor morgen
    time_t morgen = CET.now() + SECS_PER_DAY;
    tmElements_t morgen3_05;
    breakTime(morgen, morgen3_05);
    morgen3_05.Hour = 3;
    morgen3_05.Minute = 5;
    morgen3_05.Second = 0;

    CET.setEvent(initSun, makeTime(morgen3_05));
    Serial.println("Setting sun position - finished");
}

void setup() {
    // Set all connected hardware to a save position
    pinMode(PIN_TUIN_LAMP_ACTIVE, OUTPUT);
    pinMode(PIN_TUIN_LAMP_MANUAL, OUTPUT);
    pinMode(PIN_TUIN_LAMP_RELAY, OUTPUT);
    pinMode(PIN_TUIN_LAMP_BUTTON, OUTPUT);

    digitalWrite(PIN_TUIN_LAMP_ACTIVE, LOW);
    digitalWrite(PIN_TUIN_LAMP_MANUAL, LOW);
    digitalWrite(PIN_TUIN_LAMP_RELAY, LOW);
    digitalWrite(PIN_TUIN_LAMP_BUTTON, LOW);


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
    
    WiFi.hostname(hostname);
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

    #ifdef ESP32
        WiFi.enableIpV6();
    #endif
    delay(1000);
    #ifdef ESP32
        Serial.println(WiFi.localIPv6());
    #endif
    Serial.println();
    delay(1000);

    // Time sync
    Serial.println("==========================");
    Serial.println("Time sync");
    // setDebug(DEBUG);  
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

    // Read settings and status
    Serial.println("==========================");
    Serial.println("Configuration");
    lamp.prepare();
    lamp.setTimezone(&CET);
    initSun();

    Serial.println("Schedule");
    lampMorning.setStart(TIME_MODE_MIDNIGHT, 6, 45);
    lampMorning.setStop(TIME_MODE_SUNRISE, 0, 0);

    lampEvening.setStart(TIME_MODE_SUNSET, 0, 0);
    lampEvening.setStop(TIME_MODE_MIDNIGHT, 22, 48);

    lampMorning.link(lampEvening);
    lamp.setSchedule(lampMorning);
    Serial.print("Morning: ");
    lampMorning.printStatus();
    Serial.print("Evening: ");
    lampEvening.printStatus();

    Serial.println();
    Serial.println();
    delay(1000);

    // Hardware startup
    Serial.println("==========================");
    Serial.println("Hardware");
    lampButton.begin();
    lamp.setButton(&lampButton);
    lamp.pins(PIN_TUIN_LAMP_ACTIVE, PIN_TUIN_LAMP_MANUAL, PIN_TUIN_LAMP_RELAY);

    Serial.println();
    Serial.println();
    delay(1000);

    // WebServer startup
    Serial.println("==========================");
    Serial.println("Webserver");
    server.start(&CET, SPIFFS, &lamp, &sun);
    Serial.println("Webserver started");
    Serial.println();
    delay(1000);

    Serial.println();
    Serial.println();
    Serial.println("setup ready");
}

void loop() {
    // put your main code here, to run repeatedly:
    events();
    
    if (millis() % 100 == 0) {
        lampButton.loop();
        lamp.payload();
    }
}
