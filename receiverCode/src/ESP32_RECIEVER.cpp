#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WebServer.h>

#include "ui_page.h"

// =======================================================
// CONFIG
// =======================================================
static constexpr uint8_t WIFI_CHANNEL = 6;

static const uint8_t ALLOWED_SENDER_MAC_1[6] = {0x04, 0x83, 0x08, 0x57, 0xF6, 0x54};
static const uint8_t ALLOWED_SENDER_MAC_2[6] = {0x04, 0x83, 0x08, 0x59, 0x3B, 0xEC};

static constexpr bool DEBUG_LOGS = true;

// Local dashboard AP
static const char* AP_SSID = "Receiver-Dashboard";
static const char* AP_PASS = "";

// =======================================================
// PACKET FORMATS
// =======================================================
typedef struct __attribute__((packed)) {
  uint32_t seq;
  uint32_t ms;
  uint16_t pm1;
  uint16_t pm25;
  uint16_t pm10;
  uint8_t  ok;
} pms_packet_t;

typedef struct __attribute__((packed)) {
  uint16_t pm1;
  uint16_t pm25;
  uint16_t pm10;
} pms_simple_t;

typedef struct __attribute__((packed)) {
  uint32_t seq;
  uint32_t ms;
  uint16_t pm1;
  uint16_t pm25;
  uint16_t pm10;
  int16_t  temp_c_x100;
  uint16_t hum_x100;
  uint8_t  ok;
} pms_bme_packet_t;

// =======================================================
// HELPERS
// =======================================================
static bool macEquals(const uint8_t* a, const uint8_t* b) {
  for (int i = 0; i < 6; i++) if (a[i] != b[i]) return false;
  return true;
}

// Returns 0 = not allowed, 1 = sender1, 2 = sender2
static uint8_t senderId(const uint8_t* mac) {
  if (macEquals(mac, ALLOWED_SENDER_MAC_1)) return 1;
  if (macEquals(mac, ALLOWED_SENDER_MAC_2)) return 2;
  return 0;
}

