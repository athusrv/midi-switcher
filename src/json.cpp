#include "json.h"

JSON::JSON() : JSON("{}") {}

JSON::JSON(const char *data) { this->parse(data); }

JSON::~JSON() {}

void JSON::parse(const char *json) { this->json = cJSON_Parse(json); }

char *JSON::stringfy() { return cJSON_PrintUnformatted(raw()); }

cJSON *JSON::raw() {
    if (!is_valid()) {
        return NULL;
    }
    return json;
}

bool JSON::contains(const char *chain) {
    const char *chains[1] = {chain};
    return contains(chains, 1);
}

bool JSON::contains(const char **chains, size_t len, char ***mc, size_t *mcc) {
    size_t _mcc = 0;
    for (size_t i = 0; i < len; i++) {
        void **item = get<void *>(chains[i]);
        if (!item) {
            if (mc) {
                mcc++;
                *mc = (char **)realloc(mc, _mcc * sizeof(char *));
                *(mc[_mcc - 1]) = (char *)chains[i];
            } else
                return false;
        }
    }

    if (mc && _mcc > 0) {
        if (mcc)
            *mcc = _mcc;
        return false;
    }
    return true;
}

bool JSON::is_valid() { return !cJSON_IsInvalid(json); }

void JSON::split(const char *keychain, const char *sep, char ***keys, unsigned int *length) {
    if (*keys == NULL) {
        *keys = (char **)calloc(0, sizeof(char *));
    }

    char *chain = (char *)malloc((strlen(keychain) + 1) * sizeof(char));
    if (chain == NULL) {
        console::log("Error allocating memory for JSON handling");
        return;
    }
    strcpy(chain, keychain);
    char *c = strtok(chain, sep);
    unsigned int count = 0;
    while (c != NULL) {
        count++;
        *keys = (char **)realloc(*keys, count * sizeof(char *));
        if (*keys != NULL)
            (*keys)[count - 1] = c;
        c = strtok(NULL, sep);
    }

    *length = count;
    free(chain);
}

cJSON *JSON::getItem(const char *keychain) {
    cJSON *item = NULL;
    try {
        unsigned int length = 0;
        char **keys = NULL;
        split(keychain, ".", &keys, &length);
        if (!keys) {
            return NULL;
        }

        for (uint8_t i = 0; i < length; i++) {
            cJSON *prev = item;
            if (prev == NULL) {
                prev = json;
            }

            item = cJSON_GetObjectItem(prev, keys[i]);
            if (!item) {
                item = cJSON_AddObjectToObject(prev, keys[i]);
            }
        }

        free(keys);

        if (cJSON_IsInvalid(item)) {
            return NULL;
        }

        return item;
    } catch (int e) {
        return NULL;
    }
}

bool JSON::set(const char *keychain, const char *value) {
    try {
        cJSON *item = getItem(keychain);
        if (!item) {
            return false;
        }
        item->type = cJSON_String;
        item->valuestring = (char *)value;
        return true;
    } catch (int e) {
        return false;
    }
}

bool JSON::set(const char *keychain, bool value) {
    try {
        cJSON *item = getItem(keychain);
        if (value)
            item->type = cJSON_True;
        else
            item->type = cJSON_False;
        return true;
    } catch (int e) {
        return false;
    }
}

bool JSON::set(const char *keychain, double value) {
    try {
        cJSON *item = getItem(keychain);
        if (item == NULL) {
            return false;
        }
        item->type = cJSON_Number;
        cJSON_SetNumberHelper(item, value);
        return true;
    } catch (int e) {
        return false;
    }
}

void JSON::destroy() {
    if (json != NULL) {
        cJSON_Delete(json);
    }
}