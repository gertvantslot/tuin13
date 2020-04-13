#include <server.h>

Timezone CET;
AsyncWebServer server(80);
time_t serverStartTime;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

// ** Network
void onIpAddress(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", WiFi.localIP().toString());
}

void onSSID(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", WiFi.SSID());
}

// ** Time

void onTimeCurrent(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", CET.dateTime());
}
void onTimeStart(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", CET.dateTime(serverStartTime));
}

void setupWebServer() {
    // WebServer startup
    Serial.println("==========================");
    Serial.println("Webserver");
    server.onNotFound(notFound);
    // Serve files in directory "/www/" when request url starts with "/"
    // Request to the root or none existing files will try to server the defualt
    // file name "index.html" if exists
    server.on("/api/network/ipaddress.txt", onIpAddress);
    server.on("/api/network/ssid.txt", onSSID);
    server.on("/api/time/current.txt", onTimeCurrent);
    server.on("/api/time/current", onTimeCurrent);
    server.on("/api/time/start.txt", onTimeStart);
    server.serveStatic("/", SPIFFS, "/www/")
        .setDefaultFile("index.html");    
    server.begin();

    serverStartTime = now();
    Serial.println("Server started");
    Serial.println();

}