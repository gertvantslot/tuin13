#pragma once

#include <Arduino.h>

class button
{
private:
    /* data */
    int m_pin;
    bool m_pullup = true;

    uint16_t m_rebounce_duration = 50;
    uint16_t m_longpress_duration = 500;
    uint16_t m_doubleclick_duration = 500;

    bool prevState = false;
    bool m_isPressed = false;
    bool m_isClicked = false;
    bool m_surpressClick = false;
    
    bool m_isDoubleClick = false;
    bool m_isLongPress = false;

    unsigned long m_pressed_ms;
    unsigned long m_released_ms;
public:
    button(int pin);
    ~button();

    bool isPressed() { if (!m_isPressed) return false; m_isPressed = false; return true; } 
    bool isClicked() { if (!m_isClicked) return false; m_isClicked = false; return true; }
    bool isDoubleClicked() { if (!m_isDoubleClick) return false; m_isDoubleClick = false; return true; }
    bool isLongPress() { if (!m_isLongPress) return false; m_isLongPress = false; return true; }

    void begin();
    void loop();
};
