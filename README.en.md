<div align="center">

# ESPHome BMS Proxy

[![Release](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fcodeberg.org%2Fapi%2Fv1%2Frepos%2Fhavanti%2Fesphome-bms-proxy%2Ftags%3Flimit%3D1&query=%24%5B0%5D.name&label=release&style=flat-square&color=blue)](https://codeberg.org/havanti/esphome-bms-proxy/releases) [![Stars](https://img.shields.io/github/stars/havanti/esphome-bms-proxy?style=flat-square&color=yellow)](https://github.com/havanti/esphome-bms-proxy) [![ESPHome](https://img.shields.io/badge/ESPHome-%E2%89%A52026.4.2-blue?style=flat-square)](https://esphome.io) [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?style=flat-square)](LICENSE)

[Features](#what-this-project-does) • [Installation](#installation) • [Configuration](#configuration-options) • [Feedback](#feedback--testing) • [Changelog](CHANGELOG.en.md)

[🇩🇪 Deutsch](README.md) | 🇬🇧 English

*Contributions welcome — please [open an issue](../../issues) before creating a PR.*

</div>

---

## What this project does

Integrates **LiFePO4 battery management systems (BMS)** from Ective and Wattstunde into Home Assistant — via an ESP32 running as an ESPHome BLE proxy.

The BMS protocol was fully developed through reverse engineering of HCI snoop logs.

**Supported devices:**

| Device | Status |
|---|---|
| Ective LC 200L BT+LT | tested ✅ |
| Wattstunde LIX 200 LT | tested ✅ |
| Other 4S 12 V BMS with service UUID `0xFFE0` | possibly compatible |

**Technical specifications of tested units:**

| Property | Ective LC 200L | Wattstunde LIX 200 LT |
|---|---|---|
| Nominal voltage | 12.8 V | 12.8 V |
| Nominal capacity | 200 Ah / 2560 Wh | 200 Ah / 2560 Wh |
| Measured capacity¹ | 201.4 Ah | 202.7 Ah |
| Discharge current cont. / peak | 150 A / 200 A (30 s) | 200 A (30 min) / ≤350 A (3 s) |
| Charge current rec. / max. | ≤80 A / 100 A | 100 A / 150 A |
| Charge voltage | 14.6 V | 14.6 V |
| Cycle life | ≥3000 (@90 % DoD) | ≥3000 (@90 % DoD) |
| Self-discharge | <3 %/month | ≤1.5 %/month |
| Heating element | present | present |
| Terminals | M8 | M8 |
| Dimensions L×W×H (mm) | 485×170×240 | 484×170×241 |
| Weight | 25.2 kg | 25.0 kg |
| Protection rating | IP65 | IP65 |
| Certifications | CE / UN38.3 / RoHS | — |

¹ Measured value for `nominal_capacity` in the SoH calculation.

---

## Requirements

- **ESP32** with BLE — recommended: M5Stack Atom or ESP32-S3 DevKitC-1
- **Framework**: ESP-IDF (no Arduino)
- **ESPHome** ≥ 2026.4.2
- **MAC address** of the BMS (determine once using [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile))

---

## Installation

### Step 1: Add the component

```yaml
external_components:
  - source:
      type: git
      url: https://codeberg.org/havanti/esphome-bms-proxy
    components: [bms]
    refresh: 24h
```

### Step 2: Create configuration

Use `example.yaml` as a starting point. Enter the MAC addresses and store Wi-Fi credentials in `secrets.yaml`.

### Step 3: Flash

```bash
esphome run example.yaml
```

OTA update after first flash:

```bash
esphome run example.yaml --device 192.168.x.x
```

---

## Example configuration

```yaml
external_components:
  - source:
      type: git
      url: https://codeberg.org/havanti/esphome-bms-proxy
    components: [bms]
    refresh: 24h

esp32_ble_tracker:
  scan_parameters:
    interval: 1100ms
    window: 1100ms
    active: false
    continuous: true

bluetooth_proxy:
  active: false

ble_client:
  - mac_address: "XX:XX:XX:XX:XX:XX"  # MAC address of the BMS
    id: bms_ble
    auto_connect: true

bms:
  - ble_client_id: bms_ble
    soc:
      name: "State of Charge"
    voltage:
      name: "Voltage"
    current:
      name: "Current"
    power:
      name: "Power"
    cell_voltage_1:
      name: "Cell 1"
    cell_voltage_2:
      name: "Cell 2"
    cell_voltage_3:
      name: "Cell 3"
    cell_voltage_4:
      name: "Cell 4"
    temperature:
      name: "Temperature"
    delta_voltage:
      name: "Cell Delta"
    min_cell_voltage:
      name: "Cell Min"
    max_cell_voltage:
      name: "Cell Max"
    avg_cell_voltage:
      name: "Cell Avg"
    capacity:
      name: "Capacity"
    cycles:
      name: "Charge Cycles"
    nominal_capacity: 200.0
    soh:
      name: "State of Health"
    battery_charging:
      name: "Charging"
    connected:
      name: "BLE Connected"
    cell_imbalance:
      name: "Cell Imbalance"
    cell_imbalance_threshold: 50
    runtime:
      name: "Runtime"
    remaining_ah:
      name: "Remaining Charge"
    remaining_wh:
      name: "Remaining Energy"
    charge_power:
      name: "Charge Power"
    discharge_power:
      name: "Discharge Power"
    link_quality:
      name: "Link Quality"
```

For multiple BMS units, add further entries under `bms:` — each with its own `ble_client_id`.

---

## Configuration options

All options under `bms:` are optional.

| Option | Type | Description |
|---|---|---|
| `soc` | Sensor | State of charge in % |
| `voltage` | Sensor | Total voltage in V |
| `current` | Sensor | Current in A (positive = charging, negative = discharging) |
| `power` | Sensor | Power in W |
| `cell_voltage_1`–`4` | Sensor | Individual cell voltages in V |
| `temperature` | Sensor | Temperature in °C |
| `avg_cell_voltage` | Sensor | Average cell voltage in V |
| `delta_voltage` | Sensor | Cell voltage delta in V |
| `min_cell_voltage` | Sensor | Lowest cell voltage in V |
| `max_cell_voltage` | Sensor | Highest cell voltage in V |
| `capacity` | Sensor | Measured capacity in Ah |
| `cycles` | Sensor | Number of charge cycles |
| `remaining_ah` | Sensor | Remaining charge in Ah |
| `remaining_wh` | Sensor | Remaining energy in Wh |
| `charge_power` | Sensor | Charge power in W (0 when discharging) |
| `discharge_power` | Sensor | Discharge power in W (0 when charging) |
| `runtime` | Sensor | Estimated runtime in minutes (discharge only) |
| `nominal_capacity` | Number (Ah) | Nameplate capacity for State of Health calculation (default: 0 = disabled) |
| `soh` | Sensor | State of Health in % (requires `nominal_capacity`) |
| `link_quality` | Sensor | BLE link quality in % (rolling 100-cycle window) |
| `battery_charging` | Binary Sensor | `true` when the battery is charging |
| `connected` | Binary Sensor | `true` when BLE connection is active |
| `cell_imbalance` | Binary Sensor | `true` when cell delta exceeds the threshold |
| `cell_imbalance_threshold` | Number (mV) | Threshold for `cell_imbalance` (default: 50 mV) |

### Cell imbalance thresholds

| Delta | Assessment |
|---|---|
| < 20 mV | Optimal |
| 20–30 mV | Normal — balancer active |
| 30–50 mV | Worth monitoring |
| 50–100 mV | Warning (default threshold) |
| > 100 mV | Critical |

---

## Feedback & Testing

If you give this component a try, your feedback is very welcome!

Please test it with your setup and let us know how it goes — whether everything works smoothly or you run into any issues. Feel free to open an [issue](../../issues) with your findings, bug reports, or suggestions for improvement. Every report helps make this project better for everyone.

---

## Trademark Notice

Ective and Wattstunde are registered trademarks of their respective owners. This project is an independent, community-driven open-source initiative and is neither affiliated with, endorsed by, nor supported by the trademark holders. The use of these brand names in this repository is solely for the purpose of technical identification and compatibility description.

## Disclaimer

Use of this project is entirely voluntary and at your own risk.

This software is provided "as is", without warranty of any kind, express or implied. The author(s) accept no liability whatsoever for any damage to persons, property, vehicles, battery systems, or any other assets arising from the use, misuse, or inability to use this software or the configurations provided herein. This includes, but is not limited to, damage resulting from incorrect configuration, unexpected device behaviour, software bugs, or hardware failure.

Before using this project with a battery system, ensure you understand the operation of your specific system and comply with all applicable safety regulations. Always test new configurations under supervised conditions.
