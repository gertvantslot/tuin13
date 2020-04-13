#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ezTime.h>

extern Timezone CET;

void setupWebServer();