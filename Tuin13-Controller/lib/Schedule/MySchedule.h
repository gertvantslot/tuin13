#pragma once

#include <Arduino.h>
#include <Dusk2Dawn.h>
#include <ezTime.h>
#include <EEPROM.h>

#define TIME_MODE_MIDNIGHT 0
#define TIME_MODE_SUNSET 1
#define TIME_MODE_SUNRISE 2

class ScheduleTime {
   private:
    // Start tijd | Sunset | Sunrise
    uint8_t m_mode = TIME_MODE_MIDNIGHT;
    time_t m_offset;

#ifdef TESTDEVICE
    bool m_debug = true;
#else
    bool m_debug = false;
#endif

    char m_description[16];

   public:
    ScheduleTime(uint8_t mode, int hours, int minutes);
    time_t when(Dusk2Dawn &sun, time_t t);
    time_t whenSunrise(Dusk2Dawn &sun, time_t t);
    time_t whenSunset(Dusk2Dawn &sun, time_t t);

    void printStatus();
    const char *description(char *buffer, size_t len);
    void json(Print &s);
    int putInEEPROM(int offset);
    int getFromEEPROM(int offset);

};

class Schedule {
   private:
    /* data */

    const char *m_name;
    ScheduleTime m_start = ScheduleTime(TIME_MODE_MIDNIGHT, 7, 30);
    ScheduleTime m_stop = ScheduleTime(TIME_MODE_SUNRISE, 0, 0);

    Schedule *m_next;

#ifdef TESTDEVICE
    bool m_debug = true;
#else
    bool m_debug = false;
#endif

   public:
    Schedule(const char *name);
    ~Schedule();

    void setStart(int mode, int hours, int minutes);
    ScheduleTime *start() { return &m_start; }
    void setStop(int mode, int hours, int minutes);
    ScheduleTime *stop() { return &m_stop; }
    
    void link(Schedule &next);
    Schedule *next() { return m_next; }
    bool isActive(Dusk2Dawn &sun, time_t t);

    void printStatus();
    char *description(char *buffer, size_t len);
    void json(Print &s);
    void jsonAll(Print &s);
    int putInEEPROM(int offset);
    int getFromEEPROM(int offset);
};
