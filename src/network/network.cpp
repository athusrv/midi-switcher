#include "network.h"

namespace net {
using namespace std;

HTTPClient *NetworkImpl::httpClient = NULL;

NetworkImpl::NetworkImpl() {}

NetworkImpl::~NetworkImpl() {}

bool NetworkImpl::begin() {
    console::log(F("Configuring WiFi mode to AP_STA"));
    if (!WiFi.mode(WIFI_AP_STA)) {
        console::log(F("Error configuring WiFi mode to AP_STA"));
        return false;
    }

    console::log(F("Creating Acess Point"));
    WiFi.softAPConfig(accessPointIP, accessPointIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("MIDI Lights");

    console::log(F("Starting DNS server"));
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    if (!dnsServer.start(DNS_PORT, "*", accessPointIP)) {
        console::log(F("Could not start the DNS server"));
        return false;
    }
    console::log(F("DNS Server started successfully"));

    console::log(F("Creating Captive Portal"));
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    console::log(F("Serving static files"));
    server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html").setFilter(ON_AP_FILTER);
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(SPIFFS, "/www/index.html");
        }
    });
    console::log(F("Configuring CORS"));
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "DELETE, POST, PUT, GET, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    server.begin();
    console::log(F("Server started"));

    server.on("^\\/switchers\\/([0-9]+)\\/([a-z]+)$", HTTP_GET, bind(&NetworkImpl::set_switcher_state, this, placeholders::_1));
    server.on("/switchers", HTTP_GET, bind(&NetworkImpl::get_switchers_info, this, placeholders::_1));

    return true;
}

void NetworkImpl::onDisconnect(function<void()> callback) {
    onDisconnectEventCallback = callback;
}

bool NetworkImpl::isConnectedToWiFi() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkImpl::onConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    console::log(F("Connected to WiFi"));
    if (onConnectEventCallback) onConnectEventCallback();
}

void NetworkImpl::connect(function<void()> callback) {
    char **ssid = settings.get<char *>("wifi.ssid");
    char **password = settings.get<char *>("wifi.password");
    if (!ssid || strcmp(*ssid, "") == 0 || !password || strcmp(*password, "") == 0) {
        console::log(F("SSID or Password is null or empty"));
        return;
    }

    onConnectEventCallback = callback;

    console::log(F("Connecting to WiFi ("), *ssid, ").");
    WiFi.onEvent(bind(&NetworkImpl::onConnected, this, placeholders::_1, placeholders::_2), WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent([](arduino_event_id_t event, arduino_event_info_t info) {
        console::log(F("Device IP is"), WiFi.localIP());
    },
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.begin(*ssid, *password);
}

void NetworkImpl::get_switchers_info(AsyncWebServerRequest *request) {
    uint8_t *pins = NULL;
    uint8_t count;
    switcher.pins(&pins, &count);

    cJSON *response = cJSON_Duplicate(switcher.raw(), true);

    cJSON *switchers = cJSON_GetObjectItem(response, "switchers");
    for (uint8_t i = 0; i < count; i++) {
        uint8_t size = common::digits_count(i) + 1;
        char *n = (char *)malloc(size * sizeof(char));
        if (!n) {
            console::log("failed to allocate memory to convert number in char* in NetworkImpl");
            cJSON_Delete(response);
            return;
        }

        snprintf(n, size, "%d", i + 1);
        cJSON *s = cJSON_GetObjectItem(switchers, n);
        cJSON *item = cJSON_AddStringToObject(s, "state", digitalRead(pins[i]) == HIGH ? "OFF" : "ON");
        free(n);
    }

    request->send(HTTP_CODE_OK, "application/json", cJSON_PrintUnformatted(response));
    cJSON_Delete(response);
    free(pins);
}

void NetworkImpl::set_switcher_state(AsyncWebServerRequest *request) {
    uint8_t sn = request->pathArg(0).toInt();
    const char *state = request->pathArg(1).c_str();

    switcher_info_t info;
    switcher.info(sn, &info);

    digitalWrite(info.pin, strcmp(state, "on") == 0 ? LOW : HIGH);

    request->send(HTTP_CODE_OK);
}

void NetworkImpl::process() {
    dnsServer.processNextRequest();
}

NetworkImpl network;
}  // namespace net