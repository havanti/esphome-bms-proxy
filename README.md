<div align="center">

# ESPHome BMS Proxy

[![Release](https://img.shields.io/github/v/release/havanti/esphome-bms-proxy?style=flat-square&color=blue)](https://github.com/havanti/esphome-bms-proxy/releases) [![Stars](https://img.shields.io/github/stars/havanti/esphome-bms-proxy?style=flat-square&color=yellow)](https://github.com/havanti/esphome-bms-proxy) [![ESPHome](https://img.shields.io/badge/ESPHome-%E2%89%A52026.4.4-blue?style=flat-square)](https://esphome.io) [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?style=flat-square)](LICENSE)

[Funktionen](#was-dieses-projekt-macht) • [Installation](#installation) • [Konfiguration](#konfigurationsoptionen) • [Feedback](#feedback--tests) • [Changelog](CHANGELOG.md)

🇩🇪 Deutsch | [🇬🇧 English](README.en.md)

*Beiträge willkommen — bitte zuerst ein [Issue öffnen](../../issues), bevor du einen PR erstellst.*

</div>

---

## Was dieses Projekt macht

Integriert **LiFePO4-Batteriemanagement-Systeme (BMS)** von Ective und Wattstunde in Home Assistant — über einen ESP32 als ESPHome BLE-Proxy.

Das BMS-Protokoll wurde vollständig per Reverse-Engineering aus HCI-Snoop-Logs entwickelt.

**Unterstützte Geräte:**

| Gerät | Status |
|---|---|
| Ective LC 200L BT+LT | getestet ✅ |
| Wattstunde LIX 200 LT | getestet ✅ |
| Andere 4S-12V-BMS mit Service-UUID `0xFFE0` | möglicherweise kompatibel |

**Technische Daten der getesteten Akkus:**

| Eigenschaft | Ective LC 200L | Wattstunde LIX 200 LT |
|---|---|---|
| Nennspannung | 12,8 V | 12,8 V |
| Nennkapazität | 200 Ah / 2560 Wh | 200 Ah / 2560 Wh |
| Gemessene Kapazität¹ | 201,4 Ah | 202,7 Ah |
| Entladestrom Dauer / Spitze | 150 A / 200 A (30 s) | 200 A (30 min) / ≤350 A (3 s) |
| Ladestrom empf. / max. | ≤80 A / 100 A | 100 A / 150 A |
| Ladespannung | 14,6 V | 14,6 V |
| Ladezyklen | ≥3000 (@90 % DoD) | ≥3000 (@90 % DoD) |
| Selbstentladung | <3 %/Monat | ≤1,5 %/Monat |
| Heizelement | vorhanden | vorhanden |
| Pol-Anschlüsse | M8 | M8 |
| Maße L×B×H (mm) | 485×170×240 | 484×170×241 |
| Gewicht | 25,2 kg | 25,0 kg |
| Schutzart | IP65 | IP65 |
| Zertifizierungen | CE / UN38.3 / RoHS | — |

¹ Gemessener Wert für `nominal_capacity` in der SoH-Berechnung.

---

## Voraussetzungen

- **ESP32** mit BLE — empfohlen: M5Stack Atom oder ESP32-S3 DevKitC-1
- **Framework**: ESP-IDF (kein Arduino)
- **ESPHome** ≥ 2026.4.4
- **MAC-Adresse** des BMS (einmalig mit nRF Connect ermitteln)

---

## Installation

### Schritt 1: Komponente einbinden

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/havanti/esphome-bms-proxy
    components: [bms]
    refresh: always
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
      url: https://github.com/havanti/esphome-bms-proxy
    components: [bms]
    refresh: always

esp32_ble_tracker:
  scan_parameters:
    interval: 1100ms
    window: 1100ms
    active: false
    continuous: true

bluetooth_proxy:
  active: false

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
    avg_cell_voltage:
      name: "Zelle Ø"
    capacity:
      name: "Kapazität"
    cycles:
      name: "Ladezyklen"
    nominal_capacity: 200.0
    soh:
      name: "State of Health"
    battery_charging:
      name: "Laden"
    connected:
      name: "BLE Verbunden"
    cell_imbalance:
      name: "Zell-Ungleichgewicht"
    cell_imbalance_threshold: 50
    runtime:
      name: "Restlaufzeit"
    remaining_ah:
      name: "Restladung"
    remaining_wh:
      name: "Restenergie"
    charge_power:
      name: "Ladeleistung"
    discharge_power:
      name: "Entladeleistung"
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
| `avg_cell_voltage` | Sensor | Durchschnittliche Zellspannung in V |
| `delta_voltage` | Sensor | Zelldifferenz in V |
| `min_cell_voltage` | Sensor | Niedrigste Zellspannung in V |
| `max_cell_voltage` | Sensor | Höchste Zellspannung in V |
| `capacity` | Sensor | Gemessene Kapazität in Ah |
| `cycles` | Sensor | Anzahl Ladezyklen |
| `remaining_ah` | Sensor | Verbleibende Ladung in Ah |
| `remaining_wh` | Sensor | Verbleibende Energie in Wh |
| `charge_power` | Sensor | Ladeleistung in W (0 beim Entladen) |
| `discharge_power` | Sensor | Entladeleistung in W (0 beim Laden) |
| `runtime` | Sensor | Geschätzte Restlaufzeit in Minuten (nur beim Entladen) |
| `nominal_capacity` | Zahl (Ah) | Nennkapazität für State-of-Health-Berechnung (Standard: 0 = deaktiviert) |
| `soh` | Sensor | State of Health in % (erfordert `nominal_capacity`) |
| `link_quality` | Sensor | BLE-Verbindungsqualität in % (rollierendes 100-Zyklen-Fenster) |
| `battery_charging` | Binary Sensor | `true` wenn der Akku lädt |
| `connected` | Binary Sensor | `true` wenn BLE-Verbindung besteht |
| `cell_imbalance` | Binary Sensor | `true` wenn Zelldifferenz den Schwellwert überschreitet |
| `cell_imbalance_threshold` | Zahl (mV) | Schwellwert für `cell_imbalance` (Standard: 50 mV) |

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

Wer diese Komponente ausprobiert, dessen Feedback ist sehr willkommen!

Bitte mit dem eigenen Setup testen und mitteilen, wie es läuft — ob alles reibungslos funktioniert oder Probleme auftreten. Einfach ein [Issue](../../issues) mit den Ergebnissen, Fehlerberichten oder Verbesserungsvorschlägen öffnen. Jeder Bericht hilft, dieses Projekt für alle besser zu machen.

---

## Markenhinweis

Ective und Wattstunde sind eingetragene Marken ihrer jeweiligen Eigentümer. Dieses Projekt ist eine unabhängige, von der Community getriebene Open-Source-Initiative und steht in keiner Verbindung zu den Markeninhabern, wird von ihnen weder empfohlen noch unterstützt. Die Verwendung dieser Markennamen in diesem Repository dient ausschließlich der technischen Identifikation und Kompatibilitätsbeschreibung.

## Haftungsausschluss

Die Nutzung dieses Projekts erfolgt vollständig freiwillig und auf eigene Gefahr.

Diese Software wird „wie besehen" ohne jegliche ausdrückliche oder stillschweigende Gewährleistung bereitgestellt. Die Autor(en) übernehmen keinerlei Haftung für Schäden an Personen, Eigentum, Fahrzeugen, Batteriesystemen oder sonstigen Vermögenswerten, die durch die Nutzung, missbräuchliche Verwendung oder Nichtnutzbarkeit dieser Software oder der hierin enthaltenen Konfigurationen entstehen. Dies schließt unter anderem Schäden ein, die auf fehlerhafte Konfiguration, unerwartetes Geräteverhalten, Software-Fehler oder Hardware-Ausfälle zurückzuführen sind.

Vor der Nutzung dieses Projekts mit einem Batteriesystem ist sicherzustellen, dass das betreffende System vollständig verstanden wird und alle geltenden Sicherheitsvorschriften eingehalten werden. Neue Konfigurationen immer unter Aufsicht testen.
