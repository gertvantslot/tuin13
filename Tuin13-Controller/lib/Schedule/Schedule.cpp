
#include "Schedule.h"

#define debug(args...) \
    if (m_debug) Serial.print(args)
#define debugln(args...) \
    if (m_debug) Serial.println(args)

size_t printDoubleDigit(int value, char *buffer) {
    size_t index = 0;
    if (value < 10) {
        *(buffer + 0) = '0';
        index++;
    }
    itoa(value, buffer + index, 10);
    return 2;
}

size_t printTime(int hour, int minutes, char *buffer) {
    printDoubleDigit(hour, buffer);
    *(buffer + 2) = ':';
    printDoubleDigit(minutes, buffer + 3);
    return 5;
}

size_t printTime(time_t t, char *buffer) {
    tmElements_t tm;
    breakTime(t, tm);
    return printTime(tm.Hour, tm.Minute, buffer);
}


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
    debug(F(" - sunset = "));
    debug(defaultTZ->dateTime(sunset));

    time_t result = previousMidnight(t) + sunset + m_offset;
    debug(" - ");
    debug(defaultTZ->dateTime(result));
    debugln();

    return result;
}

ScheduleTime::ScheduleTime(uint8_t mode, int hours, int minutes) {
    m_mode = mode;
    m_offset = hours * SECS_PER_HOUR + minutes * SECS_PER_MIN;

    // Set description
    size_t index = 0;
    switch (mode)
    {
    case TIME_MODE_MIDNIGHT:
        printTime(hours, minutes, m_description);
        break;
    case TIME_MODE_SUNRISE:
        strcpy(m_description, "Sunrise");
        index += 7;
        if (hours > 0 || minutes > 0) {
            strcpy(m_description + index, " + ");
            index += 3;
            printTime(hours, minutes, m_description + index);
        }     
        break;
    case TIME_MODE_SUNSET:
        strcpy(m_description, "Sunset");
        index += 6;
        if (hours > 0 || minutes > 0) {
            strcpy(m_description + index, " + ");
            index += 3;
            printTime(hours, minutes, m_description + index);
        }     
        break;
    default:
        break;
    }
}

time_t ScheduleTime::when(Dusk2Dawn &sun, time_t t) {
    switch (m_mode) {
        case TIME_MODE_MIDNIGHT:
            debugln(" - midnight");
            debugln(m_offset);
            return previousMidnight(t) + m_offset;
        case TIME_MODE_SUNRISE:
            debugln(" - sunrise");
            return whenSunrise(sun, t);
        case TIME_MODE_SUNSET:
            debugln(" - sunset");
            return whenSunset(sun, t);
        default:
            return -1;
    }
}

void ScheduleTime::printStatus() {
    switch (m_mode) {
        case TIME_MODE_MIDNIGHT:
            Serial.print(F("midnight"));
            break;
        case TIME_MODE_SUNRISE:
            Serial.print(F("sunrise"));
            break;
        case TIME_MODE_SUNSET:
            Serial.print(F("sunset"));
            break;
    }
    Serial.print(F(" - "));
    Serial.print(m_offset / SECS_PER_HOUR);
    Serial.print(F(":"));
    Serial.print((m_offset % SECS_PER_HOUR) / 60);
    Serial.println();
}

Schedule::Schedule(const char * name) {
    m_name = name;
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
    debugln(F("Check schedule-item:"));

    debug(F("now : "));
    debugln(defaultTZ->dateTime(t));
    time_t startSec = m_start.when(sun, t);

    debug(F("start : "));
    debugln(defaultTZ->dateTime(startSec));

    if (t >= startSec) {
        debugln(F(" - start passed"));
        time_t stopSec = m_stop.when(sun, t);
        if (t <= stopSec) return true;
        debugln(F(" - stop passed"));
    }

    if (m_next) {
        debugln(F(" - Trying next item"));
        return m_next->isActive(sun, t);
    }

    debugln(F("Schedule - Not active"));
    return false;
}

void Schedule::printStatus() {
    char buffer[50];
    Serial.println(description(buffer, sizeof(buffer)));
}
