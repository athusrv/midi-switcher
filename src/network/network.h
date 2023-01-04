#ifndef MIDI_SWITCHER_NETWORK
#define MIDI_SWITCHER_NETWORK

#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include "../common.h"
#include "../console.h"
#include "../settings.h"
#include "../switcher.h"

#define HOSTNAME "midilights"

namespace net {

using namespace std;

class NetworkImpl {
   private:
    const byte DNS_PORT = 53;
    static HTTPClient *httpClient;
    DNSServer dnsServer;
    IPAddress accessPointIP = IPAddress(8, 8, 4, 4);
    function<void()> onConnectEventCallback = NULL;
    function<void()> onDisconnectEventCallback = NULL;

    void onConnected(WiFiEvent_t, WiFiEventInfo_t);
    void get_switchers_info(AsyncWebServerRequest *request);
    void set_switcher_state(AsyncWebServerRequest *request);

    class CaptiveRequestHandler : public AsyncWebHandler {
       public:
        CaptiveRequestHandler() {}
        virtual ~CaptiveRequestHandler() {}

        bool canHandle(AsyncWebServerRequest *request) {
            return request->host() != String(HOSTNAME) + ".local";
        }

        void handleRequest(AsyncWebServerRequest *request) {
            String location = "http://" + String(HOSTNAME) + ".local";
            if (request->host() == String(HOSTNAME) + ".local")
              location += request->url();
            request->redirect(location);
        }
    };

   public:
    AsyncWebServer server = AsyncWebServer(80);

    NetworkImpl();
    virtual ~NetworkImpl();
    bool begin();
    void onDisconnect(function<void()>);
    bool isConnectedToWiFi();
    void connect(function<void()>);
    void process();
};

extern net::NetworkImpl network;
}  // namespace net

using namespace net;

#endif