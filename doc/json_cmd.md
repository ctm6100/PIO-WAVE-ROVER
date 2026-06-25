# JSON Command Reference

Detailed reference for all JSON commands defined in `src/json_cmd.h`.

## Overview

Command shape:

```json
{"T":130}
```

- `T` is required on every command.
- Other fields depend on command type.
- Commands can be sent over serial, HTTP, or ESP-NOW wrappers.

## Transport

### Serial

- One complete JSON object per line.
- Line must end with newline.
- Parsed in `serialCtrl()`.

Example:

```text
{"T":130}
```

### HTTP

- `GET /` serves control UI.
- `GET /js` executes command from first query argument.
- Response body is `jsonInfoHttp` after handler runs.

Example:

```bash
curl "http://192.168.4.1/js?json={\"T\":130}"
```

### ESP-NOW

- Some commands send wrapped control payloads to follower devices.
- Status and receive notifications use feedback IDs `1003` and `1004`.

## Feedback Frames

| ID | Name | Description | Example |
| --- | --- | --- | --- |
| `1001` | `FEEDBACK_BASE_INFO` | Base telemetry: speed, IMU attitude, battery, optional module data | `{"T":1001,"L":0.12,"R":0.11,"r":1.3,"p":-0.4,"y":92.1,"v":12.4}` |
| `1002` | `FEEDBACK_IMU_DATA` | IMU telemetry: attitude, accel, gyro, mag, temperature | `{"T":1002,"r":1.3,"p":-0.4,"y":92.1,"ax":0.01,"ay":-0.02,"az":9.81,"gx":0.1,"gy":0.0,"gz":-0.1,"mx":12.0,"my":-4.0,"mz":30.0,"temp":28.5}` |
| `1003` | `CMD_ESP_NOW_RECV` | ESP-NOW receive notification | `{"T":1003,"mac":"FF:FF:FF:FF:FF:FF","megs":"hello!"}` |
| `1004` | `CMD_ESP_NOW_SEND` | ESP-NOW send status | `{"T":1004,"mac":"FF:FF:FF:FF:FF:FF","status":1,"megs":"xxx"}` |
| `1005` | `CMD_BUS_SERVO_ERROR` | Bus servo error feedback | `{"T":1005,"id":1,"status":1}` |

ESP-NOW send status codes:

- `0`: failed
- `1`: succeed
- `2`: error initializing ESP-NOW
- `3`: invalid MAC address format
- `4`: failed to add peer
- `5`: add peer
- `6`: delete peer
- `7`: error sending data
- `8`: sent with success

