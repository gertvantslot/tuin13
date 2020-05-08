#include "tuinLamp.h"
#include <EEPROM.h>

#include "Dusk2Dawn.h"
#include "ezTime.h"

#ifdef TESTDEVICE
#define debug(args...) \
    if (m_debug) Serial.print(args)
#define debugln(args...) \
    if (m_debug) Serial.println(args)
#else
#define debug(args...)
#define debugln(args...)
#endif

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

    m_branduren_last_count = m_time->now();
    m_branduren_published = m_time->dayOfYear();
    m_branduren_seconds = 0;
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
    m_manual_override_end = nextMidnight(m_time->now()) + hours * SECS_PER_HOUR + minutes * SECS_PER_MIN;
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

    if (isActive() && m_branduren_last_count != n) {
        m_branduren_last_count = n;
        m_branduren_seconds++;
    }
}

void tuinLamp::applyStatusLeds() {
    // Set output
    // Flash every 2,5 seconds
    int flash = (millis() % 2500) < 100 ? LOW : HIGH;
    int pwm = (isActive() ? flash : !flash) ? 256 : 0;
    analogWrite(pin_active, pwm);

    if (m_manual_force_on) {
        digitalWrite(pin_manual, (millis() / 250) % 2);
    } else {
        digitalWrite(pin_manual, m_manual_override);
    }
}

void tuinLamp::applyRelay() {
    digitalWrite(pin_relay, isActive());
}

bool tuinLamp::brandurenPublish() {
    return m_branduren_published != m_time->dayOfYear();
}

unsigned long tuinLamp::brandurenSeconds() {
    return m_branduren_seconds;
}

void tuinLamp::brandurenMarkPublished() {
    m_branduren_seconds = 0;
    m_branduren_published = m_time->dayOfYear();
}

void tuinLamp::restoreConfig(uint16_t address) {
    m_eeprom_address = address;
    EEPROM.begin(4096);

    bool found = false;

    int offset = address;
    int magic = 0;
    EEPROM.get(offset, magic);
    offset += sizeof(magic);

    // Magic code is correct
    if (magic == MAGIC_SETTINGS_NUMBER) {
        // Get version
        int version = 0;
        EEPROM.get(offset, version);
        offset += sizeof(version);

        if (version == 3) {
            // Version 3
            Serial.println(F("Retrieving configuration"));
            found = true;
            Schedule *ochtend = getSchedule();
            Schedule *avond = ochtend->next();

            offset = ochtend->getFromEEPROM(offset);
            ochtend->printStatus();
            offset = avond->getFromEEPROM(offset);
            avond->printStatus();
            Serial.println(F("Configuration restored"));
        }
    } 
    EEPROM.end();
    
    if (!found) {
        Serial.println(F("No configuration stored"));
    }
}

void tuinLamp::storeConfig() {
    if (m_eeprom_address == SETTINGS_UNKNOWN) {
        return; // No address set
    }
    Serial.println("Storing configuration");
    int offset = m_eeprom_address;
    EEPROM.begin(4096);

    int magic = MAGIC_SETTINGS_NUMBER;
    int version = 3;
    EEPROM.put(offset, magic);
    offset += sizeof(magic);
    EEPROM.put(offset, version);
    offset += sizeof(version);

    Schedule *s = getSchedule();
    offset = s->putInEEPROM(offset);
    s = s->next();
    offset = s->putInEEPROM(offset); 

    EEPROM.commit();
    EEPROM.end();
}
