#pragma once
#include <sapi.h>
#include <string>
#pragma comment(lib, "sapi.lib")

class Text2Speech {
public:
    Text2Speech();
    ~Text2Speech();
    void speak(const std::string& text);
    void stop(); // <<< ADD THIS LINE

private:
    ISpVoice* pVoice;
    HRESULT hr;
};