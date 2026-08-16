#pragma once

#include <Arduino.h>

enum class BuzzerType 
{
    ACTIVE,
    PASSIVE
};

class Buzzer 
{
    public:
        Buzzer(uint8_t pin, BuzzerType type);

        void begin();
        void start();
        void stop();
        void update();
        bool isActive() const;

    private:
        uint8_t pin;
        BuzzerType type;

        bool running = false;
        bool beepOn = false;
        unsigned long lastToggle = 0;
        uint16_t cycleCount = 0;

        // Réglages du pattern d'escalade — ajuste librement selon le rendu voulu
        static constexpr uint16_t BEEP_ON_MS = 300;         // durée d'un bip
        static constexpr uint16_t BEEP_OFF_MS_START = 700;  // pause initiale entre bips
        static constexpr uint16_t BEEP_OFF_MS_MIN = 150;    // pause minimale (rythme final)
        static constexpr uint16_t ESCALATE_EVERY_CYCLES = 4; // accélère tous les N bips
        static constexpr uint16_t ESCALATE_STEP_MS = 100;    // réduction de pause à chaque palier
        static constexpr uint16_t TONE_FREQ_START = 1500;    // Hz, uniquement pour PASSIVE
        static constexpr uint16_t TONE_FREQ_MAX = 2500;
        static constexpr uint16_t TONE_FREQ_STEP = 100;

        void setPin(bool on);
        uint16_t currentOffDuration() const;
        uint16_t currentToneFreq() const;
};

inline Buzzer::Buzzer(uint8_t pin, BuzzerType type) : pin(pin), type(type) {}

inline void Buzzer::begin() 
{
    pinMode(pin, OUTPUT);
    setPin(false);
}

inline void Buzzer::start() {
    running = true;
    beepOn = false;
    cycleCount = 0;
    lastToggle = millis();
}

inline void Buzzer::stop() 
{
    running = false;
    setPin(false);
}

inline void Buzzer::update() 
{
    if (!running) return;

    unsigned long now = millis();
    unsigned long elapsed = now - lastToggle;

    if (beepOn) 
    {
        // Bip en cours : on attend la fin avant de couper
        if (elapsed >= BEEP_ON_MS) 
        {
            setPin(false);
            beepOn = false;
            lastToggle = now;
            cycleCount++;
        }
    } 
    else {
        // Silence en cours : on attend la pause (qui se réduit avec le temps)
        if (elapsed >= currentOffDuration()) 
        {
            setPin(true);
            beepOn = true;
            lastToggle = now;
        }
    }
}

inline void Buzzer::setPin(bool on) 
{
    if (type == BuzzerType::ACTIVE) 
    {
        digitalWrite(pin, on ? HIGH : LOW);
    } 
    else 
    {
        if (on) 
        {
            tone(pin, currentToneFreq());
        } 
        else 
        {
            noTone(pin);
        }
    }
}

inline uint16_t Buzzer::currentOffDuration() const 
{
    uint16_t steps = cycleCount / ESCALATE_EVERY_CYCLES;
    int32_t off = (int32_t)BEEP_OFF_MS_START - steps * ESCALATE_STEP_MS;
    if (off < BEEP_OFF_MS_MIN) off = BEEP_OFF_MS_MIN;
    return (uint16_t)off;
}

inline uint16_t Buzzer::currentToneFreq() const 
{
    uint16_t steps = cycleCount / ESCALATE_EVERY_CYCLES;
    uint32_t freq = TONE_FREQ_START + steps * TONE_FREQ_STEP;
    if (freq > TONE_FREQ_MAX) freq = TONE_FREQ_MAX;
    return (uint16_t)freq;
}

inline bool Buzzer::isActive() const 
{
    return running;
}