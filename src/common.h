#ifndef MIDI_SWITCHER_COMMON_H
#define MIDI_SWITCHER_COMMON_H

#include <math.h>

namespace common {
static uint8_t digits_count(uint8_t n) {
    if (n == 0) return 1;
    return floor(log10(n) + 1);
}
}  // namespace common
#endif