## UGV Control

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `1` | `CMD_SPEED_CTRL` | Set left/right speed target | `L`, `R` | `{"T":1,"L":0.5,"R":0.5}` | Handler checks `L` and `R` as float. |
| `2` | `CMD_SET_MOTOR_PID` | Set motor PID and output limit | `P`, `I`, `D`, `L` | `{"T":2,"P":20,"I":2500,"D":0,"L":255}` | Often saved into mission steps. |
| `3` | `CMD_OLED_CTRL` | Set one OLED line | `lineNum`, `Text` | `{"T":3,"lineNum":0,"Text":"putYourTextHere"}` | |
| `-3` | `CMD_OLED_DEFAULT` | Restore default OLED content | none | `{"T":-3}` | |
| `4` | `CMD_MODULE_TYPE` | Change active module | `cmd` | `{"T":4,"cmd":0}` | `0` none, `1` RoArm-M2-S, `2` gimbal. |
| `11` | `CMD_PWM_INPUT` | Direct motor PWM control | `L`, `R` | `{"T":11,"L":164,"R":164}` | Bypasses PID. PWM about `-255..255`. |
| `13` | `CMD_ROS_CTRL` | Differential drive command from linear/angular velocities | `X`, `Z` | `{"T":13,"X":0.1,"Z":0.3}` | Comment says for products with encoders. |
| `126` | `CMD_GET_IMU_DATA` | Emit full IMU telemetry | none | `{"T":126}` | Response `T=1002`. |
| `127` | `CMD_CALI_IMU_STEP` | Calibrate IMU offset | none | `{"T":127}` | Keep robot still for about 5 seconds. |
| `128` | `CMD_GET_IMU_OFFSET` | Print IMU offset info | none | `{"T":128}` | Output path mostly serial-oriented. |
| `129` | `CMD_SET_IMU_OFFSET` | Set IMU offsets | `x`, `y`, `z` | `{"T":129,"x":-12,"y":0,"z":0}` | |
| `130` | `CMD_BASE_FEEDBACK` | Emit one base telemetry frame | none | `{"T":130}` | Response `T=1001`. |
| `131` | `CMD_BASE_FEEDBACK_FLOW` | Toggle continuous base feedback | `cmd` | `{"T":131,"cmd":1}` | `0` off, `1` on. |
| `132` | `CMD_LED_CTRL` | LED PWM control | `IO4`, `IO5` | `{"T":132,"IO4":255,"IO5":255}` | |
| `133` | `CMD_GIMBAL_CTRL_SIMPLE` | Simple gimbal move | `X`, `Y`, `SPD`, `ACC` | `{"T":133,"X":45,"Y":45,"SPD":0,"ACC":0}` | |
| `134` | `CMD_GIMBAL_CTRL_MOVE` | Gimbal move with separate axis speeds | `X`, `Y`, `SX`, `SY` | `{"T":134,"X":45,"Y":45,"SX":300,"SY":300}` | |
| `135` | `CMD_GIMBAL_CTRL_STOP` | Stop gimbal | none | `{"T":135}` | |
| `136` | `CMD_HEART_BEAT_SET` | Set heartbeat timeout | `cmd` | `{"T":136,"cmd":3000}` | Value in ms. |
| `137` | `CMD_GIMBAL_STEADY` | Toggle gimbal steady mode | `s`, `y` | `{"T":137,"s":1,"y":0}` | `s=0` off, `s=1` on. |
| `138` | `CMD_SET_SPD_RATE` | Set speed scaling factors | `L`, `R` | `{"T":138,"L":1,"R":1}` | |
| `139` | `CMD_GET_SPD_RATE` | Get speed scaling factors | none | `{"T":139}` | Returns JSON with `T=138`. |
| `140` | `CMD_SAVE_SPD_RATE` | Persist speed scaling factors | none | `{"T":140}` | |
| `141` | `CMD_GIMBAL_USER_CTRL` | Incremental gimbal user control | `X`, `Y`, `SPD` | `{"T":141,"X":0,"Y":0,"SPD":300}` | `-1` decrease, `1` increase, `0` stop, `2` center. |
| `142` | `CMD_FEEDBACK_FLOW_INTERVAL` | Set extra delay between feedback frames | `cmd` | `{"T":142,"cmd":0}` | Value in ms. |
| `143` | `CMD_UART_ECHO_MODE` | Toggle UART command echo | `cmd` | `{"T":143,"cmd":1}` | `0` off, `1` on. |

