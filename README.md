# Fire Detection System (ESP32 + PMS5003 + BME280)

ESP32 sensor nodes measure particulate matter and weather data, then transmit readings over ESP-NOW to a gateway ESP32 that hosts a web dashboard.


---

## Features

### Sensor Node (ESP32)
- PMS5003: PM1.0 / PM2.5 / PM10 (UART)
- BME280: temperature / humidity / pressure (I2C)
- Sends compact packets via ESP-NOW on a fixed Wi-Fi channel

### Gateway / Receiver (ESP32)
- Receives ESP-NOW packets (optionally MAC-filtered)
- Serves a browser dashboard using an HTTP server
- Shows latest readings and update status

---

## Hardware

### Per Node
- ESP32 38-Pin dev board
- PMS5003 particulate sensor
- BME280 sensor module
- Breakout board for ESP32
- Buck Converter 7.9V ---> 5.1V

## Wiring

### PMS5003 (UART)
Wiring
- PMS TX → ESP32 RX (UART2 RX)
- PMS RX → ESP32 TX (UART2 TX, optional)
- PMS VCC → 5V
- PMS GND → GND

### BME280 (I2C)
- SDA → ESP32 SDA ( GPIO21 )
- SCL → ESP32 SCL ( GPIO22 )
- VCC → 3.3V
- GND → GND

---

## Build / Flash

### 1) Flash the Gateway
- Upload the receiver/gateway firmware
- Open Serial Monitor
- Note the printed IP address
- Open the dashboard in a browser: `192.168.4.1`

### 2) Flash the Node(s)
- Set the gateway MAC (receiver peer) in the sender firmware
- Set the Wi-Fi channel to match the gateway
- Upload the sender firmware
- Confirm ESP-NOW send status is successful

---

## Configuration

You’ll find the following important values in the code:
- `WIFI_CHANNEL` (must match on sender and receiver)
- `RECEIVER_MAC` / allowed sender MAC list
- UART + I2C pin definitions
- Send rate limit (to prevent packet spam)

---

## Testing

Goal: detect smoke trends while minimizing false positives.

Likely testing:
- Baseline run in clean air (10–15 min)
- Controlled smoke source at multiple distances and heights
- Record PM2.5 response, time-to-detect, and environmental conditions


---

## Possible Issues

**ESP-NOW send fails**
- Channel mismatch
- Wrong peer MAC
- Receiver not configured on the same channel

**Dashboard loads but shows no data**
- Sender filtered out by MAC allowlist
- Packet struct mismatch between sender and receiver
- Send rate too high → increase delay

**BME280 not detected**
- Wrong I2C address (0x76 vs 0x77)
- Wiring/power issue

---

## Disclaimer

Educational prototype only. Do not use as a life-safety system or as the sole source of wildfire warning.

---
## License
This project is licensed under the MIT License — see the `LICENSE` file for details.
