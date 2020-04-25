#include "tuinLamp.h"

#include "Dusk2Dawn.h"
#include "ezTime.h"

#define debug(args...) \
    if (m_debug) Serial.print(args)
#define debugln(args...) \
    if (m_debug) Serial.println(args)

tuinLamp::tuinLamp(/* args */) {
}

tuinLamp::~tuinLamp() {
}

void tuinLamp::prepare() {
    m_sun = 0;
    m_time = 0;
    m_schedule = 0;
}

void tuinLamp::setPosition(Dusk2Dawn *sun) {
    m_sun = sun;
}

void tuinLamp::setTimezone(Timezone *time) {
    m_time = time;
}

void tuinLamp::setSchedule(Schedule &first) {
    m_schedule = &first;
}

Schedule *tuinLamp::getSchedule() {
    return m_schedule;
}

void tuinLamp::setButton(button *btn) {
    m_button = btn;
}

bool tuinLamp::isActive() {
    if (m_manual_force_on) {
        if (m_manual_override_end > m_time->now()) {
            return true;
        } else {
            m_manual_force_on = false;
        }
    }
    if (m_manual_override) {
        return !m_autoActive;
    } else {
        return m_autoActive;
    }
}

bool tuinLamp::isManualOverride() { return m_manual_override; }
bool tuinLamp::isForceOn() { return m_manual_force_on; }

void tuinLamp::pins(uint8_t active, uint8_t manual, uint8_t relay) {
    pin_active = active;
    pin_manual = manual;
    pin_relay = relay;

    pinMode(pin_active, OUTPUT);
    pinMode(pin_manual, OUTPUT);
    pinMode(pin_relay, OUTPUT);
}

void tuinLamp::manualOn(uint8_t hours, uint8_t minutes) {
    m_manual_override_end = nextMidnight(m_time->now()) 
        + hours * SECS_PER_HOUR 
        + minutes * SECS_PER_MIN;
    m_manual_override = false;
    m_manual_force_on = true;
}

void tuinLamp::manualOn() {
    if (m_autoActive) {
        // Automatisch aan => disable alle overrides
        m_manual_override = false;
        m_manual_force_on = false;
        return;
    }
    if (m_manual_force_on) {
        // already on
        return;
    }
    m_manual_override = true;
}

void tuinLamp::manualOff() {
    if (m_manual_force_on) {
        m_manual_force_on = false;
    }
    if (!m_autoActive) {
        m_manual_override = false;
        return;
    }
    m_manual_override = true;
}

void tuinLamp::payload() {
    // Test button
    if (m_button->isClicked()) {
        Serial.println(F("Button: Clicked"));
        m_manual_override = !m_manual_override;
    }

    if (m_button->isDoubleClicked()) {
        Serial.println(F("Button: Double-Clicked"));
        m_manual_override = false;
        m_manual_force_on = true;
        m_manual_override_end = m_manual_override_duration + m_time->now();
    }

    if (m_button->isLongPress()) {
        Serial.println(F("Button: Longpress"));
        m_manual_override = false;
        m_manual_force_on = false;
    }

    time_t n = m_time->now();
    bool autoActive = m_schedule->isActive(*m_sun, n);
    if (m_autoActive != autoActive) {
        // Autoactive changed => clear manual override
        m_autoActive = autoActive;
        m_manual_override = false;
        Serial.print(F("Schedule: Changed: "));
        Serial.println(isActive());
    }

    debug(F("tuinLamp active: "));
    debugln(isActive());
}

void tuinLamp::applyStatusLeds() {
    // Set output
    // Flash every 2,5 seconds
    int flash = (millis() % 2500) < 100 ? LOW : HIGH;
    digitalWrite(pin_active, isActive() ? flash : !flash);

    if (m_manual_force_on) {
        digitalWrite(pin_manual, (millis() / 250) % 2);
    } else {
        digitalWrite(pin_manual, m_manual_override);
    }
}

void tuinLamp::applyRelay() {
    digitalWrite(pin_relay, isActive());
}