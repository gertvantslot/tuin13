#pragma once

#include <Arduino.h>
#include <ezTime.h>
#include <Dusk2Dawn.h>

class tuinLamp
{
private:
    /* data */
    float m_lat = 0.0;
    float m_lon = 0.0;
    Timezone *m_time;

    Dusk2Dawn m_sun = Dusk2Dawn(0,0,0);

    uint8_t pin_led;
    uint8_t pin_button;
    uint8_t pin_relay;

    void updateSun();
    time_t sunrise(time_t midnight);
    time_t sunset(time_t midnight);

public:
    tuinLamp(/* args */);
    ~tuinLamp();

    void setPosition(float lat, float lon);
    void setTimezone(Timezone *time);
    void setPins(uint8_t led, uint8_t button, uint8_t relay);

    time_t nextSunrise();
    time_t sunriseToday();
    time_t sunriseTomorrow();

    time_t nextSunset();
    time_t sunsetToday();
    time_t sunsetTomorrow();

};
