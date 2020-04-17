#include "tuinLamp.h"

#include "Dusk2Dawn.h"
#include "ezTime.h"

float calcRunrise(float lat, float lon, time_t time, float offset) {
    return 0;
}

tuinLamp::tuinLamp(/* args */) {    
}

tuinLamp::~tuinLamp() {
}

void tuinLamp::prepare() {
    Serial.println("tuinLamp::prepare");
    m_sun = 0;
    m_time = 0;
    m_schedule= 0;
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
    if (m_manual_override) {
        return !m_autoActive;
    }
    return m_autoActive;
}

void tuinLamp::pins(uint8_t led, uint8_t relay) {
    pin_led = led;
    pin_relay = relay;

    pinMode(pin_led, OUTPUT);
    pinMode(pin_relay, OUTPUT);
}

void tuinLamp::payload() {

    if (m_button->isClicked()) {
        m_manual_override = !m_manual_override;
    }

    time_t n = m_time->now();
    bool autoActive = m_schedule->isActive(*m_sun , n);
    if (m_autoActive != autoActive) {
        // Autoactive changed => clear manual override
        m_autoActive = autoActive;
        m_manual_override = false;
    }

    digitalWrite(pin_led, isActive());
}