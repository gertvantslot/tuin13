#pragma once

#include <Arduino.h>
#include <ezTime.h>
#include <Dusk2Dawn.h>
#include <MySchedule.h>
#include <button.h>

#define SETTINGS_UNKNOWN -1
#define MAGIC_SETTINGS_NUMBER 0x2507

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

    int m_eeprom_address = SETTINGS_UNKNOWN;

    // ** *************************************************
    // ** Logging
    unsigned long m_branduren_seconds = 0;
    uint16_t m_branduren_published = 0;
    time_t m_branduren_last_count = 0;

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

    void manualOn(uint8_t hours, uint8_t minutes);
    void manualOn();
    void manualOff();
    void manualReset() {
        m_manual_force_on = false;
        m_manual_override = false;
    }

    void payload();
    void applyStatusLeds();
    void applyRelay();

    bool brandurenPublish();
    unsigned long brandurenSeconds();
    void brandurenMarkPublished();

    void restoreConfig(uint16_t address);
    void storeConfig();
};
