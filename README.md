# ne0nbanana Home Assistant Remote

A physical remote control based on an ESP32-S3 for a local Home Assistant instance. The project combines four buttons, an ILI9225 SPI display, and a high-contrast yellow-on-black terminal-style interface.

## Current Status

The breadboard prototype initializes the display and buttons, connects via Wi-Fi, and can call the light services of configured Home Assistant areas through the REST API. The current, still hard-coded menu provides:

~~~text
Main Menu
├── Lights
│   ├── Living Room
│   │   ├── On
│   │   └── Off
│   └── Office
└── Power Off
~~~

The REST client and menu structure are an intermediate development stage. For version 1.0, the firmware will be modularized, individual lights and scenes will be added, and states will be synchronized through Home Assistant's WebSocket API.

## Hardware

- ESP32-S3-FH4R2 with USB-C
- ILI9225 TFT, 176 × 220 pixels, SPI, landscape mode
- four buttons: Up, Down, Enter, and Back
- planned battery operation using a 560 mAh LiPo/Li-Ion cell
- planned charging and power circuitry using a TP4056 and MT3608

Current button pins: Up GPIO2, Down GPIO3, Enter GPIO4, and Back GPIO5. Before the final build, Down will be moved from GPIO3 to GPIO6 because GPIO3 is a strapping pin. Display: CS GPIO7, RST GPIO8, RS/DC GPIO9, MOSI GPIO10, and CLK GPIO11.

## Controls

- Up/Down changes the current selection.
- Enter opens a menu item or executes an explicit On/Off action.
- Back moves up one menu level and never triggers a switching action.
- `Power Off` in the main menu puts the ESP32 into deep sleep.
- To wake the device from this software-off state, hold Enter for at least three seconds. A shorter press returns the device to deep sleep.

Software power-off does not physically disconnect the power supply. A mechanical main power switch and display backlight shutdown are still part of the final power-management design.

## Local Configuration

~~~sh
cp include/secrets.example.h include/secrets.h
~~~

Then add the Wi-Fi credentials, the exactly verified Home Assistant base URL, and a revocable Long-Lived Access Token to `include/secrets.h`. This file is excluded from Git. Credentials must never appear in commits, logs, screenshots, or videos.

The confirmed Home Assistant area IDs are `wohnzimmer` and `arbeitszimmer`. Individual light and scene entity IDs will only be added during their respective development phases.

## Build and Flash

A local PlatformIO installation is required.

~~~sh
pio run
pio run -t upload
pio device monitor -b 115200
~~~

The project uses the `esp32-s3` environment. A successful build does not prove functionality on the physical hardware; the display, buttons, network connection, and power modes must be tested on the actual device.

## Version 1.0 Scope

Version 1.0 exclusively controls all lights in a configured room, individual lights, and lighting scenes in the Living Room and Office. Dimming, RGB control, media control, automatic discovery, cloud access, and OTA updates are intentionally outside the scope of this release.

## License

The source code is licensed under the MIT License described in [LICENSE](LICENSE).