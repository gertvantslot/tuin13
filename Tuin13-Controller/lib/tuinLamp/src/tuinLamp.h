#pragma once

#include <Arduino.h>
#include <ezTime.h>
#include <Dusk2Dawn.h>
#include <Schedule.h>
#include <button.h>

class tuinLamp
{
private:
    /* data */
    Timezone *m_time;
    Dusk2Dawn *m_sun;
    Schedule *m_schedule;
    button *m_button;

    uint8_t pin_active;
    uint8_t pin_manual;
    uint8_t pin_relay;

    bool m_autoActive;
    bool m_manual_override;
    bool m_manual_force_on;

    time_t m_manual_override_duration = 4 * SECS_PER_HOUR; // Max 4 uur manual override
    time_t m_manual_override_end;

    bool m_debug = false;

public:
    tuinLamp(/* args */);
    ~tuinLamp();

    void prepare();

    void setPosition(Dusk2Dawn *sun);
    void setTimezone(Timezone *time);
    void setSchedule(Schedule &first);
    Schedule *getSchedule();
    void setButton(button *btn);

    void pins(uint8_t active, uint8_t manual, uint8_t relay);

    bool isActive();
    bool isManualOverride();
    bool isForceOn();
    String overrideOnEnd() { return isForceOn() ? m_time->dateTime(m_manual_override_end) : String(""); }

    void manualOn();
    void manualOff();

    void payload();

};
