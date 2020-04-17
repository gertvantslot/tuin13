#include "button.h"

void button::begin() {
    pinMode(m_pin, m_pullup ? INPUT_PULLUP : INPUT);
}

void button::loop() {
    // Reset state
    m_isClicked = false;
    m_isDoubleClick = false;
    m_isLongPress = false;

    if (millis() + m_rebounce_duration < m_pressed_ms) {
        // Rebounce protection
        return;
    }

    bool pressed = digitalRead(m_pin) ? !m_pullup : m_pullup;

    if (pressed) {
        // Button is down
        if (m_isPressed) {
            // Still pressed
            if (m_pressed_ms + m_longpress_duration < millis()) {
                // long press
                m_isLongPress = true;
            }
        } else {
            // First time button is down
            m_pressed_ms = millis();
            m_isPressed = true;

            // Check for doubleclick
            if (m_released_ms + m_doubleclick_duration < millis()) {
                // Within doublepress interval
                m_isDoubleClick = true;
            }
        }
    } else {
        // Button is up
        if (m_isPressed) {
            // Button is released
            m_isPressed = false;
            m_isClicked = true;
            m_released_ms = millis();
        } else {
            // Button was already up
        }
        m_isPressed = false;
    }
}

button::button(int pin) {
    m_pin = pin;
}

button::~button() {
}
