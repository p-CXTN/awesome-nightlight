# Development

Run commands from the repository root. Install Python 3.11+ and Arduino CLI; put `arduino-cli` on PATH.

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.6 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
python scripts/build.py
```

The build packages matching OTA and 4 MB USB images in `firmware/images/`. These generated files are ignored by Git. Set `firmware/VERSION` before a release, or pass `python scripts/build.py --version 1.1.0`. Previous local releases are archived; never overwrite published releases.

Alternatively, open `firmware/NightlightRevK/NightlightRevK.ino` in Arduino IDE: ESP32 core **3.3.6**, **ESP32C3 Dev Module**, **USB CDC On Boot enabled**, **4 MB flash**, **Default 4MB with spiffs** partitions. No extra Arduino libraries are required.

## USB flash / recovery

Disconnect USB-A; connect the SuperMiniâ€™s USB-C. Install `python -m pip install -r requirements.txt`, then replace `PORT` and `USB_IMAGE` below with your serial port (e.g. `COM3` or `/dev/ttyACM0`) and the full `*-USB.bin` filename downloaded from [Releases](https://github.com/p-CXTN/awesome-nightlight/releases/latest) (or built in `firmware/images/`):

```sh
python -m esptool --chip esp32c3 --port PORT write-flash 0x0 USB_IMAGE
```

USB images erase saved settings. If connection fails, hold BOOT, tap RESET, release BOOT, and retry. Serial monitor: **115200 baud**, newline; commands: `status`, `test`, `stop`, `help`. `python firmware/provision_wifi.py PORT` prompts for Wi-Fi credentials without echoing the password.

## Checks

With `g++` on PATH:

```sh
python scripts/check.py
```

For dashboard tests, install Node.js 20+, then:

```sh
npm install --no-save --package-lock=false playwright
npx playwright install chromium
node firmware/tests/dashboard.cjs
```

Windows uses installed Edge by default; `BROWSER_CHANNEL` overrides it. Hardware acceptance: check all three LEDs, covered/uncovered lux, presence, first-time Wi-Fi setup, failed-password retry, network change, BOOT reset, and OTA reboot. Host tests do not verify the physical lamp.

## Publish a release

Update `firmware/VERSION`, commit source and instructions, tag the commit `v<version>`, and push the branch and tag. The Release workflow builds and checks firmware, packages hardware ZIPs, and publishes the files with checksums to GitHub Releases. Generated binaries stay out of Git. Never move a published release tag.
