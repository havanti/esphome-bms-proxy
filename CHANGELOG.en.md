# Changelog

[🇩🇪 Deutsch](CHANGELOG.md) | 🇬🇧 English

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## Compatibility Status

Tested against:
- ESPHome **2026.4.3** — ESP-IDF ✅

---

## [1.3.3] — 2026-04-29 — Cross-task synchronisation

### Changed
- `connected_` and `last_heartbeat_ms_` converted to `std::atomic` — prevents race conditions between the BLE GATT task and the main loop

### Documentation
- Thread-safety comment on the `link_quality_*` ring buffer (deliberately not mutex-protected — torn updates only cause minor sample loss, no crash)


## [1.3.2] — 2026-04-23 — Link-quality fixes

### Fixed
- `link_quality`: timeout now counts missed cycles at the expected packet rate (~1 failure per 1 s of silence instead of 1 per 15 s) — metric degrades faster on link loss
- `link_quality`: current and temperature packets now also reset the heartbeat (previously only cell-voltage packets did) — prevents incorrectly low readings when only certain packet types arrive
- `link_quality`: publishes `NAN` on disconnect instead of `0` — distinguishes "no signal" from "poor signal"
- Temperature range check is now inclusive (`>=` / `<=`) — boundary values 2732 K and 3983 K are now correctly accepted
- Corrected log prefix for Ective current packets (`"^:"` → `"0x5E:"`)

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