## Arm, EoAT, and Motion

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `100` | `CMD_MOVE_INIT` | Move arm to init pose | none | `{"T":100}` | |
| `101` | `CMD_SINGLE_JOINT_CTRL` | Move one joint in radians | `joint`, `rad`, `spd`, `acc` | `{"T":101,"joint":1,"rad":0,"spd":0,"acc":10}` | Joints: `1` base, `2` shoulder, `3` elbow, `4` EOAT. |
| `102` | `CMD_JOINTS_RAD_CTRL` | Move all joints in radians | `base`, `shoulder`, `elbow`, `hand`, `spd`, `acc` | `{"T":102,"base":0,"shoulder":0,"elbow":1.57,"hand":1.57,"spd":0,"acc":10}` | |
| `103` | `CMD_SINGLE_AXIS_CTRL` | Move one Cartesian axis | `axis`, `pos`, `spd` | `{"T":103,"axis":2,"pos":0,"spd":0.25}` | Axis: `1=x`, `2=y`, `3=z`, `4=t`. |
| `104` | `CMD_XYZT_GOAL_CTRL` | Move arm in Cartesian space with interpolation | `x`, `y`, `z`, `t`, `spd` | `{"T":104,"x":235,"y":0,"z":234,"t":3.14,"spd":0.25}` | |
| `1041` | `CMD_XYZT_DIRECT_CTRL` | Set direct XYZT target and move | `x`, `y`, `z`, `t` | `{"T":1041,"x":235,"y":0,"z":234,"t":3.14}` | No `spd` field. |
| `105` | `CMD_SERVO_RAD_FEEDBACK` | Emit arm pose and torque feedback | none | `{"T":105}` | Response uses `T=1051`. |
| `106` | `CMD_EOAT_HAND_CTRL` | Control EOAT/hand joint in radians | `cmd`, `spd`, `acc` | `{"T":106,"cmd":3.14,"spd":0,"acc":0}` | Common values: release `1.57`, grab `3.14`. |
| `107` | `CMD_EOAT_GRAB_TORQUE` | Set EOAT grip torque | `tor` | `{"T":107,"tor":200}` | |
| `108` | `CMD_SET_JOINT_PID` | Set joint PI gains | `joint`, `p`, `i` | `{"T":108,"joint":3,"p":16,"i":0}` | Default note in source: servo `p=32,i=0`; RoArm-M2 `p=16,i=8` in PID mode. |
| `109` | `CMD_RESET_PID` | Reset arm PID state | none | `{"T":109}` | |
| `110` | `CMD_SET_NEW_X` | Set new x-axis angle | `xAxisAngle` | `{"T":110,"xAxisAngle":0}` | |
| `111` | `CMD_DELAY_MILLIS` | Delay execution | `cmd` | `{"T":111,"cmd":3000}` | Value in ms. Used in mission files. |
| `112` | `CMD_DYNAMIC_ADAPTATION` | Set dynamic external force adaptation | `mode`, `b`, `s`, `e`, `h` | `{"T":112,"mode":1,"b":60,"s":110,"e":50,"h":50}` | `mode=0` resets limits to `1000`; `mode=1` applies given limits. |
| `113` | `CMD_SWITCH_CTRL` | Control 12V switch outputs | `pwm_a`, `pwm_b` | `{"T":113,"pwm_a":-255,"pwm_b":-255}` | Marked "NOT FOR UGV". Dispatcher currently comments this command out. |
| `114` | `CMD_LIGHT_CTRL` | Control light output | `led` | `{"T":114,"led":255}` | Marked "NOT FOR UGV". Dispatcher currently comments this command out. |
| `115` | `CMD_SWITCH_OFF` | Emergency switch off | none | `{"T":115}` | Calls `switchEmergencyStop()`. |
| `121` | `CMD_SINGLE_JOINT_ANGLE` | Move one joint in degrees | `joint`, `angle`, `spd`, `acc` | `{"T":121,"joint":1,"angle":0,"spd":10,"acc":10}` | `acc` max noted as `22.5`. |
| `122` | `CMD_JOINTS_ANGLE_CTRL` | Move all joints in degrees | `b`, `s`, `e`, `h`, `spd`, `acc` | `{"T":122,"b":0,"s":0,"e":90,"h":180,"spd":10,"acc":10}` | |
| `123` | `CMD_CONSTANT_CTRL` | Continuous angle or XYZT jog | `m`, `axis`, `cmd`, `spd` | `{"T":123,"m":0,"axis":0,"cmd":0,"spd":3}` | `m=0` angle, `m=1` XYZT; `cmd=0` stop, `1` increase, `2` decrease. |
| `124` | `CMD_EOAT_TYPE` | Set EOAT mode | `mode` | `{"T":124,"mode":0}` | `0` gripper, `1` wrist. |
| `125` | `CMD_CONFIG_EOAT` | Configure EOAT geometry | `pos`, `ea`, `eb` | `{"T":125,"pos":3,"ea":0,"eb":20}` | `pos`: `0` edge, `1` D-3.2, `2` D-4.2, `3` D-10.2. Units mm. |
| `144` | `CMD_ARM_CTRL_UI` | UI-oriented arm control | `E`, `Z`, `R` | `{"T":144,"E":100,"Z":0,"R":0}` | Used by built-in web UI. |

