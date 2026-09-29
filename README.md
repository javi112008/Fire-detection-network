# Wildfire Detection Network
<img width="439" height="257" alt="image" src="https://github.com/user-attachments/assets/cc36fa88-21a2-400b-bdba-d75553aa25aa" />

<img width="248" height="212" alt="Screenshot 2026-09-28 224438" src="https://github.com/user-attachments/assets/ceba2a11-fbaa-43a9-9904-0c6c6f815390" />

<img width="436" height="243" alt="image" src="https://github.com/user-attachments/assets/d446a8a0-6f91-452c-9d86-c100c3854412" />

<img width="4284" height="5712" alt="IMG_1723" src="https://github.com/user-attachments/assets/5fe330e6-c8fd-4d0b-8a46-e0a3b2c84c7a" />


An ESP32-based wireless sensor network that measures airborne particles, temperature, and humidity, then displays live readings and possible fire conditions on a local web dashboard.

We built this as a three-person Engineering Design and Development capstone at Grand Terrace High School. The goal was to explore how multiple low-cost sensor nodes could help identify smoke-related conditions without relying on a cloud service or an internet connection.

**CURRENT Status as of 10-28-2026:** Working proof of concept with documented smoke-exposure and communication tests. Project still needs broader testing and calibration before any real-world use :)

## Credits

- **Javi Medorio** — Developed the ESP32 C++ firmware, designed the wiring and hardware layout, planned the wireless communication system, and created the detection and alert logic. Also helped with parts of the webpage.
- **Jaitine Kristeme Macias** — Developed the HTML dashboard and led the data analysis and project reports.
- **Jonathan Taylot** — Designed the CAD enclosure, assembled the prototype, developed the mathematical calculations, and helped with wiring.


## My Contribution

I developed the C++ firmware, designed the hardware and wiring, and worked out the wireless communication architecture. I also designed the detection algorithms and decision logic, including how the system combines sensor readings and checks for agreement between nodes. I occasionally helped with the webpage implementation.

**ALL RESPECTIVE CREDITS LISTED ABOVE**

## How It Works

Two ESP32 sensor nodes each read a PMS5003 particulate sensor and a BME280 temperature/humidity sensor. They send data directly to a receiver ESP32 using ESP-NOW. The receiver identifies each node, stores its latest readings, and serves a dashboard through its own Wi-Fi access point.

The browser polls the receiver's `/telemetry` endpoint, plots readings, and runs the alert logic. The current implementation uses a **star topology**: both nodes communicate directly with the receiver.
| Component | Role |
| --- | --- |
| Sensor nodes | Read sensors and transmit telemetry, normally about once per second |
| Receiver ESP32 | Receive packets, identify nodes by MAC address, and serve the webpage and JSON telemetry |
| Browser dashboard | Display readings, maintain rolling graphs, estimate missed updates, and evaluate alerts |

### Firmware and dashboard features

- PM1.0, PM2.5, and PM10 measurements from the PMS5003 over UART.
- Temperature and relative humidity from the BME280 over I2C.
- PMS frame-header, length, and checksum validation.
- Packet sequence numbers, sender timestamps, and a flag for fresh PMS readings.
- Last-valid particulate readings with separate indicators for packet age and PM-data age.
- Live graphs for both nodes and a rule-based alert system.
- Local operation through the receiver's `Receiver-Dashboard` Wi-Fi network.

The current telemetry packet includes temperature and humidity, but does not include barometric pressure.

## Detection Logic

The dashboard evaluates recent PM2.5, temperature, and humidity readings. A node meets the fire-condition rule when its average PM2.5 reaches the configured threshold, its average temperature reaches the configured minimum, and its average relative humidity falls below the configured maximum.

With `requireBoth` enabled, both nodes must meet that rule before the dashboard shows **Possible Wildfire**. A single matching node or elevated PM2.5 produces a lower-level smoke warning. PM1.0 and PM10 appear on the dashboard, but the current classifier does not use them in its decision rule.

Requiring agreement helps avoid escalation from an isolated reading, but creates a tradeoff: wind may carry smoke toward only one node, delaying or preventing a stronger alert.

### Current configuration

The `ALERT` object in [ui_page.h](receiverCode/include/ui_page.h) currently contains:

```javascript
const ALERT = {
  windowS: 30,
  minValid: 1,
  smokePM25: 1,
  firePM25: 1,
  dryRH: 80,
  warmC: 22,
  requireBoth: true
};
```

These settings need calibration against baseline and smoke-exposure data. Both PM2.5 thresholds currently equal **1 µg/m³**. Do not treat them as validated wildfire thresholds or assume they match the settings used for the portfolio's test results.

The averaging window holds up to 30 dashboard samples, approximately 30 seconds during normal polling. Because `minValid` equals `1`, the code can classify conditions before collecting a full window; it does not enforce 30 continuous seconds of agreement.

## Hardware and Wiring

The prototype uses three ESP32 development boards: two sensor nodes and one receiver. Each sensor node uses a PMS5003, a BME280, wiring/breakout connections, and a regulated power supply. The physical prototype also uses custom enclosures and a battery/buck-converter arrangement.

