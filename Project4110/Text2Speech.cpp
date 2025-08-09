#include "Text2Speech.h"
#include <sapi.h>   

// Using Microsoft Narrator
void Text2Speech::speak(const std::string& message) {
    ISpVoice* pVoice = nullptr;
    if (FAILED(::CoInitialize(nullptr))) return;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr)) {
        std::wstring wtext(message.begin(), message.end());
        pVoice->Speak(wtext.c_str(), 0, nullptr);
        pVoice->Release();
    }
    ::CoUninitialize();
}

