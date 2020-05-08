
#include "MySchedule.h"

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
    tmElements_t tm;
    breakTime(t, tm);
    time_t sunset = sun.sunset(tm.Year, tm.Month, tm.Day, false) * 60;
    time_t result = previousMidnight(t) + sunset + m_offset;

    return result;
}

ScheduleTime::ScheduleTime(uint8_t mode, int hours, int minutes) {
    m_mode = mode;
    m_offset = hours * SECS_PER_HOUR + minutes * SECS_PER_MIN;

    // Set description
    size_t index = 0;
    switch (mode) {
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
            return previousMidnight(t) + m_offset;
        case TIME_MODE_SUNRISE:
            return whenSunrise(sun, t);
        case TIME_MODE_SUNSET:
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

const char *ScheduleTime::description(char *buffer, size_t len) {
    strncpy(buffer, m_description, len);
    return buffer;
}

void ScheduleTime::json(Print &s) {
    s.print(F("{ \"mode\": "));
    s.print(m_mode);
    s.print(F(", \"offset\": { \"hour\": "));
    s.print(m_offset / SECS_PER_HOUR);
    s.print(F(", \"minute\": "));
    s.print((m_offset % SECS_PER_HOUR) / 60);
    s.print(F(" } }"));
}

int ScheduleTime::putInEEPROM(int offset) {
    EEPROM.put(offset, m_mode);
    EEPROM.put(offset + sizeof(m_mode), m_offset);
    return offset + sizeof(m_mode) + sizeof(m_offset);
}

int ScheduleTime::getFromEEPROM(int offset) {
    EEPROM.get(offset, m_mode);
    EEPROM.get(offset + sizeof(m_mode), m_offset);
    return offset + sizeof(m_mode) + sizeof(m_offset);
}

Schedule::Schedule(const char *name) {
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
    time_t startSec = m_start.when(sun, t);
    if (t >= startSec) {
        time_t stopSec = m_stop.when(sun, t);
        if (t <= stopSec) return true;
    }

    if (m_next) {
        return m_next->isActive(sun, t);
    }
    return false;
}

void Schedule::printStatus() {
    char buffer[50];
    Serial.println(description(buffer, sizeof(buffer)));
}

char *Schedule::description(char *buffer, size_t len) {
    strncpy(buffer, m_name, len);
    strlcat(buffer, ": ", len);

    size_t index = strnlen(buffer, len);
    m_start.description(buffer + index, len - index);
    strlcat(buffer, " -> ", len);

    index = strnlen(buffer, len);
    m_stop.description(buffer + index, len - index);
    return buffer;
}

void Schedule::json(Print &s) {
    s.print(F("{ \"name\": \""));
    s.print(m_name);
    s.print(F("\", \"start\": "));
    m_start.json(s);
    s.print(F(", \"stop\": "));
    m_stop.json(s);
    s.print(F(" }"));
}

void Schedule::jsonAll(Print &s) {
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

int Schedule::putInEEPROM(int offset) {
    int o = offset;
    o = m_start.putInEEPROM(o);
    o = m_stop.putInEEPROM(o);
    return o;
}

int Schedule::getFromEEPROM(int offset) {
    int o = offset;
    o = m_start.getFromEEPROM(o);
    o = m_stop.getFromEEPROM(o);
    return o;
}