## File Commands

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `200` | `CMD_SCAN_FILES` | Scan LittleFS files | none | `{"T":200}` | Reads file list and first line. |
| `201` | `CMD_CREATE_FILE` | Create a new file | `name`, `content` | `{"T":201,"name":"file.txt","content":"inputContentHere."}` | Creates in LittleFS root. |
| `202` | `CMD_READ_FILE` | Read file contents | `name` | `{"T":202,"name":"file.txt"}` | Primarily serial output. |
| `203` | `CMD_DELETE_FILE` | Delete file | `name` | `{"T":203,"name":"file.txt"}` | |
| `204` | `CMD_APPEND_LINE` | Append line to file | `name`, `content` | `{"T":204,"name":"file.txt","content":"inputContentHere."}` | |
| `205` | `CMD_INSERT_LINE` | Insert line into file | `name`, `lineNum`, `content` | `{"T":205,"name":"file.txt","lineNum":3,"content":"content"}` | `lineNum` is 1-based in API comments. |
| `206` | `CMD_REPLACE_LINE` | Replace one line in file | `name`, `lineNum`, `content` | `{"T":206,"name":"file.txt","lineNum":3,"content":"Content"}` | |
| `207` | `CMD_READ_LINE` | Read one line from file | `name`, `lineNum` | `{"T":207,"name":"file.txt","lineNum":3}` | |
| `208` | `CMD_DELETE_LINE` | Delete one line from file | `name`, `lineNum` | `{"T":208,"name":"file.txt","lineNum":3}` | |

## Torque, Mission, and Step Editing

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `210` | `CMD_TORQUE_CTRL` | Toggle servo torque lock | `cmd` | `{"T":210,"cmd":1}` | `0` off, `1` on. Applies via `servoTorqueCtrl(254, ...)`. |
| `220` | `CMD_CREATE_MISSION` | Create mission file | `name`, `intro` | `{"T":220,"name":"mission_a","intro":"test mission created in flash."}` | Saved as `name.mission`. |
| `221` | `CMD_MISSION_CONTENT` | Get mission content | `name` | `{"T":221,"name":"mission_a"}` | |
| `222` | `CMD_APPEND_STEP_JSON` | Append step from raw JSON string | `name`, `step` | `{"T":222,"name":"mission_a","step":"{\"T\":104,\"x\":235,\"y\":0,\"z\":234,\"t\":3.14,\"spd\":0.25}"}` | `step` must itself be valid JSON string. |
| `223` | `CMD_APPEND_STEP_FB` | Append step from current arm feedback | `name`, `spd` | `{"T":223,"name":"mission_a","spd":0.25}` | Stores generated `T=104` step. |
| `224` | `CMD_APPEND_DELAY` | Append delay step | `name`, `delay` | `{"T":224,"name":"mission_a","delay":3000}` | Stored internally as `{"T":111,"cmd":3000}`. |
| `225` | `CMD_INSERT_STEP_JSON` | Insert step JSON at step index | `name`, `stepNum`, `step` | `{"T":225,"name":"mission_a","stepNum":3,"step":"{\"T\":114,\"led\":255}"}` | |
| `226` | `CMD_INSERT_STEP_FB` | Insert step from current arm feedback | `name`, `stepNum`, `spd` | `{"T":226,"name":"mission_a","stepNum":3,"spd":0.25}` | |
| `227` | `CMD_INSERT_DELAY` | Insert delay step | `name`, `stepNum`, `delay` | `{"T":227,"name":"mission_a","stepNum":3,"delay":3000}` | Source comment omits `name`. Current dispatcher reads `spd` instead of `delay`; this looks like bug. |
| `228` | `CMD_REPLACE_STEP_JSON` | Replace step with raw JSON string | `name`, `stepNum`, `step` | `{"T":228,"name":"mission_a","stepNum":3,"step":"{\"T\":114,\"led\":255}"}` | |
| `229` | `CMD_REPLACE_STEP_FB` | Replace step from current arm feedback | `name`, `stepNum`, `spd` | `{"T":229,"name":"mission_a","stepNum":3,"spd":0.25}` | |
| `230` | `CMD_REPLACE_DELAY` | Replace step with delay | `name`, `stepNum`, `delay` | `{"T":230,"name":"mission_a","stepNum":3,"delay":3000}` | Stored as `T=111` delay step. |
| `231` | `CMD_DELETE_STEP` | Delete mission step | `name`, `stepNum` | `{"T":231,"name":"mission_a","stepNum":3}` | |
| `241` | `CMD_MOVE_TO_STEP` | Execute one mission step | `name`, `stepNum` | `{"T":241,"name":"mission_a","stepNum":3}` | |
| `242` | `CMD_MISSION_PLAY` | Play mission repeatedly | `name`, `times` | `{"T":242,"name":"mission_a","times":3}` | `times=-1` loops forever. |

