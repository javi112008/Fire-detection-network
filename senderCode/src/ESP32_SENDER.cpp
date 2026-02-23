#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <math.h>

// =================== CONFIG ===================
static constexpr uint8_t WIFI_CHANNEL = 6;

// Use whichever MAC works in YOUR setup.
// In many setups: receiver STA MAC. If AP MAC works for you, keep it.
static const uint8_t RECEIVER_MAC[6] = {0x04, 0x83, 0x08, 0x57, 0x8F, 0x3D};

// Join receiver AP to lock channel
static const char* RX_AP_SSID = "Receiver-Dashboard";
static const char* RX_AP_PASS = ""; // open

static constexpr int PMS_RX_PIN = 16;
static constexpr int PMS_TX_PIN = 17;

static constexpr int I2C_SDA = 21;
static constexpr int I2C_SCL = 22;

// PMS is ~1 frame/sec. Don’t spam faster than the sensor.
static constexpr uint32_t SEND_PERIOD_MS = 1000;

// PMS constants
static constexpr uint8_t  PMS_HDR1 = 0x42;
static constexpr uint8_t  PMS_HDR2 = 0x4D;
static constexpr uint16_t PMS_FRAME_LEN_VALUE = 28;
static constexpr int      PMS_FRAME_BYTES = 32;
static constexpr uint32_t PMS_READ_TIMEOUT_MS = 150;   // per-frame read timeout
static constexpr uint32_t PMS_WAIT_WINDOW_MS  = 1200;  // wait window to catch a frame

// =================== PACKET ===================
// Pressure removed, matches your receiver pms_bme_packet_t
typedef struct __attribute__((packed)) {
  uint32_t seq;
  uint32_t ms;
  uint16_t pm1;
  uint16_t pm25;
  uint16_t pm10;
  int16_t  temp_c_x100;
  uint16_t hum_x100;
  uint8_t  ok; // 1 = PMS frame captured this cycle
} pms_bme_packet_t;

static pms_bme_packet_t pkt{};
static Adafruit_BME280 bme;
static bool bmeOk = false;

static esp_now_peer_info_t peerInfo{};
static volatile uint32_t g_lastTxSeq = 0;

// =================== CALLBACK ===================
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  (void)info;
  Serial.printf("TX seq=%lu status=%s\n",
                (unsigned long)g_lastTxSeq,
                status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}
#else
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  (void)mac_addr;
  Serial.printf("TX seq=%lu status=%s\n",
                (unsigned long)g_lastTxSeq,
                status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}
#endif

