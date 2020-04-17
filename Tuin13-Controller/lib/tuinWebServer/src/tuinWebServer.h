#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
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
        request->send(200, "text/plain", WiFi.localIPv6().toString());
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

   public:
    tuinWebServer(uint16_t port);
    void start(Timezone *timezone, fs::FS& fs, tuinLamp *lamp, Dusk2Dawn *sun);
    ~tuinWebServer();
};

