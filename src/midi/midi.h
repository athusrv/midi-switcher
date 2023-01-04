#ifndef MIDI_SWITCHER_MIDI
#define MIDI_SWITCHER_MIDI

#define ONE_PARTICIPANT

#include <AppleMIDI.h>
#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "../console.h"

enum MIDINoteType {
  ON,
  OFF
};

namespace wmidi {
using namespace std;

class MIDIImpl {
 private:
  function<void(uint32_t, const char *)> onConnectCallback;
  function<void(uint32_t)> onDisconnectCallback;
  function<void(MIDINoteType, byte, byte, byte)> onDataCallback;

  static function<void(const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *)> onConnectHelper;
  static function<void(const APPLEMIDI_NAMESPACE::ssrc_t &ssrc)> onDisconnectHelper;
  static function<void(MIDINoteType, byte, byte, byte)> onDataHelper;

  void onConnect_(const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *);
  void onDisconnect_(const APPLEMIDI_NAMESPACE::ssrc_t &ssrc);
  void onData_(MIDINoteType, byte, byte, byte);

 public:
  MIDIImpl();
  virtual ~MIDIImpl();
  bool begin();
  void onConnect(function<void(uint32_t id, const char *name)>);
  void onDisconnect(function<void(uint32_t id)>);
  void onData(function<void(MIDINoteType, byte, byte, byte)>);
  void process();
};
extern wmidi::MIDIImpl cmidi;
}  // namespace wmidi

using namespace wmidi;
#endif