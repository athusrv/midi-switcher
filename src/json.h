#ifndef MIDI_SWITCHER_JSON_H
#define MIDI_SWITCHER_JSON_H

#include <Arduino.h>
#include <cJSON.h>

#include "console.h"

class JSON {
   protected:
    cJSON *json;

   private:
    void split(const char *keychain, const char *sep, char ***keys, unsigned int *length);
    cJSON *getItem(const char *keychain);

   public:
    JSON();
    JSON(const char *data);
    virtual ~JSON();

    void parse(const char *json);
    char *stringfy();
    bool is_valid();
    cJSON *raw();
    bool contains(const char *chain);
    bool contains(const char **chains, size_t len, char ***mc = NULL, size_t *mcc = NULL);

    template <typename T>
    T *get(const char *chain) {
        if (cJSON_IsInvalid(json)) {
            return NULL;
        }
        cJSON *item = NULL;
        try {
            unsigned int length = 0;
            char **keys = NULL;
            split(chain, ".", &keys, &length);
            if(!keys) {
                return NULL;
            }

            for (uint8_t i = 0; i < length; i++) {
                if (item == NULL) {
                    item = cJSON_GetObjectItem(json, keys[i]);
                } else {
                    item = cJSON_GetObjectItem(item, keys[i]);
                }
            }

            free(keys);

            if (cJSON_IsInvalid(item)) {
                return NULL;
            }

            void *value = NULL;
            if (cJSON_IsNumber(item)) {
                value = &item->valuedouble;
            } else if (cJSON_IsString(item)) {
                value = &item->valuestring;
            } else if (cJSON_IsBool(item)) {
                static bool b;
                if (cJSON_IsTrue(item))
                    b = true;
                else
                    b = false;
                value = &b;
            } else if (cJSON_IsArray(item) || cJSON_IsObject(item)) {
                value = item;
            }
            return static_cast<T *>(value);
        } catch (int e) {
            if (item != NULL) {
                console::log(F("Failed to get or parse "), item->string, F(" from JSON object; "), e);
            } else {
                console::log(F("Failed to process JSON serialization; "), e);
            }
        }
        return NULL;
    }

    virtual bool set(const char *keychain, const char *value);
    virtual bool set(const char *keychain, bool value);
    virtual bool set(const char *keychain, double value);

    void destroy();
};

#endif