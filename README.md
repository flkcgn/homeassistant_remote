# homeassistant_remote

Ein ESP32-basiertes Remote zur Steuerung von Home Assistant.

## Ziel

Dieses Repository dient als Ausgangspunkt für eine kleine Fernbedienung mit ESP32, die per WLAN mit Home Assistant kommuniziert und z. B. Lichter, Szenen oder Medien steuert.

## Geplanter Funktionsumfang

- Verbindung zum WLAN
- Anbindung an Home Assistant (z. B. REST API oder MQTT)
- Tasten-/Encoder-Eingaben am ESP32
- Auslösen von Aktionen in Home Assistant
- Statusanzeige über kleines Display

## Voraussetzungen

- ESP32-S3 mit USB-C
- 2,2" TFT-Display mit ILI9225, 176 × 220 px, SPI
- 4× Taster für Up / Down / Enter / Back
- 1× Ein-/Aus-Schalter
- 1× 3,7-V-LiPo/Li-Ion-Akku, 560 mAh
- 1× TP4056-Lademodul mit Akkuschutz (DW01A + 8205A)
- 1× MT3608 Step-Up-Wandler
- 1× Schottky-Diode, z. B. 1N5819
- 1× 220-µF-Elko, 16 V
- 1× JST-PH-2.0-Stecker, 2-polig
- 560mAh lipo Akku
- 1× Lochrasterplatine, ca. 50 × 100 mm
- Litze / Schaltdraht
- Lötzinn

Für den Prototyp zusätzlich:
- Breadboard
- Jumper-/Dupont-Kabel

## Lokale Konfiguration

Kopiere vor dem ersten Build die lokale Konfigurationsvorlage:

```sh
cp include/secrets.example.h include/secrets.h
```
