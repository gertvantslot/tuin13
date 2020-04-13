#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ezTime.h>

class tuinWebServer : public AsyncWebServer {
   private:
    /* data */
    Timezone *m_timezone;
    time_t m_startTime;

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

   public:
    tuinWebServer(uint16_t port);
    void start(Timezone *timezone, fs::FS& fs);
    ~tuinWebServer();
};

