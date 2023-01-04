#include <midi/midi.h>
#include <network/network.h>
#include <switcher.h>
#include <scenes.h>

bool netReady = false;
bool midiReady = false;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    console::begin(115200);

    if (!network.begin() || !settings.begin() || !scenes.begin()) {
        return;
    }

    netReady = true;

    network.connect([]() {
        if (!cmidi.begin()) {
            return;
        }

        console::log(F("MIDI is setup"));
        midiReady = true;
    });

    cmidi.onConnect([](uint32_t id, const char* name) {
        console::log(F("Connected to session ID = "), id, " and Name = ", name);
    });

    cmidi.onDisconnect([](uint32_t id) {
        console::log(F("Disconnected from session "), id);
    });

    cmidi.onData([](MIDINoteType type, byte channel, byte note, byte intensity) {
        scenes.execute(channel, note, type == MIDINoteType::OFF ? "off" : "on", intensity);
    });
}

void loop() {
    if (netReady) {
        network.process();

        if (network.isConnectedToWiFi() && midiReady) {
            cmidi.process();
        }
    }
}