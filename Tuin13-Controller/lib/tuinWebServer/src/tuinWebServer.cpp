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

void tuinWebServer::onScheduleSave(AsyncWebServerRequest *request) {
    int idx = 0;
    int start_mode = 0;
    int start_hour = 0;
    int start_min = 0;

    int stop_mode = 0;
    int stop_hour = 0;
    int stop_min = 0;

    int params = request->params();
    for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
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

    request->redirect("/");
}

void tuinWebServer::start(Timezone *timezone, fs::FS& fs, tuinLamp *lamp, Dusk2Dawn *sun) {
    m_timezone = timezone;
    m_startTime = timezone->now();
    m_lamp = lamp;
    m_sun = sun;

    onNotFound([this](AsyncWebServerRequest *request){ this->notFound(request); });
    on("/api/network/ipaddress.txt", [this](AsyncWebServerRequest *request){ this->onIpAddress(request); });
    on("/api/network/ipaddressv6.txt", [this](AsyncWebServerRequest *request){ this->onIpAddressV6(request); });
    on("/api/network/ssid.txt", [this](AsyncWebServerRequest *request){ this->onSSID(request); });
    
    on("/api/time/current.txt", [this](AsyncWebServerRequest *request){ this->onTimeCurrent(request); });
    on("/api/time/current", [this](AsyncWebServerRequest *request){ this->onTimeCurrent(request); });
    on("/api/time/start.txt", [this](AsyncWebServerRequest *request){ this->onTimeStart(request); });
    on("/api/time/sunrise.txt", [this](AsyncWebServerRequest *request){ this->onSunrise(request); });
    on("/api/time/sunset.txt", [this](AsyncWebServerRequest *request){ this->onSunset(request); });

    on("/api/lamp/burning.txt", [this](AsyncWebServerRequest *request){ this->onLampBurning(request); });
    on("/api/lamp/status.txt", [this](AsyncWebServerRequest *request){ this->onLampStatus(request); });
    on("/api/lamp/status.json", [this](AsyncWebServerRequest *request){ this->onLampStatusJson(request); });
    on("/api/lamp/schedule.txt", [this](AsyncWebServerRequest *request){ this->onLampSchedule(request); });
    on("/api/lamp/schedule.json", [this](AsyncWebServerRequest *request){ this->onLampScheduleJson(request); });
    on("/api/lamp/on.txt", [this](AsyncWebServerRequest *request){ this->onLampOn(request); });
    on("/api/lamp/off.txt", [this](AsyncWebServerRequest *request){ this->onLampOff(request); });
    on("/api/lamp/timer.txt", [this](AsyncWebServerRequest *request){ this->onLampTimer(request); });

    on("/schedule.save.html", [this](AsyncWebServerRequest *request) { this->onScheduleSave(request); });

    serveStatic("/", fs, "/www/")
        .setDefaultFile("index.min.html");    
    begin();

}