// =================== PMS ===================
static inline uint16_t readBE16(const uint8_t* p) {
  return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

bool readPMS_ATM(HardwareSerial& s, uint16_t& pm1, uint16_t& pm25, uint16_t& pm10) {
  while (s.available() >= 2) {
    if (s.peek() != PMS_HDR1) { s.read(); continue; }
    s.read();
    if (s.read() != PMS_HDR2) continue;

    uint8_t frame[PMS_FRAME_BYTES];
    frame[0] = PMS_HDR1;
    frame[1] = PMS_HDR2;

    uint32_t start = millis();
    int got = 0;
    while (got < 30) {
      if (s.available()) frame[2 + got++] = (uint8_t)s.read();
      else {
        if (millis() - start > PMS_READ_TIMEOUT_MS) return false;
        yield();
      }
    }

    if (readBE16(&frame[2]) != PMS_FRAME_LEN_VALUE) continue;

    uint16_t sum = 0;
    for (int i = 0; i < 30; i++) sum += frame[i];
    uint16_t chk = readBE16(&frame[30]);
    if (sum != chk) continue;

    // ATM values
    pm1  = readBE16(&frame[10]);
    pm25 = readBE16(&frame[12]);
    pm10 = readBE16(&frame[14]);
    return true;
  }
  return false;
}

// Wait up to PMS_WAIT_WINDOW_MS to catch ONE valid frame.
bool waitForPmsFrame(uint16_t& pm1, uint16_t& pm25, uint16_t& pm10) {
  uint32_t t0 = millis();
  while (millis() - t0 < PMS_WAIT_WINDOW_MS) {
    if (readPMS_ATM(Serial2, pm1, pm25, pm10)) return true;
    delay(5);
  }
  return false;
}

// =================== BME ===================
static inline int16_t clampI16(long v) {
  if (v < -32768) return -32768;
  if (v >  32767) return  32767;
  return (int16_t)v;
}
static inline uint16_t clampU16(long v) {
  if (v < 0) return 0;
  if (v > 65535) return 65535;
  return (uint16_t)v;
}

void setupBME() {
  Wire.begin(I2C_SDA, I2C_SCL);
  bmeOk = bme.begin(0x76);
  if (!bmeOk) bmeOk = bme.begin(0x77);
  Serial.println(bmeOk ? "BME280 OK." : "BME280 not found. Sending 0s.");
}

// =================== ESP-NOW SETUP ===================
static void printChannel(const char* tag) {
  uint8_t ch; wifi_second_chan_t sch;
  esp_wifi_get_channel(&ch, &sch);
  Serial.printf("%s channel=%u\n", tag, ch);
}

static void forceChannel(uint8_t ch) {
  esp_err_t e = esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  Serial.printf("Force channel %u -> %s\n", ch, esp_err_to_name(e));
}

void setupEspNow() {
  WiFi.mode(WIFI_STA);

  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  WiFi.disconnect(true, true);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);
  delay(100);

  Serial.print("SENDER STA MAC: ");
  Serial.println(WiFi.macAddress());

  // Join receiver AP to lock channel
  if (strlen(RX_AP_PASS) == 0) WiFi.begin(RX_AP_SSID);
  else WiFi.begin(RX_AP_SSID, RX_AP_PASS);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 7000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi status: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "NOT CONNECTED");

  if (WiFi.status() != WL_CONNECTED) {
    // If we failed to connect, don’t trust the channel.
    forceChannel(WIFI_CHANNEL);
  }

  printChannel("SENDER");

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_del_peer(RECEIVER_MAC);

  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, RECEIVER_MAC, 6);
  peerInfo.ifidx   = WIFI_IF_STA;
  peerInfo.channel = 0;      // use current channel
  peerInfo.encrypt = false;

  esp_err_t perr = esp_now_add_peer(&peerInfo);
  if (perr != ESP_OK) {
    Serial.printf("add_peer failed: 0x%X (%s)\n", (unsigned)perr, esp_err_to_name(perr));
    while (true) delay(1000);
  }

  Serial.println("ESP-NOW ready.");
}

// =================== SETUP / LOOP ===================
void setup() {
  Serial.begin(115200);
  delay(200);

  // FIX: buffer size must be set BEFORE begin()
  Serial2.setRxBufferSize(256);
  Serial2.begin(9600, SERIAL_8N1, PMS_RX_PIN, PMS_TX_PIN);

  setupBME();
  setupEspNow();
}

void loop() {
  static uint32_t seq = 0;
  static uint32_t lastSendMs = 0;

  // Keep last good PM so we don't spam zeros when a read fails
  static bool pmEverValid = false;
  static uint16_t last_pm1 = 0, last_pm25 = 0, last_pm10 = 0;

  uint32_t now = millis();
  if (now - lastSendMs < SEND_PERIOD_MS) return;
  lastSendMs = now;

  // Try to get a fresh PMS frame this cycle
  uint16_t pm1=0, pm25=0, pm10=0;
  bool gotPms = waitForPmsFrame(pm1, pm25, pm10);
  if (gotPms) {
    pmEverValid = true;
    last_pm1 = pm1;
    last_pm25 = pm25;
    last_pm10 = pm10;
  }

  int16_t  t_x100 = 0;
  uint16_t h_x100 = 0;
  if (bmeOk) {
    float t = bme.readTemperature();
    float h = bme.readHumidity();
    t_x100 = clampI16(lroundf(t * 100.0f));
    h_x100 = clampU16(lroundf(h * 100.0f));
  }

  pkt.seq = ++seq;
  pkt.ms  = now;

  pkt.pm1  = pmEverValid ? last_pm1  : 0;
  pkt.pm25 = pmEverValid ? last_pm25 : 0;
  pkt.pm10 = pmEverValid ? last_pm10 : 0;

  pkt.temp_c_x100 = t_x100;
  pkt.hum_x100    = h_x100;

  // ok=1 only when PMS actually produced a NEW valid frame this cycle
  pkt.ok = gotPms ? 1 : 0;

  g_lastTxSeq = pkt.seq;

  esp_err_t err = esp_now_send(RECEIVER_MAC, (uint8_t*)&pkt, sizeof(pkt));
  Serial.printf("SEND seq=%lu ok=%u pm25=%u err=0x%X (%s)\n",(unsigned long)pkt.seq, (unsigned)pkt.ok, (unsigned)pkt.pm25,(unsigned)err, esp_err_to_name(err));
}