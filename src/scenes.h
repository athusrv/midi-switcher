#ifndef MIDI_SWITCHER_SCENES_H
#define MIDI_SWITCHER_SCENES_H

#include <Arduino.h>
#include <SPIFFS.h>

#include "console.h"
#include "json.h"
#include "switcher.h"
#include "common.h"

namespace scn {
struct scene_comb_t {
    uint8_t channel, note, intensity;
    const char* type;
};

class ScenesImpl : public JSON {
   private:
    bool ready = false;

   public:
    ScenesImpl();
    virtual ~ScenesImpl();
    bool begin();
    bool save();
    void execute(uint8_t channel, uint8_t note, const char* type, uint8_t intensity);
};

extern scn::ScenesImpl scenes;
}  // namespace scn

using namespace scn;

#endif