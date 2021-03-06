#include "button.h"

void button::begin() {
    pinMode(m_pin, m_pullup ? INPUT_PULLUP : INPUT);
}

void button::loop() {
    // Reset state
    // m_isClicked = false;
    // m_isDoubleClick = false;
    // m_isLongPress = false;

    if (millis() + m_rebounce_duration < m_pressed_ms) {
        // Rebounce protection
        return;
    }

    bool state = digitalRead(m_pin) ? !m_pullup : m_pullup;

    if (state) {
        // Button is down
        if (prevState) {
            // Still pressed
            if (m_pressed_ms + m_longpress_duration < millis()) {
                // long press
                Serial.println("Longpress");
                m_isLongPress = true;
                m_isClicked = false;
                m_isPressed = false;
                m_surpressClick = true;
            }
        } else {
            // First time button is down
            m_pressed_ms = millis();
            m_isPressed = true;
            Serial.println("Press");

        }
    } else {
        // Button is up
        if (prevState) {
            // Button is released
            // Check for doubleclick
            if (m_released_ms + m_doubleclick_duration > millis()) {
                // Within doublepress interval
                Serial.println("Doubleclick");
                m_isDoubleClick = true;
                m_surpressClick = true;
                m_isClicked = false;
            }

            if (m_surpressClick) {
                m_surpressClick = false;
            } else {
                m_isClicked = true;
                Serial.println("Clicked  = Released");
            }
            m_released_ms = millis();
            m_pressed_ms = millis(); // Rebound protection
        } else {
            // Button was already up
        }
        m_isPressed = false;
    }
    prevState = state;
}

button::button(int pin) {
    m_pin = pin;
}

button::~button() {
}
