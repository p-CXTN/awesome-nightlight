# Coding instructions

- Start with `README.md` and `DEVELOPMENT.md`. Keep user documentation concise; keep discussion and progress reports out of deliverables.
- Firmware: `firmware/NightlightRevK/`. Main sketch: pins, sensors, loop, serial commands. `Lamp.h`: lighting state machine. `Radar.*`: LD2410C protocol. `Updater.*`: Wi-Fi, settings, HTTP, OTA. `Dashboard.h` / `WifiPage.h`: embedded HTML/CSS/JS.
- Preserve Rev K pins: LEDs 10/0/1; radar OUT 3, RX 6, TX 7; I2C SDA 4/SCL 5; BOOT 9; active-low onboard LED 8. Preserve nonblocking timing and fail-off behavior when lux is unavailable.
- Do not change stored settings formats, OTA partitions, or Wi-Fi recovery behavior without a migration/recovery plan. Keep credentials and device dumps out of Git.
- Run `python scripts/check.py`; run dashboard tests for UI/network changes and `python scripts/build.py` for firmware changes. Report checks actually run and hardware checks still needed.
- Publish both versioned OTA and USB binaries, release.json, and SHA256SUMS as GitHub release assets, never as tracked files. Update `firmware/VERSION` for a new release. Local outputs and archives live in ignored `firmware/images/`; use the packager, never hand-edit manifests.
- Hardware: `hardware/revK-native/`; see `hardware/README.md`. Preserve pin/net assignments and measured geometry. Run placement/connectivity checks after design edits; native EDA ERC/DRC remains necessary.
