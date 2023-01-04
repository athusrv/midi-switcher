#ifndef MIDI_SWITCHER_SETTINGS_H
#define MIDI_SWITCHER_SETTINGS_H

#include <Arduino.h>
#include <SPIFFS.h>

#include "console.h"
#include "json.h"

namespace stngs {
class SettingsImpl : public JSON {
private:
    bool ready = false;
    bool empty(const char* chain, bool expect_str = true);

public:
    SettingsImpl();
    virtual ~SettingsImpl();

    bool begin();
    bool save();
};

extern stngs::SettingsImpl settings;
} // namespace settings

using namespace stngs;

#endif