## ESP-NOW Commands

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `300` | `CMD_BROADCAST_FOLLOWER` | Configure broadcast follower behavior | `mode`, `mac` | `{"T":300,"mode":0,"mac":"CC:DB:A7:5B:E4:1C"}` | `mode=1` enables broadcast control. `mode=0` disables it and whitelists one MAC. |
| `301` | `CMD_ESP_NOW_CONFIG` | Set ESP-NOW mode | `mode` | `{"T":301,"mode":3}` | `0` none, `1` group leader, `2` single leader, `3` follower. |
| `302` | `CMD_GET_MAC_ADDRESS` | Get this device MAC address | none | `{"T":302}` | |
| `303` | `CMD_ESP_NOW_ADD_FOLLOWER` | Add peer MAC | `mac` | `{"T":303,"mac":"CC:DB:A7:5B:E4:1C"}` | Broadcast MAC also allowed. |
| `304` | `CMD_ESP_NOW_REMOVE_FOLLOWER` | Remove peer MAC | `mac` | `{"T":304,"mac":"CC:DB:A7:5B:E4:1C"}` | |
| `305` | `CMD_ESP_NOW_GROUP_CTRL` | Send group command to peers | `dev`, `b`, `s`, `e`, `h`, `cmd`, `megs` | `{"T":305,"dev":0,"b":0,"s":0,"e":1.57,"h":1.57,"cmd":0,"megs":"hello!"}` | Broadcast MAC cannot be in broadcast peer list. |
| `306` | `CMD_ESP_NOW_SINGLE` | Send single-device or broadcast command | `mac`, `dev`, `b`, `s`, `e`, `h`, `cmd`, `megs` | `{"T":306,"mac":"FF:FF:FF:FF:FF:FF","dev":0,"b":0,"s":0,"e":0,"h":0,"cmd":1,"megs":"{\"T\":114,\"led\":255}"}` | `cmd=0` real-time servo sync path; wrapped JSON often sent in `megs`. |

