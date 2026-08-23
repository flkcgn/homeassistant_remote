# ne0nbanana Home Assistant Remote

Eine physische Fernbedienung auf Basis eines ESP32-S3 für eine lokale Home-Assistant-Instanz. Das Projekt kombiniert vier Tasten, ein ILI9225-SPI-Display und eine kontrastreiche gelb-schwarze Terminaloberfläche.

## Aktueller Stand

Der Breadboard-Prototyp initialisiert Display und Tasten, verbindet sich per WLAN und kann die Licht-Dienste konfigurierter Home-Assistant-Bereiche über die REST-API aufrufen. Das aktuelle, noch fest codierte Menü bietet:

```text
Hauptmenü
├── Licht
│   ├── Wohnzimmer
│   │   ├── An
│   │   └── Aus
│   └── Arbeitszimmer
└── Ausschalten
```

Der REST-Client und die Menüstruktur sind ein Zwischenstand. Für Version 1.0 werden die Firmware modularisiert, Einzellampen und Szenen ergänzt und Zustände über Home Assistants WebSocket-API synchronisiert.

## Hardware

- ESP32-S3-FH4R2 mit USB-C
- ILI9225-TFT, 176 × 220 Pixel, SPI, Landscape-Betrieb
- vier Tasten: Up, Down, Enter und Back
- geplanter Akkubetrieb mit 560-mAh-LiPo/Li-Ion-Zelle
- geplante Lade- und Versorgungsschaltung mit TP4056 und MT3608

Aktuelle Taster-Pins: Up GPIO2, Down GPIO3, Enter GPIO4 und Back GPIO5. Vor dem finalen Aufbau wird Down wegen der Strapping-Funktion von GPIO3 auf GPIO6 verlegt. Display: CS GPIO7, RST GPIO8, RS/DC GPIO9, MOSI GPIO10 und CLK GPIO11.

## Bedienung

- Up/Down ändern die Auswahl.
- Enter öffnet einen Menüpunkt oder führt eine explizite An-/Aus-Aktion aus.
- Back geht eine Ebene zurück und führt keine Schaltaktion aus.
- `Ausschalten` im Hauptmenü versetzt den ESP32 in den Tiefschlaf.
- Zum Aufwecken aus diesem Software-Aus Enter mindestens drei Sekunden halten. Ein kürzerer Druck führt zurück in den Tiefschlaf.

Software-Aus trennt die Versorgung nicht physisch. Ein mechanischer Hauptschalter und die Abschaltung der Displaybeleuchtung gehören noch zum finalen Energiekonzept.

## Lokale Konfiguration

```sh
cp include/secrets.example.h include/secrets.h
```

Anschließend in `include/secrets.h` WLAN, die exakt verifizierte Home-Assistant-Basis-URL und einen widerrufbaren Long-Lived Access Token eintragen. Die Datei ist von Git ausgeschlossen. Zugangsdaten dürfen weder in Commits noch in Logs, Screenshots oder Videos erscheinen.

Die bestätigten Home-Assistant-Bereichs-IDs sind `wohnzimmer` und `arbeitszimmer`. Einzelne Lampen- und Szenen-IDs werden erst für die entsprechenden Entwicklungsphasen ergänzt.

## Build und Flash

Voraussetzung ist eine lokale PlatformIO-Installation.

```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

Das Projekt verwendet die Umgebung `esp32-s3`. Ein erfolgreicher Build belegt keine Funktion auf der physischen Hardware; Display, Tasten, Netzwerk und Energiemodi müssen am realen Gerät geprüft werden.

## Scope von Version 1.0

Version 1.0 steuert ausschließlich alle Lichter eines konfigurierten Raums, einzelne Lampen und Lichtszenen in Wohnzimmer und Arbeitszimmer. Dimmen, RGB, Mediensteuerung, automatische Discovery, Cloudzugriff und OTA-Updates sind bewusst nicht Teil dieses Releases.

## Lizenz

Der Quellcode steht unter der in [LICENSE](LICENSE) beschriebenen MIT-Lizenz.
