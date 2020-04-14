#include "tuinWebServer.h"

tuinWebServer ::tuinWebServer(uint16_t port) : AsyncWebServer(port) {
}

tuinWebServer ::~tuinWebServer() {
}

void tuinWebServer::start(Timezone *timezone, fs::FS& fs, tuinLamp *lamp) {
    m_timezone = timezone;
    m_startTime = timezone->now();
    m_lamp = lamp;

    onNotFound([this](AsyncWebServerRequest *request){ this->notFound(request); });
    on("/api/network/ipaddress.txt", [this](AsyncWebServerRequest *request){ this->onIpAddress(request); });
    on("/api/network/ipaddressv6.txt", [this](AsyncWebServerRequest *request){ this->onIpAddressV6(request); });
    on("/api/network/ssid.txt", [this](AsyncWebServerRequest *request){ this->onSSID(request); });
    
    on("/api/time/current.txt", [this](AsyncWebServerRequest *request){ this->onTimeCurrent(request); });
    on("/api/time/current", [this](AsyncWebServerRequest *request){ this->onTimeCurrent(request); });
    on("/api/time/start.txt", [this](AsyncWebServerRequest *request){ this->onTimeStart(request); });
    on("/api/time/sunrise.txt", [this](AsyncWebServerRequest *request){ this->onSunrise(request); });
    on("/api/time/sunset.txt", [this](AsyncWebServerRequest *request){ this->onSunset(request); });

    serveStatic("/", fs, "/www/")
        .setDefaultFile("index.html");    
    begin();

}
