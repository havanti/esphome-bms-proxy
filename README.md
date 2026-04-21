# ESPHome — BMS Proxy 🔋

🇩🇪 Deutsch | [🇬🇧 English](README.en.md) | [Changelog](CHANGELOG.md)

> Beiträge willkommen — bitte zuerst ein [Issue öffnen](../../issues), bevor du einen PR erstellst.

---

## Was dieses Projekt macht

Integriert **LiFePO4-Batteriemanagement-Systeme (BMS)** von Ective und Wattstunde in Home Assistant — über einen ESP32 als ESPHome BLE-Proxy.

Das BMS-Protokoll wurde vollständig per Reverse-Engineering aus HCI-Snoop-Logs entwickelt.

**Unterstützte Geräte:**

| Gerät | Status |
|---|---|
| Ective 200 Ah | getestet ✅ |
| Wattstunde 200 Ah | getestet ✅ |
| Andere 4S-12V-BMS mit Service-UUID `0xFFE0` | möglicherweise kompatibel |

---

## Voraussetzungen

- **ESP32** mit BLE — empfohlen: M5Stack Atom oder ESP32-S3 DevKitC-1
- **Framework**: ESP-IDF (kein Arduino)
- **ESPHome** ≥ 2026.3.0
- **MAC-Adresse** des BMS (einmalig mit [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile) ermitteln)

---

## Installation

### Schritt 1: Komponente einbinden

```yaml
external_components:
  - source:
      type: git
      url: https://codeberg.org/havanti/esphome-bms-proxy
    components: [bms]
    refresh: 24h
```

### Schritt 2: Konfiguration erstellen

Die Datei `example.yaml` als Ausgangspunkt verwenden. MAC-Adressen eintragen und WLAN-Zugangsdaten in `secrets.yaml` hinterlegen.

### Schritt 3: Flashen

```bash
esphome run example.yaml
```

OTA-Update nach dem Erstflash:

```bash
esphome run example.yaml --device 192.168.x.x
```

---

## Beispielkonfiguration

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
  - mac_address: "XX:XX:XX:XX:XX:XX"  # MAC-Adresse des BMS
    id: bms_ble
    auto_connect: true

bms:
  - ble_client_id: bms_ble
    soc:
      name: "Ladezustand"
    voltage:
      name: "Spannung"
    current:
      name: "Strom"
    power:
      name: "Leistung"
    cell_voltage_1:
      name: "Zelle 1"
    cell_voltage_2:
      name: "Zelle 2"
    cell_voltage_3:
      name: "Zelle 3"
    cell_voltage_4:
      name: "Zelle 4"
    temperature:
      name: "Temperatur"
    delta_voltage:
      name: "Zell-Delta"
    min_cell_voltage:
      name: "Zelle Min"
    max_cell_voltage:
      name: "Zelle Max"
    capacity:
      name: "Kapazität"
    cycles:
      name: "Ladezyklen"
    battery_charging:
      name: "Laden"
    connected:
      name: "BLE Verbunden"
    cell_imbalance:
      name: "Zell-Ungleichgewicht"
    cell_imbalance_threshold: 50
    runtime:
      name: "Restlaufzeit"
    remaining_wh:
      name: "Restenergie"
    link_quality:
      name: "Verbindungsqualität"
```

Für mehrere BMS weitere Einträge unter `bms:` hinzufügen — jeder mit eigenem `ble_client_id`.

---

## Konfigurationsoptionen

Alle Optionen unter `bms:` sind optional.

| Option | Typ | Beschreibung |
|---|---|---|
| `soc` | Sensor | Ladezustand in % |
| `voltage` | Sensor | Gesamtspannung in V |
| `current` | Sensor | Strom in A (positiv = laden, negativ = entladen) |
| `power` | Sensor | Leistung in W |
| `cell_voltage_1`–`4` | Sensor | Einzelne Zellspannungen in V |
| `temperature` | Sensor | Temperatur in °C |
| `delta_voltage` | Sensor | Zelldifferenz in V |
| `min_cell_voltage` | Sensor | Niedrigste Zellspannung in V |
| `max_cell_voltage` | Sensor | Höchste Zellspannung in V |
| `capacity` | Sensor | Nennkapazität in Ah |
| `cycles` | Sensor | Anzahl Ladezyklen |
| `battery_charging` | Binary Sensor | `true` wenn der Akku lädt |
| `connected` | Binary Sensor | `true` wenn BLE-Verbindung besteht |
| `cell_imbalance` | Binary Sensor | `true` wenn Zelldifferenz den Schwellwert überschreitet |
| `cell_imbalance_threshold` | Zahl (mV) | Schwellwert für `cell_imbalance` (Standard: 50 mV) |
| `runtime` | Sensor | Geschätzte Restlaufzeit in Minuten (nur beim Entladen) |
| `remaining_wh` | Sensor | Verbleibende Energie in Wh |
| `link_quality` | Sensor | BLE-Verbindungsqualität in % (rollierendes 100-Zyklen-Fenster) |

### Zelldifferenz-Schwellwerte

| Delta | Bewertung |
|---|---|
| < 20 mV | Optimal |
| 20–30 mV | Normal — Balancer aktiv |
| 30–50 mV | Beobachten |
| 50–100 mV | Warnung (Standard-Schwellwert) |
| > 100 mV | Kritisch |

---

## Feedback & Tests

Rückmeldungen sind willkommen — insbesondere zu anderen BMS-Varianten. Bitte ein [Issue](../../issues) mit dem Testergebnis öffnen.

---

## Haftungsausschluss

Die Nutzung dieses Projekts erfolgt vollständig freiwillig und auf eigene Gefahr. Diese Software wird „wie besehen" ohne jegliche Gewährleistung bereitgestellt.
