# ESPHome — BMS Proxy 🔋

[🇩🇪 Deutsch](README.md) | 🇬🇧 English | [Changelog](CHANGELOG.en.md)

> Contributions welcome — please [open an issue](../../issues) before creating a PR.

---

## What this project does

Integrates **LiFePO4 battery management systems (BMS)** from Ective and Wattstunde into Home Assistant — via an ESP32 running as an ESPHome BLE proxy.

The BMS protocol was fully developed through reverse engineering of HCI snoop logs.

**Supported devices:**

| Device | Status |
|---|---|
| Ective 200 Ah | tested ✅ |
| Wattstunde 200 Ah | tested ✅ |
| Other 4S 12 V BMS with service UUID `0xFFE0` | possibly compatible |

---

## Requirements

- **ESP32** with BLE — recommended: M5Stack Atom or ESP32-S3 DevKitC-1
- **Framework**: ESP-IDF (no Arduino)
- **ESPHome** ≥ 2026.3.0
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
    active: true
    continuous: true

esp32_ble:
  max_connections: 6

bluetooth_proxy:
  active: true

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
    capacity:
      name: "Capacity"
    cycles:
      name: "Charge Cycles"
    battery_charging:
      name: "Charging"
    connected:
      name: "BLE Connected"
    cell_imbalance:
      name: "Cell Imbalance"
    cell_imbalance_threshold: 50
    runtime:
      name: "Runtime"
    remaining_wh:
      name: "Remaining Energy"
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
| `delta_voltage` | Sensor | Cell voltage delta in V |
| `min_cell_voltage` | Sensor | Lowest cell voltage in V |
| `max_cell_voltage` | Sensor | Highest cell voltage in V |
| `capacity` | Sensor | Nominal capacity in Ah |
| `cycles` | Sensor | Number of charge cycles |
| `battery_charging` | Binary Sensor | `true` when the battery is charging |
| `connected` | Binary Sensor | `true` when BLE connection is active |
| `cell_imbalance` | Binary Sensor | `true` when cell delta exceeds the threshold |
| `cell_imbalance_threshold` | Number (mV) | Threshold for `cell_imbalance` (default: 50 mV) |
| `runtime` | Sensor | Estimated runtime in minutes (discharge only) |
| `remaining_wh` | Sensor | Remaining energy in Wh |
| `link_quality` | Sensor | BLE link quality in % (rolling 100-cycle window) |

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

Feedback is welcome — especially regarding other BMS variants. Please open an [issue](../../issues) with your test results.

---

## Disclaimer

Use of this project is entirely voluntary and at your own risk. This software is provided "as is" without any warranty.
