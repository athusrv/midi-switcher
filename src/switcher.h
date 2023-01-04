#ifndef MIDI_SWITCHER_DEVICE_H
#define MIDI_SWITCHER_DEVICE_H

#include <Arduino.h>
#include <SPIFFS.h>

#include "json.h"
#include "console.h"
#include "common.h"

namespace swtch {

struct switcher_info_t {
    uint8_t pin;
    bool dimmer;
};

class SwitcherImpl : public JSON {
   private:
    bool ready = false;
    bool empty(const char* chain, bool expect_str = true);

   public:
    SwitcherImpl();
    virtual ~SwitcherImpl();
    bool begin();
    uint8_t count();
    void info(uint8_t index, switcher_info_t* info);
    void pins(uint8_t** p, uint8_t* len);
};
extern swtch::SwitcherImpl switcher;
}  // namespace swtch

using namespace swtch;

#endif