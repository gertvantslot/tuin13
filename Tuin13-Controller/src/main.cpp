#include <Arduino.h>

#ifdef ESP32
#include <AsyncTCP.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include <ezTime.h>
#include <myWiFi.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MySchedule.h"
#include "button.h"
#include "pins.h"
#include "tuinLamp.h"
#include "tuinWebServer.h"

#ifdef TESTDEVICE
#define debug(args...) \
    if (m_debug) Serial.print(args)
#define debugln(args...) \
    if (m_debug) Serial.println(args)
#else
#define debug(args...)
#define debugln(args...)
#endif

#define l(args...) \
    Serial.print(args)
#define ln(args...) \
    Serial.println(args)

#define EEPROM_ADDRESS_EZTIME 0x00
#define EEPROM_ADDRESS_LAMP   0x40

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

/************************* Adafruit.io Setup *********************************/

// #define MQTT_SECURE

#define AIO_SERVER "io.adafruit.com"

// WiFiFlientSecure for SSL/TLS support
#ifdef MQTT_SECURE

// Using port 8883 for MQTTS
#define AIO_SERVERPORT 8883
WiFiClientSecure client;
// io.adafruit.com SHA1 fingerprint
static const char* fingerprint PROGMEM = "77 00 54 2D DA E7 D8 03 27 31 23 99 EB 27 DB CB A5 4C 57 18";

#else
#define AIO_SERVERPORT 1883
WiFiClient client;
#endif

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

#if TESTDEVICE
Adafruit_MQTT_Publish mqtt_lamp = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/debug.tuinlamp");
Adafruit_MQTT_Publish mqtt_uren = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/debug.branduren");
Adafruit_MQTT_Subscribe mqtt_onoff = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/debug.manual");
#else
Adafruit_MQTT_Publish mqtt_lamp = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/tuin13.tuinlamp");
Adafruit_MQTT_Publish mqtt_uren = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/tuin13.branduren");
Adafruit_MQTT_Subscribe mqtt_onoff = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/tuin13.manual");
#endif

unsigned long mqtt_retry_connect = 0;
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
    int8_t ret;

    // Stop if already connected.
    if (mqtt.connected()) {
        return;
    }

    if (millis() < mqtt_retry_connect) {
        return;
    }

    l(F("Connecting to MQTT... "));
    mqtt_retry_connect = millis() + 5000;  // Retry once every 5 seconds

    if ((ret = mqtt.connect()) != 0) {  // connect will return 0 for connected
        ln(mqtt.connectErrorString(ret));
        l(F("Retrying MQTT connection in 5 seconds..."));
        mqtt.disconnect();
        return;
    }

    l(F("MQTT Connected!"));
}

/* *************************************************** */

