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

void tuinLamp::setButton(button *btn) {
    m_button = btn;
}

bool tuinLamp::isActive() {
    if (m_manual_force_on) {
        Serial.print(m_manual_override_duration);
        if (m_manual_override_start + m_manual_override_duration > m_time->now()) {
            Serial.print("*");
            return true;
        } else {
            m_manual_force_on = false;
        }
    }
    if (m_manual_override) {
        Serial.print(".");
        return !m_autoActive;
    } else {
        return m_autoActive;
    }
}

void tuinLamp::pins(uint8_t led, uint8_t relay) {
    pin_led = led;
    pin_relay = relay;

    pinMode(pin_led, OUTPUT);
    pinMode(pin_relay, OUTPUT);
}

void tuinLamp::payload() {
    // Test button
    if (m_button->isClicked()) {
        m_manual_override = !m_manual_override;
    }

    if (m_button->isDoubleClicked()) {
        m_manual_override = false;
        m_manual_force_on = true;
        m_manual_override_start = m_time->now();
    }

    if (m_button->isLongPress()) {
        m_manual_override = false;
        m_manual_force_on = false;
    }

    time_t n = m_time->now();
    bool autoActive = m_schedule->isActive(*m_sun, n);
    if (m_autoActive != autoActive) {
        // Autoactive changed => clear manual override
        m_autoActive = autoActive;
        m_manual_override = false;
    }

    debug("tuinLamp active: ");
    debugln(isActive());

    if (isActive()) {
        Serial.print("+");
    } else {
        Serial.print("-");
    }

    // Set output
    digitalWrite(pin_led, !isActive());
    digitalWrite(pin_relay, isActive());
}
