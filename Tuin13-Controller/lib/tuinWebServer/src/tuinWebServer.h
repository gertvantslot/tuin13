#pragma once

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

#include <ESPAsyncWebServer.h>
#include <ezTime.h>

#include "tuinLamp.h"

extern const char *text_plain;
extern const char *application_json;

class tuinWebServer : public AsyncWebServer {
   private:
    /* data */
    Timezone *m_timezone;
    time_t m_startTime;

    Dusk2Dawn *m_sun;
    tuinLamp *m_lamp;

    void l(const char *msg) {
        Serial.println(msg);
    }

    String sunRise();
    String sunSet();

    // Handle web requests
    void notFound(AsyncWebServerRequest *request);

    // ** Network
    void onIpAddress(AsyncWebServerRequest *request);
    void onIpAddressV6(AsyncWebServerRequest *request);
    void onSSID(AsyncWebServerRequest *request);

    // ** Time
    void onTimeCurrent(AsyncWebServerRequest *request);
    void onTimeStart(AsyncWebServerRequest *request);
    void onSunrise(AsyncWebServerRequest *request);
    void onSunset(AsyncWebServerRequest *request);
    
    // ** Lamp
    void onLampSchedule(AsyncWebServerRequest *request);
    void onLampScheduleJson(AsyncWebServerRequest *request);
    void onLampBurning(AsyncWebServerRequest *request);
    void onLampTimer(AsyncWebServerRequest *request);
    void onLampStatus(AsyncWebServerRequest *request);
    void onLampStatusJson(AsyncWebServerRequest *request);
    void onLampOn(AsyncWebServerRequest *request);
    void onLampOff(AsyncWebServerRequest *request);

    void onScheduleSave(AsyncWebServerRequest *request);

   public:
    tuinWebServer(uint16_t port);
    void start(Timezone *timezone, fs::FS &fs, tuinLamp *lamp, Dusk2Dawn *sun);
    ~tuinWebServer();
};
