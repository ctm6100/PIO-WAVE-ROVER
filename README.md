# waveshare_rover

PlatformIO port of [Waveshare Rover v0.9](https://www.waveshare.com/wiki/WAVE_ROVER) firmware for ESP32.

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

## JSON command API

Most runtime control goes through JSON commands defined in `src/json_cmd.h` and dispatched in `src/uart_ctrl.h`.

Full command reference lives in [doc/json_cmd.md](doc/json_cmd.md).

Command format:

- every command must include `T`
- `T` is command ID
- other keys depend on command type

Supported input paths:

- serial: one JSON object per line, newline terminated
- HTTP: send JSON as first query arg to `/js`
- ESP-NOW: some commands can be wrapped and forwarded to follower devices

Typical HTTP test:

```bash
curl "http://192.168.4.1/js?json={\"T\":130}"
```

`server.arg(0)` is used internally, so only first query arg matters.

### Command groups

Common command ranges:

- `1` to `143`: rover drive, OLED, IMU, feedback, LEDs, gimbal, speed tuning
- `100` to `144`: RoArm-M2 motion, joints, XYZT, torque, UI
- `200` to `208`: LittleFS file operations
- `220` to `242`: mission create, edit, play
- `300` to `306`: ESP-NOW config and forwarding
- `401` to `408`: WiFi config and status

### Common commands

Drive and base:

- `{"T":1,"L":0.5,"R":0.5}` set left/right speed target
- `{"T":11,"L":164,"R":164}` direct PWM input, range about `-255..255`
- `{"T":13,"X":0.1,"Z":0.3}` ROS-style linear/angular control
- `{"T":130}` get one base feedback packet
- `{"T":131,"cmd":1}` enable continuous base feedback

IMU and feedback:

- `{"T":126}` get IMU packet
- `{"T":127}` calibrate IMU while robot stays still
- `{"T":129,"x":-12,"y":0,"z":0}` set IMU offsets
- `{"T":143,"cmd":1}` enable UART echo for received commands

RoArm-M2 and motion:

- `{"T":100}` move arm to init pose
- `{"T":101,"joint":1,"rad":0,"spd":0,"acc":10}` move one joint in radians
- `{"T":104,"x":235,"y":0,"z":234,"t":3.14,"spd":0.25}` move arm in Cartesian space
- `{"T":105}` get arm position and torque feedback
- `{"T":121,"joint":1,"angle":0,"spd":10,"acc":10}` move one joint in degrees

Gimbal and LEDs:

- `{"T":132,"IO4":255,"IO5":255}` LED PWM control
- `{"T":133,"X":45,"Y":45,"SPD":0,"ACC":0}` simple gimbal move
- `{"T":137,"s":1,"y":0}` enable gimbal steady mode

Files and missions:

- `{"T":200}` scan LittleFS files
- `{"T":201,"name":"test.txt","content":"hello"}` create file
- `{"T":220,"name":"mission_a","intro":"demo mission"}` create mission
- `{"T":222,"name":"mission_a","step":"{\"T\":104,\"x\":235,\"y\":0,\"z\":234,\"t\":3.14,\"spd\":0.25}"}` append mission step
- `{"T":242,"name":"mission_a","times":3}` play mission three times

ESP-NOW and WiFi:

- `{"T":301,"mode":3}` set ESP-NOW mode
- `{"T":302}` get this device MAC address
- `{"T":306,"mac":"FF:FF:FF:FF:FF:FF","dev":0,"b":0,"s":0,"e":0,"h":0,"cmd":1,"megs":"{\"T\":132,\"IO4\":255,\"IO5\":0}"}` broadcast wrapped command
- `{"T":401,"cmd":3}` set WiFi boot mode to AP+STA
- `{"T":405}` get WiFi status

### Feedback examples

Base feedback response from `{"T":130}` uses `T = 1001`:

```json
{
  "T": 1001,
  "L": 0.12,
  "R": 0.11,
  "r": 1.3,
  "p": -0.4,
  "y": 92.1,
  "v": 12.4
}
```

If `moduleType = 1`, base feedback also includes arm pose and torque fields such as `x`, `y`, `z`, `b`, `s`, `e`, `t`, `torB`, `torS`, `torE`, `torH`.

IMU response from `{"T":126}` uses `T = 1002`:

```json
{
  "T": 1002,
  "r": 1.3,
  "p": -0.4,
  "y": 92.1,
  "ax": 0.01,
  "ay": -0.02,
  "az": 9.81,
  "gx": 0.1,
  "gy": 0.0,
  "gz": -0.1,
  "mx": 12.0,
  "my": -4.0,
  "mz": 30.0,
  "temp": 28.5
}
```

RoArm feedback from `{"T":105}` uses `T = 1051`.

### Notes

- serial handler expects full JSON ending with newline
- some commands print feedback to serial only
- HTTP path returns `jsonInfoHttp` as response body after command runs
- command comments in `src/json_cmd.h` are currently best source of exact field names and units

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
