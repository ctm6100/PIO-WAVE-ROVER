# waveshare_rover

PlatformIO port of Waveshare Rover v0.9 firmware for ESP32.

This repo keeps original Arduino-style module layout, then builds it with PlatformIO using `espressif32`, `esp32dev`, and Arduino framework.

## What this firmware does

- Drives Waveshare rover base on ESP32
- Initializes OLED, INA219 battery monitor, IMU, encoders, motor PID, WiFi, ESP-NOW, and HTTP control server
- Supports optional module modes for RoArm-M2 or gimbal control
- Stores runtime config and mission files in LittleFS
- Serves built-in web control page from ESP32 on port `80`

Main startup flow lives in `src/main.cpp` and initializes:

- LittleFS
- motor and servo control
- WiFi
- HTTP server
- ESP-NOW
- encoders and PID loops

## Project layout

```text
.
├── platformio.ini
├── src/
│   ├── main.cpp
│   ├── *_ctrl.h / *_module.h
│   └── data/
│       ├── devConfig.json
│       └── wifiConfig.json
├── include/
├── lib/
└── test/
```

Important files:

- `platformio.ini` - PlatformIO environment and library dependencies
- `src/main.cpp` - firmware entry point
- `src/ugv_config.h` - board, mode, servo, motor, and behavior defaults
- `src/wifi_ctrl.h` - WiFi config loading and AP/STA logic
- `src/http_server.h` - web server endpoints
- `src/web_page.h` - embedded control UI
- `src/files_ctrl.h` - LittleFS helpers

## Requirements

- PlatformIO Core or VS Code + PlatformIO extension
- ESP32 development board compatible with `board = esp32dev`
- USB serial connection for flash and monitor

## Build

```bash
pio run
```

Current PlatformIO environment:

- `platform = espressif32`
- `board = esp32dev`
- `framework = arduino`
- `monitor_speed = 115200`

## Flash firmware

```bash
pio run -t upload
```

## Serial monitor

```bash
pio device monitor -b 115200
```

## LittleFS config files

Firmware expects config files in LittleFS root, especially:

- `/wifiConfig.json`
- mission files created at runtime

Current repo stores sample JSON files in `src/data/`, but default PlatformIO filesystem upload uses top-level `data/` directory.

If you want to pre-load filesystem contents, copy sample files into a top-level `data/` folder first:

```text
data/
├── devConfig.json
└── wifiConfig.json
```

Then upload LittleFS image:

```bash
pio run -t uploadfs
```

If no filesystem config exists, firmware still mounts LittleFS and may create files at runtime.

## WiFi behavior

WiFi settings load from `/wifiConfig.json`.

Supported boot modes:

- `0` = WiFi off
- `1` = AP
- `2` = STA
- `3` = AP + STA

Default logic in code:

- boot mode defaults to AP when no config exists
- after first successful STA connection, firmware can switch default boot behavior to AP+STA
- HTTP control page is served from `/`

Before flashing to your device, replace sample SSIDs and passwords with your own values.

Example format:

```json
{
  "wifi_mode_on_boot": 3,
  "sta_ssid": "YOUR_WIFI_NAME",
  "sta_password": "YOUR_WIFI_PASSWORD",
  "ap_ssid": "Rover",
  "ap_password": "12345678"
}
```

## Web control

Firmware starts a synchronous `WebServer` on port `80`.

- `GET /` serves embedded HTML control page
- `GET /js?...` accepts JSON command payload through request argument `0`

After WiFi comes up:

- AP address is `192.168.4.1`
- STA address is shown on serial output and OLED

## Hardware and mode notes

Config in `src/ugv_config.h` currently defaults to:

- `mainType = 1` for WAVE ROVER
- `moduleType = 0` for base rover without RoArm-M2 or gimbal
- `espNowMode = 3` for follower mode

Optional module modes:

- `moduleType = 1` for RoArm-M2
- `moduleType = 2` for gimbal

## Libraries

Configured through PlatformIO:

- ArduinoJson
- SCServo
- Adafruit SSD1306
- INA219_WE
- ESP32Encoder
- SimpleKalmanFilter
- Adafruit ICM20X
- Arduino PID Library

## Known repo caveats

- Repo is ported from Waveshare Rover v0.9 source layout, so many modules remain header-driven instead of split into `.cpp` files
- Filesystem sample data currently lives under `src/data/`, not top-level `data/`
- Web server uses `WebServer`, not async server

## Development

Build in VS Code with PlatformIO task or from shell:

```bash
pio run
```

This repo already builds successfully for `esp32dev` in current state.