| Sensor connection | ESP32 connection |
| --- | --- |
| PMS5003 TX | GPIO 16 — UART2 RX |
| PMS5003 RX | GPIO 17 — UART2 TX; firmware reads the sensor's incoming frames |
| PMS5003 VCC | 5 V supply |
| PMS5003 GND | Common ground |
| BME280 SDA | GPIO 21 |
| BME280 SCL | GPIO 22 |
| BME280 VCC | 3.3 V |
| BME280 GND | Common ground |

The firmware uses UART at 9600 baud and checks both `0x76` and `0x77` for the BME280's I2C address. Verify the pin labels and power requirements of your exact modules before connecting them.

## Repository Files

| File | Purpose |
| --- | --- |
| [senderCode/src/ESP32_SENDER.cpp](senderCode/src/ESP32_SENDER.cpp) | Sensor readings, packet creation, and ESP-NOW transmission |
| [receiverCode/src/ESP32_RECIEVER.cpp](receiverCode/src/ESP32_RECIEVER.cpp) | Packet reception, node tracking, Wi-Fi access point, and HTTP server |
| [receiverCode/include/ui_page.h](receiverCode/include/ui_page.h) | Embedded HTML, CSS, JavaScript, graphs, and alert logic |
| [senderCode/platformio.ini](senderCode/platformio.ini) | Sender build configuration and sensor dependencies |
| [receiverCode/platformio.ini](receiverCode/platformio.ini) | Receiver build configuration |

## Setup

Both firmware projects use PlatformIO, the Arduino framework, and the `esp32dev` board configuration.

1. Clone this repository and open `receiverCode` and `senderCode` as separate PlatformIO projects.
2. Check each `platformio.ini`. Change or remove the hardcoded `monitor_port = COM3` to match your computer. Remove or adjust `lib_extra_dirs` if you do not use the referenced local Arduino library folder.
3. Build and upload the receiver firmware. Open its serial monitor at **115200 baud** and record the receiver's printed **STA MAC address** and AP IP address.
4. Set `RECEIVER_MAC` in the sender firmware to that STA MAC address. Keep `WIFI_CHANNEL` consistent on both projects; the current setting is **6**.
5. Upload the sender firmware to each sensor node and record each node's printed `SENDER STA MAC`.
6. Set `ALLOWED_SENDER_MAC_1` and `ALLOWED_SENDER_MAC_2` in the receiver firmware to your nodes' addresses, then upload the receiver again.
7. Connect a phone or computer to **Receiver-Dashboard** and open **http://192.168.4.1**, or the AP IP printed by your receiver. Confirm that both nodes update and report fresh sensor data.

The sender configuration declares the Adafruit BME280 and Adafruit Unified Sensor libraries. The sender attempts to join the receiver's access point to align its radio channel, while it sends telemetry through ESP-NOW.

## Prototype Testing

Our EDD portfolio documents baseline measurements, smoke exposure at several distances, uneven plume exposure, dust, heat-only conditions, recovery, and packet delivery.

| Test or measurement | Reported result |
| --- | --- |
| Baseline PM2.5 | Node A: 3 µg/m³; Node B: 4 µg/m³ |
| Smoke peak at 20 ft | Node A: 1,439 µg/m³; Node B: 699 µg/m³ |
| Uneven exposure at 60–65 ft | Node A: 26 µg/m³; Node B: 122 µg/m³ |
| Heat-only test | Node A reached 46.9°C with PM2.5 at 8 µg/m³; the report recorded no full wildfire alert |
| Packet delivery across recorded runs | 4,718 of 4,800 expected packets received: **98.3% delivery**, or **1.7% loss** |

These figures come from our project report, not a benchmark of every revision in this repository. **Packet delivery measures communication performance, not fire-detection accuracy.** Distance from the smoke source also does not establish wireless range or a guaranteed detection radius.

### Design changes from testing

Our first enclosure did not fit all the components correctly and had misaligned sensor openings. Later testing exposed heat buildup in the black enclosure. The team revised the CAD, added ventilation, changed the exterior to white, and updated the lid design. These changes addressed practical fit and exposure problems; they did not establish a verified waterproof rating.

## Limitations and Next Steps

- Collect longer baseline runs and repeated smoke, dust, fog, and exhaust trials before drawing conclusions about false-positive or false-negative rates.
- Tune and document the alert thresholds, and compare strict two-node agreement with weighted or partial agreement.
- Move alert evaluation out of the browser if the system needs to monitor continuously without an open dashboard. Graph history and alert evaluation currently depend on the browser session.
- Improve fault reporting. The current dashboard can show “Normal” when telemetry fetching fails, and the sender uses zero temperature/humidity values if it cannot initialize the BME280. Neither behavior should imply safe conditions.
- Validate power consumption, battery runtime, outdoor durability, and radio performance independently. Solar power, week-long battery life, and area coverage remain goals rather than demonstrated capabilities here.

The current access point is open, and the sender disables ESP-NOW encryption. This configuration serves the local prototype; a deployed version would need additional access controls and communication security.

## License

[MIT License](LICENSE) — Copyright © 2026 Javier Medorio Cancino.