void initSun() {
    Serial.println(F("Setting sun position"));
    sun = Dusk2Dawn(51.313363, 4.4954359, -CET.getOffset() / SECS_PER_MIN);
    lamp.setPosition(&sun);

    // Iedere dag om 03:05 -> Zet de zon opnieuw voor opvangen wijziging zomer/wintertijd
    // (bij winteruur/zomeruur wijziging om 03:05 altijd achter de rug)

    // Zet event voor morgen
    time_t morgen = CET.now() + SECS_PER_DAY;
    tmElements_t morgen3_05;
    breakTime(morgen, morgen3_05);
    morgen3_05.Hour = 3;
    morgen3_05.Minute = 5;
    morgen3_05.Second = 0;

    CET.setEvent(initSun, makeTime(morgen3_05));
    Serial.println(F("Setting sun position - finished)"));
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
    delay(500);
    Serial.println();
    Serial.println();
    Serial.println(F("=========================="));
    Serial.println(F(" Tuin13 - 2020 - Starting"));
    Serial.println(F("=========================="));
    Serial.println();

    // WiFi connection
    Serial.println(F("=========================="));
    Serial.println(F("Starting WiFi connection"));

    WiFi.hostname(hostname);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
    Serial.println();
#ifdef ESP32
    WiFi.enableIpV6();
    Serial.println(WiFi.localIPv6());
#endif
    Serial.println();

    // Time sync
    Serial.println(F("=========================="));
    Serial.println(F("Time sync"));
    // setDebug(DEBUG);
    waitForSync();
    if (!CET.setCache(EEPROM_ADDRESS_EZTIME)) {
        CET.setLocation(F("Europe/Berlin"));
    }
    Serial.println(F("Time is set."));
    CET.setDefault();  // Use CET as default timezone
    Serial.println(CET.dateTime());
    Serial.println();

    // WebServer startup
    Serial.println(F("=========================="));
    Serial.println(F("Filesystem"));
    SPIFFS.begin();
    Serial.println(F("Filesystem started"));
    Serial.println();

    // Read settings and status
    Serial.println(F("=========================="));
    Serial.println(F("Configuration"));
    lamp.prepare();
    lamp.setTimezone(&CET);
    initSun();

    Serial.println(F("Schedule"));
    lampMorning.setStart(TIME_MODE_MIDNIGHT, 6, 45);
    lampMorning.setStop(TIME_MODE_SUNRISE, 0, 00);

#if TESTDEVICE
    lampEvening.setStart(TIME_MODE_SUNSET, 2, 25);
#else
    lampEvening.setStart(TIME_MODE_SUNSET, 0, 00);
#endif
    lampEvening.setStop(TIME_MODE_MIDNIGHT, 23, 30);

    lampMorning.link(lampEvening);
    lamp.setSchedule(lampMorning);
    lamp.restoreConfig(EEPROM_ADDRESS_LAMP);

    Serial.print(F("Morning: "));
    lampMorning.printStatus();
    Serial.print(F("Evening: "));
    lampEvening.printStatus();

    Serial.println();
    Serial.println();

    // Hardware startup
    Serial.println(F("=========================="));
    Serial.println(F("Hardware"));
    lampButton.begin();
    lamp.setButton(&lampButton);
    lamp.pins(PIN_TUIN_LAMP_ACTIVE, PIN_TUIN_LAMP_MANUAL, PIN_TUIN_LAMP_RELAY);

    Serial.println();
    Serial.println();

    // MQTT + Google Assistant
    Serial.println(F("=========================="));
    Serial.println(F("Adafruit MQTT"));

#ifdef MQTT_SECURE
    // check the fingerprint of io.adafruit.com's SSL cert
    client.setFingerprint(fingerprint);
#endif

    mqtt.subscribe(&mqtt_onoff);
    MQTT_connect();
    if (mqtt.connected()) {
        // Set all to Zero
        mqtt_lamp.publish(0);

        // Reset manual activation
        #ifdef TESTDEVICE
        mqtt.publish(IO_USERNAME "/feeds/debug.manual", "2");
        #else
        mqtt.publish(IO_USERNAME "/feeds/tuin13.manual", "2");
        #endif

        // Clear mqtt queue, while we where gone
        while (mqtt.readSubscription(1000)) {
        }
    }
    Serial.println();
    Serial.println();

    // WebServer startup
    Serial.println(F("=========================="));
    Serial.println(F("Webserver"));
    server.start(&CET, SPIFFS, &lamp, &sun);
    Serial.println(F("Webserver started"));

    Serial.println();
    Serial.println();
    Serial.println(F("setup ready"));
#ifdef TESTDEVICE
    Serial.println(F("=========================="));
    Serial.println(F("Running on TESTDEVICE"));
#endif
}

unsigned long lastPublish = 0L;
unsigned long lastPayload = 0L;
unsigned long lastMqttCheck = 0L;

void loop() {
    // put your main code here, to run repeatedly:
    events();

    if (millis() - lastPayload >= 100) {
        lastPayload = millis();
        lampButton.loop();
        lamp.payload();
    }
    lamp.applyStatusLeds();
    lamp.applyRelay();

    if (millis() - lastMqttCheck >= 333) {
        // Check 3x per second
        // readSubscription is slow, so do not do this in every loop() round
        lastMqttCheck = millis();

        MQTT_connect();

        if (mqtt.connected()) {
            if (millis() - lastPublish >= 60000) {
                lastPublish = millis();  // Every minute
                Serial.print(CET.dateTime());
                Serial.print(F(" - MQQT: logging current value: "));
                Serial.println(lamp.isActive());
                mqtt_lamp.publish(lamp.isActive());
            }

            if (lamp.brandurenPublish()) {
                unsigned long branduren_seconds = lamp.brandurenSeconds();
                double uren = branduren_seconds / 3600.0;
                Serial.print(CET.dateTime());
                Serial.print(F(" MQTT: Logging branduren: "));
                Serial.print(branduren_seconds);
                Serial.print(F("sec = "));
                Serial.print(uren);
                Serial.println(F(" uren"));
                mqtt_uren.publish(uren);
                lamp.brandurenMarkPublished();
            }

            Adafruit_MQTT_Subscribe* subscription;
            while ((subscription = mqtt.readSubscription())) {
                if (subscription == &mqtt_onoff) {
                    Serial.print(CET.dateTime());
                    Serial.print(F(" MQTT: Received: "));
                    Serial.println((char*)mqtt_onoff.lastread);
                    int value = atoi((char*)mqtt_onoff.lastread);

                    switch (value) {
                        case 0:
                            lamp.manualOff();
                            break;
                        case 1:
                            lamp.manualOn(4, 0);
                            break;
                        case 2:  // reset
                            lamp.manualReset();
                            break;
                    }
                    lamp.payload();
                }
            }
        }
    }
}

