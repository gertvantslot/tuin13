#pragma once

#include <Arduino.h>
#include <Dusk2Dawn.h>
#include <ezTime.h>

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

    const char *description(char *buffer, size_t len) {
        strncpy(buffer, m_description, len);
        return buffer;
    }

    void json(Print &s) {
        s.print(F("{ \"mode\": "));
        s.print(m_mode);
        s.print(F(", \"offset\": { \"hour\": "));
        s.print(m_offset / SECS_PER_HOUR);
        s.print(F(", \"minute\": "));
        s.print((m_offset % SECS_PER_HOUR) / 60);
        s.print(F(" } }"));
    }
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
    char *description(char *buffer, size_t len) {
        strncpy(buffer, m_name, len);
        strlcat(buffer, ": ", len);

        size_t index = strnlen(buffer, len);
        m_start.description(buffer + index, len - index);
        strlcat(buffer, " -> ", len);

        index = strnlen(buffer, len);
        m_stop.description(buffer + index, len - index);
        return buffer;
    }

    void json(Print &s) {
        s.print(F("{ \"name\": \""));
        s.print(m_name);
        s.print(F("\", \"start\": "));
        m_start.json(s);
        s.print(F(", \"stop\": "));
        m_stop.json(s);
        s.print(F(" }"));
    }

    void jsonAll(Print &s) {
        bool first = true;
        Schedule *current = this;
        s.print(F("[ "));

        while (current) {
            if (first) {
                first = false;
            } else {
                s.print(F(", "));
            }
            current->json(s);
            current = current->next();
        }
        s.print(F("]"));
    }
};
