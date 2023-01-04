#include "midi.h"

namespace wmidi {
using namespace std;

function<void(const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *)> MIDIImpl::onConnectHelper = NULL;
function<void(const APPLEMIDI_NAMESPACE::ssrc_t &ssrc)> MIDIImpl::onDisconnectHelper = NULL;
function<void(MIDINoteType, byte, byte, byte)> MIDIImpl::onDataHelper = NULL;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "ESP32 MIDI", 5004);

MIDIImpl::MIDIImpl() {

  onConnectHelper = bind(&MIDIImpl::onConnect_, this, placeholders::_1, placeholders::_2);
  onDisconnectHelper = bind(&MIDIImpl::onDisconnect_, this, placeholders::_1);
  onDataHelper = bind(&MIDIImpl::onData_, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4);
}

MIDIImpl::~MIDIImpl() {}

bool MIDIImpl::begin() {
  MIDI.begin();

  if (!MDNS.begin(AppleMIDI.getName())) {
    console::log(F("Error setting up MDNS responder"));
    return false;
  }

  console::log(F("mDNS responder started ("), AppleMIDI.getName(), F(".local)"));
  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *name) {
    // Serial.println("Connected to session ID = " + String(ssrc) + " and Name = " + String(name));
    MIDIImpl::onConnectHelper(ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc) {
    // Serial.println("Disconnected from session " + String(ssrc));
    MIDIImpl::onDisconnectHelper(ssrc);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte intensity) {
    // Serial.println("Note On | Channel = " + String(channel) + " Note = " + String(note) + " Intensity = " + String(intensity));
    MIDIImpl::onDataHelper(MIDINoteType::ON, channel, note, intensity);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte _) {
    // Serial.println("Note Off | Channel = " + String(channel) + " Note = " + String(note));
    MIDIImpl::onDataHelper(MIDINoteType::OFF, channel, note, 0);
  });

  return true;
}

void MIDIImpl::onConnect_(const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *name) {
  if (onConnectCallback) onConnectCallback(ssrc, name);
}

void MIDIImpl::onDisconnect_(const APPLEMIDI_NAMESPACE::ssrc_t &ssrc) {
  if (onDisconnectCallback) onDisconnectCallback(ssrc);
}

void MIDIImpl::onData_(MIDINoteType type, byte channel, byte note, byte intensity) {
  if (onDataCallback) onDataCallback(type, channel, note, intensity);
}

void MIDIImpl::onConnect(function<void(uint32_t id, const char *name)> callback) {
  onConnectCallback = callback;
}

void MIDIImpl::onDisconnect(function<void(uint32_t id)> callback) {
  onDisconnectCallback = callback;
}

void MIDIImpl::onData(function<void(MIDINoteType, byte, byte, byte)> callback) {
  onDataCallback = callback;
}

void MIDIImpl::process() {
  MIDI.read();
}

MIDIImpl cmidi;
}  // namespace wmidi