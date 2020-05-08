#include "tuinWebServer.h"

const char *text_plain = "text/plain";
const char *application_json = "application/json";

tuinWebServer ::tuinWebServer(uint16_t port) : AsyncWebServer(port) {
}

tuinWebServer ::~tuinWebServer() {
}

void parseTime(const char *time, int &hour, int &min) {
    char buf[3];
    strncpy(buf, time, 2);
    hour = atoi(buf);
    min = atoi(time + strlen(time) - 2);
}

String tuinWebServer::sunRise() {
    tmElements_t tm;
    time_t current = m_timezone->now();
    breakTime(current, tm);
    int minutes = m_sun->sunrise(tm.Year, tm.Month, tm.Day, false);
    time_t result = previousMidnight(current) + minutes * SECS_PER_MIN;
    return m_timezone->dateTime(result, "d-m-Y H:i:s");
}

String tuinWebServer::sunSet() {
    tmElements_t tm;
    time_t current = m_timezone->now();
    breakTime(current, tm);
    int minutes = m_sun->sunset(tm.Year, tm.Month, tm.Day, false);
    time_t result = previousMidnight(current) + minutes * SECS_PER_MIN;
    return m_timezone->dateTime(result, "d-m-Y H:i:s");
}

// Handle web requests
void tuinWebServer::notFound(AsyncWebServerRequest *request) {
    l("Notfound");
    request->send(404, text_plain, F("Not found"));
}

// ** Network
void tuinWebServer::onIpAddress(AsyncWebServerRequest *request) {
    request->send(200, text_plain, WiFi.localIP().toString());
}

void tuinWebServer::onIpAddressV6(AsyncWebServerRequest *request) {
#ifdef ESP32
    request->send(200, text_plain, WiFi.localIPv6().toString());
#else
    request->send(200, text_plain, "");
#endif
}

void tuinWebServer::onSSID(AsyncWebServerRequest *request) {
    request->send(200, text_plain, WiFi.SSID());
}

// ** Time

void tuinWebServer::onTimeCurrent(AsyncWebServerRequest *request) {
    l("TimeCurrent");
    request->send(200, text_plain, m_timezone->dateTime("d-m-Y H:i:s"));
}
void tuinWebServer::onTimeStart(AsyncWebServerRequest *request) {
    l("TimeStart");
    request->send(200, text_plain, m_timezone->dateTime(m_startTime, "d-m-Y H:i:s"));
}

void tuinWebServer::onSunrise(AsyncWebServerRequest *request) {
    request->send(200, text_plain, sunRise());
}

void tuinWebServer::onSunset(AsyncWebServerRequest *request) {
    request->send(200, text_plain, sunSet());
}

// ** Lamp
void tuinWebServer::onLampSchedule(AsyncWebServerRequest *request) {
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

void tuinWebServer::onLampScheduleJson(AsyncWebServerRequest *request) {
    l("LampScheduleJson");
    AsyncResponseStream *response = request->beginResponseStream(application_json);
    response->print(F("{ \"schedule\": "));
    m_lamp->getSchedule()->jsonAll(*response);
    response->print(F(" }"));
    request->send(response);
}

void tuinWebServer::onLampBurning(AsyncWebServerRequest *request) {
    request->send(200, text_plain, m_lamp->isActive() ? "1" : "0");
}

void tuinWebServer::onLampTimer(AsyncWebServerRequest *request) {
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

void tuinWebServer::onLampStatus(AsyncWebServerRequest *request) {
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

void tuinWebServer::onLampStatusJson(AsyncWebServerRequest *request) {
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

void tuinWebServer::onLampOn(AsyncWebServerRequest *request) {
    m_lamp->manualOn();
    onLampStatus(request);
}

void tuinWebServer::onLampOff(AsyncWebServerRequest *request) {
    m_lamp->manualOff();
    onLampStatus(request);
}

void tuinWebServer::onScheduleSave(AsyncWebServerRequest *request) {
    int idx = 0;
    int start_mode = 0;
    int start_hour = 0;
    int start_min = 0;

    int stop_mode = 0;
    int stop_hour = 0;
    int stop_min = 0;

    int params = request->params();
    for (int i = 0; i < params; i++) {
        AsyncWebParameter *p = request->getParam(i);
        if (p->name().equalsIgnoreCase("idx")) {
            idx = p->value().toInt();
            continue;
        }
        if (p->name().equalsIgnoreCase("start-mode")) {
            start_mode = p->value().toInt();
        }
        if (p->name().equalsIgnoreCase("start-time")) {
            parseTime(p->value().c_str(), start_hour, start_min);
        }
        if (p->name().equalsIgnoreCase("stop-mode")) {
            stop_mode = p->value().toInt();
        }
        if (p->name().equalsIgnoreCase("stop-time")) {
            parseTime(p->value().c_str(), stop_hour, stop_min);
        }
    }
    Schedule *schedule = m_lamp->getSchedule();
    if (idx > 0) {
        schedule = schedule->next();
    }

    schedule->setStart(start_mode, start_hour, start_min);
    schedule->setStop(stop_mode, stop_hour, stop_min);

    m_lamp->storeConfig();

    request->redirect("/");
}

void tuinWebServer::start(Timezone *timezone, fs::FS &fs, tuinLamp *lamp, Dusk2Dawn *sun) {
    m_timezone = timezone;
    m_startTime = timezone->now();
    m_lamp = lamp;
    m_sun = sun;

    onNotFound([this](AsyncWebServerRequest *request) { this->notFound(request); });
    on("/api/network/ipaddress.txt", [this](AsyncWebServerRequest *request) { this->onIpAddress(request); });
    on("/api/network/ipaddressv6.txt", [this](AsyncWebServerRequest *request) { this->onIpAddressV6(request); });
    on("/api/network/ssid.txt", [this](AsyncWebServerRequest *request) { this->onSSID(request); });

    on("/api/time/current.txt", [this](AsyncWebServerRequest *request) { this->onTimeCurrent(request); });
    on("/api/time/current", [this](AsyncWebServerRequest *request) { this->onTimeCurrent(request); });
    on("/api/time/start.txt", [this](AsyncWebServerRequest *request) { this->onTimeStart(request); });
    on("/api/time/sunrise.txt", [this](AsyncWebServerRequest *request) { this->onSunrise(request); });
    on("/api/time/sunset.txt", [this](AsyncWebServerRequest *request) { this->onSunset(request); });

    on("/api/lamp/burning.txt", [this](AsyncWebServerRequest *request) { this->onLampBurning(request); });
    on("/api/lamp/status.txt", [this](AsyncWebServerRequest *request) { this->onLampStatus(request); });
    on("/api/lamp/status.json", [this](AsyncWebServerRequest *request) { this->onLampStatusJson(request); });
    on("/api/lamp/schedule.txt", [this](AsyncWebServerRequest *request) { this->onLampSchedule(request); });
    on("/api/lamp/schedule.json", [this](AsyncWebServerRequest *request) { this->onLampScheduleJson(request); });
    on("/api/lamp/on.txt", [this](AsyncWebServerRequest *request) { this->onLampOn(request); });
    on("/api/lamp/off.txt", [this](AsyncWebServerRequest *request) { this->onLampOff(request); });
    on("/api/lamp/timer.txt", [this](AsyncWebServerRequest *request) { this->onLampTimer(request); });

    on("/schedule.save.html", [this](AsyncWebServerRequest *request) { this->onScheduleSave(request); });

    serveStatic("/", fs, "/www/")
        .setDefaultFile("index.min.html");
    begin();
}
