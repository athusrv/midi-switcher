#include "settings.h"

#define FILE "/config.json"

namespace stngs {

SettingsImpl::SettingsImpl() : JSON() {}

SettingsImpl::~SettingsImpl() {}

bool SettingsImpl::begin() {
  if (!ready) {
    if (!SPIFFS.begin(true)) {
      console::log(F("Failed to mount SPIFFS for Settings"));
      return false;
    }

    if (!SPIFFS.exists(FILE)) {
      console::log(F("There's no config.json file. Abort!"));
      return false;
    }
    File file;
    file = SPIFFS.open(FILE, FILE_READ);
    char *json = (char*) calloc(0, sizeof(char));
    unsigned int count = 0;

    while (file.available()) {
      char c = (char)file.read();
      count++;
      json = (char *)realloc(json, count * sizeof(char));

      if (!json) return false;
      json[count - 1] = c;
    }
    file.close();
    parse(json);
    free(json);
    console::log(F("Settings are successfully loaded"));
    ready = true;
  }

  return ready;
}

bool SettingsImpl::save() {
  if (!SPIFFS.remove(FILE)) {
    console::log(F("Failed to remove the old configuration file from SPIFFS"));
    return false;
  }
  char *settings = cJSON_PrintUnformatted(json);
  File file = SPIFFS.open(FILE, FILE_WRITE);
  if (!file)
    return false;

  file.println(settings);
  file.close();

  return true;
}

bool SettingsImpl::empty(const char *chain, bool expect_str) {
  void **data = get<void *>(chain);
  if (!data)
    return true;

  if (expect_str) {
    return strcmp((const char *)*data, "") == 0;
  }
  return false;
}

SettingsImpl settings;

}  // namespace stngs