## WiFi Commands

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `401` | `CMD_WIFI_ON_BOOT` | Set WiFi mode on boot | `cmd` | `{"T":401,"cmd":3}` | `0` off, `1` AP, `2` STA, `3` AP+STA. |
| `402` | `CMD_SET_AP` | Start AP mode | `ssid`, `password` | `{"T":402,"ssid":"UGV","password":"12345678"}` | |
| `403` | `CMD_SET_STA` | Start STA mode | `ssid`, `password` | `{"T":403,"ssid":"na","password":"ps"}` | |
| `404` | `CMD_WIFI_APSTA` | Start AP+STA mode | `ap_ssid`, `ap_password`, `sta_ssid`, `sta_password` | `{"T":404,"ap_ssid":"UGV","ap_password":"12345678","sta_ssid":"na","sta_password":"ps"}` | |
| `405` | `CMD_WIFI_INFO` | Get WiFi status | none | `{"T":405}` | Response includes `ip`, `rssi`, `wifi_mode_on_boot`, `sta_ssid`, `sta_password`, `ap_ssid`, `ap_password`, `mac`. |
| `406` | `CMD_WIFI_CONFIG_CREATE_BY_STATUS` | Write `/wifiConfig.json` from current in-memory settings | none | `{"T":406}` | |
| `407` | `CMD_WIFI_CONFIG_CREATE_BY_INPUT` | Write `/wifiConfig.json` from input values | `mode`, `ap_ssid`, `ap_password`, `sta_ssid`, `sta_password` | `{"T":407,"mode":3,"ap_ssid":"UGV","ap_password":"12345678","sta_ssid":"na","sta_password":"ps"}` | |
| `408` | `CMD_WIFI_STOP` | Disconnect WiFi | none | `{"T":408}` | |

## Servo Settings

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `501` | `CMD_SET_SERVO_ID` | Change servo ID | `raw`, `new` | `{"T":501,"raw":1,"new":11}` | |
| `502` | `CMD_SET_MIDDLE` | Set current servo position as middle | `id` | `{"T":502,"id":11}` | Common IDs: base `11`, shoulder drive `12`, shoulder driven `13`, elbow `14`, gripper `15`. |
| `503` | `CMD_SET_SERVO_PID` | Set servo P gain | `id`, `p` | `{"T":503,"id":14,"p":16}` | Source comment calls it `P/PID`. |

## ESP32 System Commands

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `600` | `CMD_REBOOT` | Reboot device | none | `{"T":600}` | Calls `esp_restart()`. |
| `601` | `CMD_FREE_FLASH_SPACE` | Get free LittleFS space | none | `{"T":601}` | Response includes `info`, `total`, `free`. |
| `602` | `CMD_BOOT_MISSION_INFO` | Read boot mission content | none | `{"T":602}` | Equivalent to reading mission `boot`. |
| `603` | `CMD_RESET_BOOT_MISSION` | Reset boot mission | none | `{"T":603}` | Deletes `boot.mission`, then recreates boot file content. |
| `604` | `CMD_NVS_CLEAR` | Clear NVS and reinit | none | `{"T":604}` | Useful when WiFi state gets stuck. |
| `605` | `CMD_INFO_PRINT` | Set debug print level | `cmd` | `{"T":605,"cmd":1}` | `2` flow feedback, `1` debug serial, `0` silent. |

## Main and Module Type

| ID | Macro | Purpose | Fields | Example | Notes |
| --- | --- | --- | --- | --- | --- |
| `900` | `CMD_MM_TYPE_SET` | Set main type and module type | `main`, `module` | `{"T":900,"main":1,"module":0}` | Main types: `1` WAVE ROVER, `2` UGV02, `3` UGV01. |

## Response Notes

- Many query commands print to serial and also populate `jsonInfoHttp` for HTTP responses.
- `CMD_BASE_FEEDBACK` response includes extra fields when module is active:
  - RoArm mode: `x`, `y`, `z`, `b`, `s`, `e`, `t`, `torB`, `torS`, `torE`, `torH`
  - Gimbal mode: `pan`, `tilt`
- `CMD_SERVO_RAD_FEEDBACK` returns `T=1051`, not `T=105`.
- `CMD_GET_SPD_RATE` returns payload tagged with `T=138`.

## Known Quirks

- `CMD_SWITCH_CTRL` (`113`) and `CMD_LIGHT_CTRL` (`114`) are defined but commented out in `src/uart_ctrl.h` for current UGV build.
- `CMD_INSERT_DELAY` (`227`) source comment uses `delay`, but current dispatcher passes `jsonCmdReceive["spd"]` into `insertDelayCmd()`. This looks like implementation bug.
- `CMD_GET_IMU_OFFSET` (`128`) has weaker structured HTTP feedback than other query commands.
- HTTP handler uses first query arg only via `server.arg(0)`.
