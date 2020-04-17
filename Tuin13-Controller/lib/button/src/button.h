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

    bool m_isPressed = false;
    bool m_isClicked = false;
    bool m_isDoubleClick = false;
    bool m_isLongPress = false;

    unsigned long m_pressed_ms;
    unsigned long m_released_ms;
public:
    button(int pin);
    ~button();

    bool isPressed() { return m_isPressed; } 
    bool isClicked() { return m_isClicked; }
    bool isDoubleClicked() { return m_isDoubleClick; }
    bool isLongPress() { return m_isLongPress; }

    void begin();
    void loop();
};
