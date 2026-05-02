# Changelog

🇩🇪 Deutsch | [🇬🇧 English](CHANGELOG.en.md)

Alle wesentlichen Änderungen an diesem Projekt werden in dieser Datei dokumentiert.

Das Format basiert auf [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## Kompatibilitätsstatus

Getestet mit:
- ESPHome **2026.4.3** — ESP-IDF ✅

---

## [1.3.3] — 2026-04-29 — Cross-Task-Synchronisation

### Geändert
- `connected_` und `last_heartbeat_ms_` auf `std::atomic` umgestellt — verhindert Race Conditions zwischen BLE-GATT-Task und main loop

### Dokumentation
- Thread-Safety-Kommentar zum `link_quality_*`-Ringpuffer (bewusst nicht mutex-geschützt — torn updates verursachen nur kleinen Sample-Verlust, keinen Crash)


## [1.3.2] — 2026-04-23 — Link-Quality-Fixes

### Behoben
- `link_quality`: Timeout zählt jetzt gemäß erwarteter Paketrate (~1 Ausfall pro 1 s Stille statt 1 pro 15 s) — Metrik reagiert schneller auf Verbindungsabbruch
- `link_quality`: Strom- und Temperaturpakete zählen jetzt ebenfalls als Heartbeat (bisher nur Zellenspannungspakete) — verhindert falsch niedrige Werte wenn nur bestimmte Pakettypen ankommen
- `link_quality`: veröffentlicht `NAN` beim Trennen statt `0` — unterscheidet „kein Signal" von „schlechtes Signal"
- Temperaturbereich-Prüfung inklusiv (`>=` / `<=`) — Grenzwerte 2732 K und 3983 K werden jetzt korrekt akzeptiert
- Log-Prefix für Ective-Strompakete korrigiert (`"^:"` → `"0x5E:"`)

---

## [1.3.1] — 2026-04-21 — Link-Quality & Charging-Sensor-Fixes

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
