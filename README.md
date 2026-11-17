# Nightlight

ESP32-C3 SuperMini nightlight with LD2410C presence detection, BH1750 light sensing, three LEDs, and a local web dashboard. Includes Rev K PCB, schematics, BOM, and firmware.

## First setup

1. Power the lamp through USB-A. Join **Nightlight-xxxxxx** (no password); stay connected despite the “no internet” warning.
2. Open **http://192.168.4.1**, select your **2.4 GHz Wi-Fi**, enter its password, and choose **Check connection and save**.
3. After restart, rejoin your home Wi-Fi and open the displayed IP or `nightlight-xxxxxx.local`. If missing, find the IP in your router’s client list.

If the lamp still has another network saved, hold **BOOT for 10 seconds while powered on**, then release to reopen setup.

## Manage

- **Settings:** brightness, light thresholds, and radar sensitivity. **Testing:** LEDs and sensor readings; **Stop** restores automatic lighting.
- **Wi-Fi:** change networks. BOOT reset clears Wi-Fi/IP settings only; lamp/radar settings remain.
- **Update:** download the `*-OTA.bin` from [Releases](https://github.com/p-CXTN/awesome-nightlight/releases/latest) and upload it; keep power connected until restart. Updates retain settings. Never upload the USB image here.
- Setup Wi-Fi is open; the dashboard has no login. Use a trusted local network and trusted firmware.

## Develop or flash a blank board

See [development instructions](DEVELOPMENT.md) and [agent instructions](AGENTS.md). [Hardware files](hardware/README.md) include the schematic, PCB, and BOM. Firmware downloads and `SHA256SUMS` are attached to [GitHub Releases](https://github.com/p-CXTN/awesome-nightlight/releases/latest).

Project source and design files: [MIT license](LICENSE). External toolchains and their code retain their own licenses; see [third-party notices](THIRD_PARTY.md).
