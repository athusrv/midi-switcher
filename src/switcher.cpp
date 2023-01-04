#include "switcher.h"

#define FILE "/switcher.json"

namespace swtch {

SwitcherImpl::SwitcherImpl() : JSON() {
}

SwitcherImpl::~SwitcherImpl() {}

bool SwitcherImpl::begin() {
    if (!ready) {
        if (!SPIFFS.begin(true)) {
            console::log(F("Failed to mount SPIFFS for Switcher"));
            return false;
        }

        if (!SPIFFS.exists(FILE)) {
            console::log(F("There's no switcher.json file. Abort!"));
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

        uint8_t* ps = NULL;
        uint8_t len;
        pins(&ps, &len);

        for(uint8_t i = 0; i < len; i++) {
            pinMode(ps[i], OUTPUT);
            digitalWrite(ps[i], HIGH);
        }

        free(ps);

        ready = true;
    }

    return ready;
}

uint8_t SwitcherImpl::count() {
    double* count = get<double>("count");
    if (count == NULL) return 0;

    return *count;
}

void SwitcherImpl::info(uint8_t index, switcher_info_t* info) {
    if (info == NULL) return;

    uint8_t il = common::digits_count(index) + 1;

    size_t pkl = il + 15;
    char* pinKey = (char*)malloc(pkl * sizeof(char));
    if (!pinKey) return;
    snprintf(pinKey, pkl, "switchers.%d.pin", index);

    size_t dkl = il + 18;
    char* dimmerKey = (char*)malloc(dkl * sizeof(char));
    if (!dimmerKey) {
        free(pinKey);
        return;
    }
    snprintf(dimmerKey, dkl, "switchers.%d.dimmer", index);

    double* pin = get<double>(pinKey);
    bool* dimmer = get<bool>(dimmerKey);

    if (pin != NULL && dimmer != NULL) {
        info->pin = *pin;
        info->dimmer = *dimmer;
    }

    free(pinKey);
    free(dimmerKey);
}

void SwitcherImpl::pins(uint8_t** p, uint8_t* len) {
    *len = count();
    if (*p == NULL) {
        *p = (uint8_t*)malloc(*len * sizeof(uint8_t));
    } else {
        *p = (uint8_t*)realloc(*p, *len * sizeof(uint8_t));
    }

    for (uint8_t i = 0; i < *len; i++) {
        switcher_info_t info;
        this->info(i + 1, &info);
        (*p)[i] = info.pin;
    }
}

bool SwitcherImpl::empty(const char* chain, bool expect_str) {
    void** data = get<void*>(chain);
    if (!data)
        return true;

    if (expect_str) {
        return strcmp((const char*)*data, "") == 0;
    }
    return false;
}

SwitcherImpl switcher;
}  // namespace swtch