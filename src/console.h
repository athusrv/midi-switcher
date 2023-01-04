#ifndef MIDI_SWITCHER_CONSOLE_H
#define MIDI_SWITCHER_CONSOLE_H

#include <Arduino.h>

namespace console {

static void begin(unsigned long baud) {
  Serial.begin(baud);
  while (!Serial)
    ;
}

template <typename T>
static void plain(T last) {
  Serial.println(last);
}

template <typename T, typename... Args>
static void plain(T head, Args... tail) {
  Serial.print(head);
  Serial.print(' ');
  plain(tail...);
}

template <typename... Args>
static void log(Args... args) {
  plain(args...);
}

}  // namespace console

#endif
