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

    // Handle web requests
    void notFound(AsyncWebServerRequest *request) {
        l("Notfound");
        request->send(404, text_plain, F("Not found"));
    }

    // ** Network
    void onIpAddress(AsyncWebServerRequest *request) {
        request->send(200, text_plain, WiFi.localIP().toString());
    }

    void onIpAddressV6(AsyncWebServerRequest *request) {
#ifdef ESP32
        request->send(200, text_plain, WiFi.localIPv6().toString());
#else
        request->send(200, text_plain, "");
#endif
    }

    void onSSID(AsyncWebServerRequest *request) {
        request->send(200, text_plain, WiFi.SSID());
    }

    // ** Time

    void onTimeCurrent(AsyncWebServerRequest *request) {
        l("TimeCurrent");
        request->send(200, text_plain, m_timezone->dateTime("d-m-Y H:i:s"));
    }
    void onTimeStart(AsyncWebServerRequest *request) {
        l("TimeStart");
        request->send(200, text_plain, m_timezone->dateTime(m_startTime, "d-m-Y H:i:s"));
    }

    String sunRise() {
        tmElements_t tm;
        time_t current = m_timezone->now();
        breakTime(current, tm);
        int minutes = m_sun->sunrise(tm.Year, tm.Month, tm.Day, false);
        time_t result = previousMidnight(current) + minutes * SECS_PER_MIN;
        return m_timezone->dateTime(result, "d-m-Y H:i:s");
    }

    String sunSet() {
        tmElements_t tm;
        time_t current = m_timezone->now();
        breakTime(current, tm);
        int minutes = m_sun->sunset(tm.Year, tm.Month, tm.Day, false);
        time_t result = previousMidnight(current) + minutes * SECS_PER_MIN;
        return m_timezone->dateTime(result, "d-m-Y H:i:s");
    }

    void onSunrise(AsyncWebServerRequest *request) {
        request->send(200, text_plain, sunRise());
    }

    void onSunset(AsyncWebServerRequest *request) {
        request->send(200, text_plain, sunSet());
    }

    void onLampSchedule(AsyncWebServerRequest *request) {
        l("LampSchedule");
        char buffer[256];
        *buffer = 0;  // Set empty/zero string
        Schedule *s = m_lamp->getSchedule();
        bool first = true;
        while (s) {
            if (!first) {
                strlcat(buffer, "\r\n", sizeof(buffer));
            }
            size_t index = strnlen(buffer, sizeof(buffer));
            s->description(buffer + index, sizeof(buffer) - index);
            s = s->next();
        }
        request->send(200, text_plain, buffer);
    }

    void onLampScheduleJson(AsyncWebServerRequest *request) {
        l("LampScheduleJson");
        AsyncResponseStream *response = request->beginResponseStream(application_json);
        response->print(F("{ \"schedule\": "));
        m_lamp->getSchedule()->jsonAll(*response);
        response->print(F(" }"));
        request->send(response);
    }

    void onLampBurning(AsyncWebServerRequest *request) {
        request->send(200, text_plain, m_lamp->isActive() ? "1" : "0");
    }

    void onLampTimer(AsyncWebServerRequest *request) {
        unsigned long seconds = m_lamp->brandurenSeconds();
        if (seconds == 0) {
            request->send(200, text_plain, "0 sec");
            return;
        }
        AsyncResponseStream *response = request->beginResponseStream(text_plain);
        uint8_t hours = seconds / SECS_PER_HOUR;
        uint8_t minutes = (seconds % SECS_PER_HOUR) / SECS_PER_MIN;
        uint8_t s = seconds % SECS_PER_MIN;

        response->print(hours);
        response->print(" uur ");
        response->print(minutes);
        response->print(" min ");
        response->print(s);
        response->print(" sec");
        request->send(response);
    }

    void onLampStatus(AsyncWebServerRequest *request) {
        String description;
        if (m_lamp->isActive()) {
            if (m_lamp->isForceOn()) {
                description = F("AAN - Blijft aan");
            } else {
                if (m_lamp->isManualOverride()) {
                    description = F("AAN - Manueel");
                } else {
                    description = F("AAN");
                }
            }
        } else {
            if (m_lamp->isManualOverride()) {
                description = F("UIT - manueel");
            } else {
                description = F("UIT");
            }
        }
        request->send(200, text_plain, description);
    }

    void onLampStatusJson(AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream(application_json);
        response->print(F("{ \"status\": "));
        response->print(m_lamp->isActive() ? 1 : 0);
        response->print(F(", \"manueel\": "));
        response->print(m_lamp->isForceOn() || m_lamp->isManualOverride() ? 1 : 0);
        response->print(F(", \"tot\" : \""));
        if (m_lamp->isManualOverride()) {
            response->print(m_lamp->overrideOnEnd());
        }
        response->print(F("\" }"));
        request->send(response);
    }

    void onLampOn(AsyncWebServerRequest *request) {
        m_lamp->manualOn();
        onLampStatus(request);
    }

    void onLampOff(AsyncWebServerRequest *request) {
        m_lamp->manualOff();
        onLampStatus(request);
    }

   public:
    tuinWebServer(uint16_t port);
    void start(Timezone *timezone, fs::FS &fs, tuinLamp *lamp, Dusk2Dawn *sun);
    ~tuinWebServer();
};
