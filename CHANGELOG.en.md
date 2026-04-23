# Changelog

[🇩🇪 Deutsch](CHANGELOG.md) | 🇬🇧 English

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## Compatibility Status

Tested against:
- ESPHome **2026.4.2** — ESP-IDF ✅

---

## [1.3.1] — 2026-04-19 — Link-quality & charging-sensor fixes

### Fixed
- `link_quality`: heartbeat clock is now seeded on GATT connect so the 15 s
  timeout arms even when the BMS never delivers cell-voltage packets
  (previously the metric stayed silent on half-working links)
- `battery_charging`: added ±200 mA hysteresis and based the decision on the
  signed current value — eliminates sensor flapping around the zero-current
  point

---

## [1.2.0] — 2026-04-12 — Cleanup & Documentation

### Changed
- Removed `notify_handle_` member, use constant `NOTIFY_HANDLE` directly
- `NOTIFY_HANDLE` changed from `static const` to `static constexpr`
- Various stale and incorrect comments corrected

### Documentation
- README (DE + EN): Complete rewrite — consistent structure

---

## [1.1.1] — 2026-04-05 — Fixes

### Fixed
- Pinned ESP-IDF to recommended version (`recommended`) — prevents build issues with newer ESP-IDF releases
- Corrected repository URL in `example.yaml` to Codeberg

---

## [1.0.0] — 2026-03-29 — Initial Release

### Added
- Component `bms` for Topband BMS over BLE (tested with Ective and Wattstunde 200 Ah LiFePO4)
- Sensors: `soc`, `voltage`, `current`, `power`, `cell_voltage_1..4`, `temperature`, `capacity`, `cycles`, `delta_voltage`, `avg/min/max_cell_voltage`, `runtime`, `remaining_wh`, `remaining_ah`, `charge_power`, `discharge_power`, `soh`, `link_quality`
- Binary sensors: `connected`, `battery_charging`, `cell_imbalance`
- BLE GATT: service `0xFFE0`, notify characteristic `0xFFE4` (handle `0x0016`)
- Fully developed through reverse engineering of HCI snoop logs — packet parser by length/content