static void printMac(const uint8_t* mac) {
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// =======================================================
// LATEST DATA STORAGE
// =======================================================
struct NodeData {
  uint8_t  present;
  uint8_t  hasBme;

  uint8_t  last_ok;        // ok from most recent packet
  uint32_t seq;
  uint32_t sender_ms;
  uint32_t rx_millis;

  uint8_t  pm_valid;
  uint32_t pm_rx_millis;

  uint16_t pm1, pm25, pm10;
  int16_t  temp_c_x100;
  uint16_t hum_x100;
};
static NodeData g_nodes[3]; // index 1..2 used
static portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;

// =======================================================
// WEB SERVER
// =======================================================
WebServer server(80);

static void sendTelemetryJson() {
  NodeData n1, n2;
  portENTER_CRITICAL(&g_mux);
  n1 = g_nodes[1];
  n2 = g_nodes[2];
  portEXIT_CRITICAL(&g_mux);

  auto nodeToJson = [](const NodeData& n) -> String {
    float age_s    = n.present   ? (millis() - n.rx_millis)    / 1000.0f : 0.0f;
    float pm_age_s = n.pm_valid  ? (millis() - n.pm_rx_millis) / 1000.0f : 0.0f;

    float temp_c   = n.temp_c_x100 / 100.0f;
    float hum_pct  = n.hum_x100 / 100.0f;
  

    String s = "{";
    s += "\"present\":" + String(n.present ? 1 : 0) + ",";
    s += "\"hasBme\":"  + String(n.hasBme ? 1 : 0) + ",";
    s += "\"last_ok\":" + String(n.last_ok ? 1 : 0) + ",";
    s += "\"seq\":"     + String(n.seq) + ",";

    s += "\"pm_valid\":" + String(n.pm_valid ? 1 : 0) + ",";
    s += "\"pm_age_s\":" + String(pm_age_s, 1) + ",";
    s += "\"pm1\":"     + String(n.pm1) + ",";
    s += "\"pm25\":"    + String(n.pm25) + ",";
    s += "\"pm10\":"    + String(n.pm10) + ",";

    s += "\"temp_c\":"  + String(temp_c, 2) + ",";
    s += "\"hum_pct\":" + String(hum_pct, 2) + ",";
    s += "\"age_s\":"   + String(age_s, 1);
    s += "}";
    return s;
  };

  String out = "{";
  out += "\"uptime_ms\":" + String(millis()) + ",";
  out += "\"node1\":" + nodeToJson(n1) + ",";
  out += "\"node2\":" + nodeToJson(n2);
  out += "}";

  server.send(200, "application/json", out);
}

// =======================================================
// ESP-NOW RECEIVE CALLBACK
// =======================================================
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
void onEspNowRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  const uint8_t* srcMac = info->src_addr;
#else
void onEspNowRecv(const uint8_t* srcMac, const uint8_t* data, int len) {
#endif
  uint8_t nodeId = senderId(srcMac);
  if (nodeId == 0) {
    if (DEBUG_LOGS) {
      Serial.print("Blocked sender ");
      printMac(srcMac);
      Serial.printf(" len=%d\n", len);
    }
    return;
  }

  // ---- BME packet ----
  if (len == (int)sizeof(pms_bme_packet_t)) {
    pms_bme_packet_t pkt;
    memcpy(&pkt, data, sizeof(pkt));

    uint32_t now = millis();

    portENTER_CRITICAL(&g_mux);
    NodeData& n = g_nodes[nodeId];

    // Always mark alive
    n.present   = 1;
    n.hasBme    = 1;
    n.last_ok   = pkt.ok ? 1 : 0;
    n.seq       = pkt.seq;
    n.sender_ms = pkt.ms;
    n.rx_millis = now;

    // BME values are still useful even if PMS frame failed
    n.temp_c_x100   = pkt.temp_c_x100;
    n.hum_x100      = pkt.hum_x100;

    // Only update PM when ok==1 (prevents flicker)
    if (pkt.ok) {
      n.pm_valid     = 1;
      n.pm_rx_millis = now;
      n.pm1  = pkt.pm1;
      n.pm25 = pkt.pm25;
      n.pm10 = pkt.pm10;
    }

    portEXIT_CRITICAL(&g_mux);
    return;
  }

  // ---- PMS packet (with ok flag) ----
  if (len == (int)sizeof(pms_packet_t)) {
    pms_packet_t pkt;
    memcpy(&pkt, data, sizeof(pkt));

    uint32_t now = millis();

    portENTER_CRITICAL(&g_mux);
    NodeData& n = g_nodes[nodeId];

    n.present   = 1;
    n.hasBme    = 0;
    n.last_ok   = pkt.ok ? 1 : 0;
    n.seq       = pkt.seq;
    n.sender_ms = pkt.ms;
    n.rx_millis = now;

    if (pkt.ok) {
      n.pm_valid     = 1;
      n.pm_rx_millis = now;
      n.pm1  = pkt.pm1;
      n.pm25 = pkt.pm25;
      n.pm10 = pkt.pm10;
    }

    portEXIT_CRITICAL(&g_mux);
    return;
  }

  // ---- Simple PMS packet (no ok flag) ----
  if (len == (int)sizeof(pms_simple_t)) {
    pms_simple_t s;
    memcpy(&s, data, sizeof(s));

    uint32_t now = millis();

    portENTER_CRITICAL(&g_mux);
    NodeData& n = g_nodes[nodeId];

    n.present     = 1;
    n.hasBme      = 0;
    n.last_ok     = 1;
    n.rx_millis   = now;

    n.pm_valid     = 1;
    n.pm_rx_millis = now;
    n.pm1  = s.pm1;
    n.pm25 = s.pm25;
    n.pm10 = s.pm10;

    portEXIT_CRITICAL(&g_mux);
    return;
  }

  if (DEBUG_LOGS) {
    Serial.print("Unknown payload from ");
    printMac(srcMac);
    Serial.printf(" | node=%u bytes=%d\n", nodeId, len);
  }
}

// =======================================================
// SETUP / LOOP
// =======================================================
void setup() {
  Serial.begin(115200);
  delay(200);

  // AP+STA for dashboard + ESPNOW
  WiFi.mode(WIFI_AP_STA);
  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  WiFi.disconnect(true, true);
  WiFi.setSleep(false);
  delay(50);

  // Lock channel, then start AP on that channel
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  bool apOk = WiFi.softAP(AP_SSID, AP_PASS, WIFI_CHANNEL);
  if (!apOk) Serial.println("softAP failed");

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  Serial.print("Receiver STA MAC (use this in sender): ");
  Serial.println(WiFi.macAddress());

  Serial.print("Receiver AP  MAC: ");
  Serial.println(WiFi.softAPmacAddress());

  uint8_t ch; wifi_second_chan_t sch;
  esp_wifi_get_channel(&ch, &sch);
  Serial.printf("Receiver channel: %u\n", ch);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(onEspNowRecv);

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/telemetry", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    sendTelemetryJson();
  });

  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
  delay(2);
}