#include "Text2Speech.h"

#include <windows.h>
#include <sapi.h>
#include <iostream>

// In Text2Speech.cpp
Text2Speech::Text2Speech() {
    pVoice = nullptr;
    if (FAILED(::CoInitialize(nullptr))) {
        std::cout << "DEBUG: COM initialization FAILED." << std::endl;
        return;
    }
    hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (FAILED(hr)) {
        std::cout << "DEBUG: Failed to create SAPI voice instance." << std::endl;
    }
    else {
        std::cout << "DEBUG: SAPI voice created successfully." << std::endl;
    }
    currentState = IDLE;
}

Text2Speech::~Text2Speech() {
    if (pVoice) {
        pVoice->Release();
        pVoice = nullptr;
    }
    ::CoUninitialize();
}

// NEW function that must be called in your main loop
void Text2Speech::update_status() {
    // If we think we're speaking, check if the voice has actually finished.
    // This resets our state back to IDLE when speech is complete.
    if (currentState != IDLE && !is_speaking()) {
        currentState = IDLE;
    }
}

// Checks if the voice is currently speaking.
bool Text2Speech::is_speaking() {
    if (!pVoice) return false;
    SPVOICESTATUS status;
    pVoice->GetStatus(&status, nullptr);
    return (status.dwRunningState == 1);
}

// HIGH PRIORITY: Interrupts low-priority speech and BLOCKS until finished.
void Text2Speech::speak_high_priority(const std::string& message) {
    if (SUCCEEDED(hr) && pVoice) {
        // This function is blocking, so we can manage the state here.
        currentState = SPEAKING_HIGH_PRIORITY;

        std::wstring wtext(message.begin(), message.end());
        pVoice->Speak(wtext.c_str(), SVSFPurgeBeforeSpeak, nullptr);

        // Once Speak() returns, it's done. Reset state to IDLE.
        currentState = IDLE;
    }
}

// LOW PRIORITY: Plays in the background, does NOT interrupt, and will NOT play if another sound is active.
void Text2Speech::speak_low_priority(const std::string& message) {
    if (SUCCEEDED(hr) && pVoice) {
        // Do NOT interrupt a high-priority message.
        if (currentState == SPEAKING_HIGH_PRIORITY) {
            return;
        }

        std::wstring wtext(message.begin(), message.end());

        // Use Purge flag to interrupt other low-priority messages.
        HRESULT speakHr = pVoice->Speak(wtext.c_str(), SPF_ASYNC | SVSFPurgeBeforeSpeak, nullptr);

        if (SUCCEEDED(speakHr)) {
            // If we successfully started speaking, update our state.
            currentState = SPEAKING_LOW_PRIORITY;
        }
    }
}

// ONLY can stop speak_low_priority()
void Text2Speech::stop() {
    if (SUCCEEDED(hr) && pVoice) {
        // Speaking an empty string with SVSFPurgeBeforeSpeak is the standard way to halt speech
        pVoice->Speak(L"", SVSFPurgeBeforeSpeak, nullptr);
    }
}