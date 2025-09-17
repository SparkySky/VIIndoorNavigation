#include "Text2Speech.h"

using namespace std;

// Constructor: Initializes COM and creates the voice instance ONCE.
Text2Speech::Text2Speech() {
    pVoice = nullptr;
    if (FAILED(::CoInitialize(nullptr))) {
        // Handle error if needed
        return;
    }
    hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
}

// Destructor: Releases the voice instance and uninitializes COM.
Text2Speech::~Text2Speech() {
    if (pVoice) {
        pVoice->Release();
        pVoice = nullptr;
    }
    ::CoUninitialize();
}

// speak: Sends text to be spoken asynchronously.
void Text2Speech::speak(const std::string& message) {
    if (SUCCEEDED(hr) && pVoice) {
        std::wstring wtext(message.begin(), message.end());
        // SPF_ASYNC makes the call non-blocking.
        // SVSFPurgeBeforeSpeak cancels any previous unfinished speech.
        pVoice->Speak(wtext.c_str(), SPF_ASYNC | SVSFPurgeBeforeSpeak, nullptr);
    }
}

// stop: Immediately interrupts any currently playing speech.
void Text2Speech::stop() {
    if (SUCCEEDED(hr) && pVoice) {
        // Speaking an empty string with Purge flag is the SAPI way to stop speech.
        pVoice->Speak(L"", SVSFPurgeBeforeSpeak, nullptr);
    }
}