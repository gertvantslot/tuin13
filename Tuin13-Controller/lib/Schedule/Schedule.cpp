
#include "Schedule.h"

time_t ScheduleTime::whenSunrise(Dusk2Dawn &sun, time_t t) {
    // Get reference time of current sunrise
    int y = year(t);
    int m = month(t);
    int d = day(t);

    time_t sunrise = sun.sunrise(y, m, d, false) * 60;
    return previousMidnight(t) + sunrise + m_offset;
}

time_t ScheduleTime::whenSunset(Dusk2Dawn &sun, time_t t) {
    // Get reference time of current sunrise
    int y = year(t);
    int m = month(t);
    int d = day(t);

    time_t sunset = sun.sunset(y, m, d, false) * 60;
    Serial.print(" - sunset = ");
    Serial.print(defaultTZ->dateTime(sunset));

    time_t result = previousMidnight(t) + sunset + m_offset;
    Serial.print(" - ");
    Serial.print(defaultTZ->dateTime(result));
    Serial.println();

    return result;
}

ScheduleTime::ScheduleTime(uint8_t mode, int hours, int minutes) {
    m_mode = mode;
    m_offset = hours * SECS_PER_HOUR + minutes * SECS_PER_MIN;
}

time_t ScheduleTime::when(Dusk2Dawn &sun, time_t t) {
    switch (m_mode) {
        case TIME_MODE_MIDNIGHT:
            Serial.println(" - midnight");
            Serial.println(m_offset);
            return previousMidnight(t) + m_offset;
        case TIME_MODE_SUNRISE:
            Serial.println(" - sunrise");
            return whenSunrise(sun, t);
        case TIME_MODE_SUNSET:
            Serial.println(" - sunset");
            return whenSunset(sun, t);
        default:
            return -1;
    }
}

void ScheduleTime::printStatus() {
    switch (m_mode) {
        case TIME_MODE_MIDNIGHT:
            Serial.print("midnight");
            break;
        case TIME_MODE_SUNRISE:
            Serial.print("sunrise");
            break;
        case TIME_MODE_SUNSET:
            Serial.print("sunset");
            break;
    }
    Serial.print(" - ");
    Serial.print(m_offset / SECS_PER_HOUR);
    Serial.print(":");
    Serial.print((m_offset % SECS_PER_HOUR) / 60);
    Serial.println();
}

Schedule::Schedule(/* args */) {
}

Schedule::~Schedule() {
}

void Schedule::setStart(int mode, int hours, int minutes) {
    m_start = ScheduleTime(mode, hours, minutes);
}

void Schedule::setStop(int mode, int hours, int minutes) {
    m_stop = ScheduleTime(mode, hours, minutes);
}

void Schedule::link(Schedule &next) {
    m_next = &next;
}

bool Schedule::isActive(Dusk2Dawn &sun, time_t t) {
    Serial.println("Check schedule-item:");

    Serial.print("now : ");
    Serial.println(defaultTZ->dateTime(t));
    time_t startSec = m_start.when(sun, t);

    Serial.print("start : ");
    Serial.println(defaultTZ->dateTime(startSec));

    if (t >= startSec) {
        Serial.println(" - start passed");
        time_t stopSec = m_stop.when(sun, t);
        if (t <= stopSec) return true;
        Serial.println(" - stop passed");
    }

    if (m_next) {
        Serial.println(" - Trying next item");
        return m_next->isActive(sun, t);
    }

    Serial.println("Schedule - Not active");
    return false;
}

void Schedule::printStatus() {
    Serial.print("Start : ");
    m_start.printStatus();
    Serial.print("Stop  : ");
    m_stop.printStatus();
}