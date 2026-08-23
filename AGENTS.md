# Repository Guidelines

## Project Structure & Module Organization

- `src/main.cpp` contains the ESP32-S3 firmware: button handling, TFT rendering, Wi-Fi setup, and Home Assistant REST calls.
- `include/secrets.example.h` documents required local configuration. Copy it to the ignored `include/secrets.h`; never add real credentials to tracked files.
- `platformio.ini` defines the `esp32-s3` PlatformIO environment, Arduino framework, board settings, and TFT dependency.
- `.githooks/pre-commit` rejects generated files and likely secrets. There is currently no separate `test/` or asset directory. Add PlatformIO tests under `test/<feature>/` when extracting testable logic.

## Build, Test, and Development Commands

Run commands from the repository root:

```sh
cp include/secrets.example.h include/secrets.h  # create local configuration
pio run                                         # compile the firmware
pio run -t upload                               # flash the connected ESP32-S3
pio device monitor -b 115200                    # view serial output
pio run -t clean                                # remove PlatformIO build output
git config core.hooksPath .githooks             # enable repository hooks
```

The environment is named `esp32-s3`; use `pio run -e esp32-s3` when selecting it explicitly.

## Coding Style & Naming Conventions

Use C++ compatible with the Arduino toolchain and four-space indentation. Place opening braces on the same line as declarations and control statements. Follow the existing naming: `camelCase` for functions and variables, `UPPER_SNAKE_CASE` for compile-time constants, and `PascalCase` for structs. Prefer `constexpr` for pin assignments and fixed values. Keep hardware mappings grouped and documented, bound fixed-buffer writes with `snprintf`, and avoid long blocking work in `loop()`.

## Testing Guidelines

No automated tests or coverage requirement currently exists. Every change must at least pass `pio run`. For UI, input, Wi-Fi, or API changes, test on the target board and describe the setup and observed serial/display behavior in the pull request. Name future PlatformIO test files `test_<behavior>.cpp` and keep hardware-independent logic isolated where practical.

## Commit & Pull Request Guidelines

History currently contains only `Initial commit`, so no established commit convention exists. Use short, imperative subjects such as `Add Wi-Fi reconnect status`. Keep commits focused. Pull requests should explain the user-visible or hardware impact, list validation commands, link relevant issues, and include display photos or serial excerpts when behavior changes.

## Security & Configuration

Never commit `include/secrets.h`, Home Assistant tokens, Wi-Fi passwords, private keys, or `.env` files. Review staged changes with `git diff --cached` and keep the pre-commit hook enabled.
