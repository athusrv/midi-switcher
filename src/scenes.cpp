#include "scenes.h"

#define FILE "/scenes.json"

namespace scn {

ScenesImpl::ScenesImpl() : JSON() {}
ScenesImpl::~ScenesImpl() {}

bool ScenesImpl::begin() {
    if (!ready) {
        if (!switcher.begin()) {
            console::log(F("Failed to initialize the switcher"));
            return false;
        }

        if (!SPIFFS.begin(true)) {
            console::log(F("Failed to mount SPIFFS for Scenes"));
            return false;
        }

        if (!SPIFFS.exists(FILE)) {
            console::log(F("There's no scenes.json file. Abort!"));
            return false;
        }
        File file;
        file = SPIFFS.open(FILE, FILE_READ);
        char* json = (char*)calloc(0, sizeof(char));
        unsigned int count = 0;

        while (file.available()) {
            char c = (char)file.read();
            count++;
            json = (char*)realloc(json, count * sizeof(char));

            if (!json) return false;
            json[count - 1] = c;
        }
        file.close();
        parse(json);

        free(json);
        console::log(F("Scenes are successfully loaded"));
        ready = true;
    }

    return ready;
}

bool ScenesImpl::save() {
    if (!SPIFFS.remove(FILE)) {
        console::log(F("Failed to remove the old scenes file from SPIFFS"));
        return false;
    }
    char* settings = cJSON_PrintUnformatted(json);
    File file = SPIFFS.open(FILE, FILE_WRITE);
    if (!file)
        return false;

    file.println(settings);
    file.close();

    return true;
}

void ScenesImpl::execute(uint8_t channel, uint8_t note, const char* type, uint8_t intensity) {
    uint8_t cl = common::digits_count(channel);
    uint8_t nl = common::digits_count(note);
    uint8_t il = common::digits_count(intensity);

    size_t kl = cl + nl + strlen(type) + il + 4;
    char* key = (char*)malloc(kl * sizeof(char));
    snprintf(key, kl, "%d/%d/%s/%d", channel, note, type, intensity);
    if (!key) {
        console::log("failed to allocate memory for the scene key");
        return;
    }

    console::log("looking for scene key:", key);
    cJSON* states = get<cJSON>(key);
    free(key);

    if (!states) {
        console::log("could not find the scene for key:", key);
        return;
    }

    uint8_t* pins = NULL;
    uint8_t len;
    switcher.pins(&pins, &len);

    for (uint8_t i = 0; i < len; i++) {
        pinMode(pins[i], OUTPUT);
        cJSON* item = cJSON_GetArrayItem(states, i);
        if (item) {
            digitalWrite(pins[i], (uint8_t)item->valuedouble);
        }
    }

    free(pins);
    console::log("scene execution is complete");
}

ScenesImpl scenes;
}  // namespace scn