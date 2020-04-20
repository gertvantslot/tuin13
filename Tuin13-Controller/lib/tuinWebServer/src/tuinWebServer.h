#pragma once

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

#include <ESPAsyncWebServer.h>
#include <ezTime.h>

#include "tuinLamp.h"

class tuinWebServer : public AsyncWebServer {
   private:
    /* data */
    Timezone *m_timezone;
    time_t m_startTime;
    
    Dusk2Dawn *m_sun;
    tuinLamp *m_lamp;

    // Handle web requests
    void notFound(AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    }

    // ** Network
    void onIpAddress(AsyncWebServerRequest *request) {
        request->send(200, "text/plain", WiFi.localIP().toString());
    }

    void onIpAddressV6(AsyncWebServerRequest *request) {
        #ifdef ESP32
        request->send(200, "text/plain", WiFi.localIPv6().toString());
        #else
        request->send(200, "text/plain", "");
        #endif
    }

    void onSSID(AsyncWebServerRequest *request) {
        request->send(200, "text/plain", WiFi.SSID());
    }

    // ** Time

    void onTimeCurrent(AsyncWebServerRequest *request) {
        request->send(200, "text/plain", m_timezone->dateTime());
    }
    void onTimeStart(AsyncWebServerRequest *request) {
        request->send(200, "text/plain", m_timezone->dateTime(m_startTime));
    }

    String sunRise() {
        tmElements_t tm;
        time_t current = m_timezone->now();
        breakTime(current, tm);
        int minutes = m_sun->sunrise(tm.Year, tm.Month, tm.Day, false);
        time_t result = previousMidnight(current) + minutes * SECS_PER_MIN;
        return m_timezone->dateTime(result);
    }

    String sunSet() {
        tmElements_t tm;
        time_t current = m_timezone->now();
        breakTime(current, tm);
        int minutes = m_sun->sunset(tm.Year, tm.Month, tm.Day, false);
        time_t result = previousMidnight(current) + minutes * SECS_PER_MIN;
        return m_timezone->dateTime(result);
    }

    void onSunrise(AsyncWebServerRequest *request) {
        request->send(200, "text/plain", sunRise());
    }

    void onSunset(AsyncWebServerRequest *request) {
        request->send(200, "text/plain", sunSet());
    }

    void onLampSchedule(AsyncWebServerRequest *request) {
        char buffer[256];
        *buffer = 0; // Set empty/zero string
        Schedule *s = m_lamp->getSchedule();
        bool first = true;
        while(s) {
            if (!first) {
                strlcat(buffer, "\r\n", sizeof(buffer));
            }
            size_t index = strnlen(buffer, sizeof(buffer));
            s->description(buffer + index, sizeof(buffer) - index);
            s = s->next();
        }
        request->send(200, "text/plain", buffer);
    }

    void onLampBurning(AsyncWebServerRequest *request) {
        request->send(200, "text/plain", m_lamp->isActive() ? "1" : "0");
    }

    void onLampStatus(AsyncWebServerRequest *request) {
        char * description;
        if (m_lamp->isActive()) {
            if (m_lamp->isForceOn()) {
                description = "AAN - Blijft aan";
            } else {
                if (m_lamp->isManualOverride()) {
                    description = "AAN - Manueel";
                } else {
                    description = "AAN";
                }
            }
        } else {
            if (m_lamp->isManualOverride()) {
                description = "UIT - manueel";
            }
        }
        request->send(200, "text/plain", description);
    }

   public:
    tuinWebServer(uint16_t port);
    void start(Timezone *timezone, fs::FS& fs, tuinLamp *lamp, Dusk2Dawn *sun);
    ~tuinWebServer();
};

