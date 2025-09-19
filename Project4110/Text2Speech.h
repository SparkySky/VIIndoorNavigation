// In Text2Speech.h
#pragma once
#include <string>
#include <sapi.h>

class Text2Speech {
public:
    // ADD THIS ENUM
    enum VoiceState {
        IDLE,
        SPEAKING_LOW_PRIORITY,
        SPEAKING_HIGH_PRIORITY
    };

    Text2Speech();
    ~Text2Speech();

    void speak_low_priority(const std::string& message);
    void speak_high_priority(const std::string& message);
    bool is_speaking();
    void stop();

    // ADD THIS NEW FUNCTION
    void update_status();

private:
    ISpVoice* pVoice;
    HRESULT hr;

    // ADD THIS STATE VARIABLE
    VoiceState currentState;
};