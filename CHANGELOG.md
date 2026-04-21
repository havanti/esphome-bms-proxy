# Changelog

🇩🇪 Deutsch | [🇬🇧 English](CHANGELOG.en.md)

Alle wesentlichen Änderungen an diesem Projekt werden in dieser Datei dokumentiert.

Das Format basiert auf [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [1.3.1] — 2026-04-19 — Link-Quality & Charging-Sensor-Fixes

### Behoben
- `link_quality`: Heartbeat-Uhr wird beim GATT-Connect gesetzt, damit der
  15-s-Timeout auch dann greift, wenn das BMS keine Zellenspannungs-Pakete
  liefert (vorher blieb die Metrik stumm)
- `battery_charging`: ±200 mA Hysterese hinzugefügt und Entscheidung auf den
  vorzeichenbehafteten Stromwert umgestellt — verhindert das Flackern des
  Binary-Sensors am Nulldurchgang

---

## [1.2.0] — 2026-04-12 — Aufräumen & Dokumentation

### Geändert
- `set_mode_and_setpoint(bool, float)` → `set_mode(bool)` — ungenutzter `temp_celsius`-Parameter entfernt (Protokoll verbietet Setpoint vor CMD_ON)
- Setpoint-Rundung durch `lroundf()` ersetzt
- `CMD_*`-Arrays aus dem Header in die Implementierungsdatei verschoben
- `notify_handle_`-Member entfernt, Konstante `NOTIFY_HANDLE` direkt verwendet
- `NOTIFY_HANDLE` von `static const` auf `static constexpr` geändert
- Diverse veraltete und falsche Kommentare korrigiert

### Dokumentation
- README (DE + EN): Vollständig überarbeitet — einheitliche Struktur

---

## [1.1.1] — 2026-04-05 — Fixes

### Behoben
- ESP-IDF auf empfohlene Version gepinnt (`recommended`) — verhindert Build-Probleme mit neueren ESP-IDF-Releases
- Repo-URL in `example.yaml` auf Codeberg korrigiert

---

## [1.0.0] — 2026-03-29 — Erstveröffentlichung

### Hinzugefügt
- Komponente `bms` für Topband BMS via BLE (getestet mit Ective und Wattstunde 200 Ah LiFePO4)
- Sensoren: `soc`, `voltage`, `current`, `power`, `cell_voltage_1..4`, `temperature`, `capacity`, `cycles`, `delta_voltage`, `avg/min/max_cell_voltage`, `runtime`, `remaining_wh`, `remaining_ah`, `charge_power`, `discharge_power`, `soh`, `link_quality`
- Binary Sensors: `connected`, `battery_charging`, `cell_imbalance`
- BLE GATT: Service `0xFFE0`, Notify-Characteristic `0xFFE4` (Handle `0x0016`)
- Vollständig per Reverse-Engineering aus HCI-Snoop-Logs entwickelt — Packet-Parser per Länge/Inhalt
