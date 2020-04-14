#include "tuinLamp.h"

#include "Dusk2Dawn.h"

float calcRunrise(float lat, float lon, time_t time, float offset) {
    return 0;
}

tuinLamp::tuinLamp(/* args */) {
    
}

tuinLamp::~tuinLamp() {
}

void tuinLamp::updateSun() {
    if (m_lat && m_lon && m_time) {
        m_sun = Dusk2Dawn(m_lat, m_lon, -m_time->getOffset() / 60);
    }
}

void tuinLamp::setPosition(float lat, float lon) {
    m_lat = lat;
    m_lon = lon;
    updateSun();
}

void tuinLamp::setTimezone(Timezone *time) {
    m_time = time;
    updateSun();
}

void tuinLamp::setPins(uint8_t led, uint8_t button, uint8_t relay) {
    pin_led = led;
    pin_button = button;
    pin_relay = relay;

    pinMode(pin_button, INPUT_PULLUP);
    pinMode(pin_led, OUTPUT);
    pinMode(pin_relay, OUTPUT);
}

time_t tuinLamp::nextSunrise() {
    time_t now = m_time->now();
    time_t today = sunriseToday();
    if (today < now) {
        return sunriseTomorrow();
    }
    return today;
}

time_t tuinLamp::sunrise(time_t midnight) {
    int y = year(midnight);
    int m = month(midnight);
    int d = day(midnight);

    int sunRiseMinutes = m_sun.sunrise(y, m, d, false);
    time_t result = midnight + sunRiseMinutes * SECS_PER_MIN;
    return result;
}

time_t tuinLamp::sunriseToday() {
    time_t now = m_time->now();
    time_t midnight = previousMidnight(now);
    return sunrise(midnight);
}

time_t tuinLamp::sunriseTomorrow() {
    time_t now = m_time->now();
    time_t midnight = nextMidnight(now);
    return sunrise(midnight);
}

time_t tuinLamp::nextSunset() {
    time_t now = m_time->now();
    time_t today = sunsetToday();
    if (today < now) {
        return sunsetTomorrow();
    }
    return today;
}

time_t tuinLamp::sunset(time_t midnight) {
    int y = year(midnight);
    int m = month(midnight);
    int d = day(midnight);

    int sunsetMinutes = m_sun.sunset(y, m, d, false);
    time_t result = midnight + sunsetMinutes * SECS_PER_MIN;
    return result;
}

time_t tuinLamp::sunsetToday() {
    time_t now = m_time->now();
    time_t midnight = previousMidnight(now);
    return sunset(midnight);
}

time_t tuinLamp::sunsetTomorrow() {
    time_t now = m_time->now();
    time_t midnight = nextMidnight(now);
    return sunset(midnight);
}
