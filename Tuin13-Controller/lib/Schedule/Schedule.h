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

    time_t whenSunrise(Dusk2Dawn &sun, time_t t);
    time_t whenSunset(Dusk2Dawn &sun, time_t t);

   public:
    ScheduleTime(uint8_t mode, int hours, int minutes);
    time_t when(Dusk2Dawn &sun, time_t t);
    void printStatus();
};

class Schedule {
   private:
    /* data */

    ScheduleTime m_start = ScheduleTime(TIME_MODE_MIDNIGHT, 7, 30);
    ScheduleTime m_stop = ScheduleTime(TIME_MODE_SUNRISE, 0, 0);

    Schedule *m_next;

   public:
    Schedule(/* args */);
    ~Schedule();

    void setStart(int mode, int hours, int minutes);
    void setStop(int mode, int hours, int minutes);
    void link(Schedule &next);
    bool isActive(Dusk2Dawn &sun, time_t t);
    
    void printStatus();
};

