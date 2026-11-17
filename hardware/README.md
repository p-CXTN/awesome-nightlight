# Rev K hardware

52 Ã— 52 mm, two-layer carrier; ESP32-C3 SuperMini, LD2410C, GY-302/BH1750, three LED channels. USB-A supplies power.

- Edit/import: `revK-native/Nightlight-Schematic.json` and `Nightlight-PCB.json` in EasyEDA/JLCEDA Standard, or download `Nightlight-RevK-JLCEDA-Standard.zip` from [Releases](https://github.com/p-CXTN/awesome-nightlight/releases/latest) and import it into JLCEDA Professional.
- View: `revK-native/schematic.svg`, `pcb.svg`, `assembly-plan.png`.
- Parts and assembly: `revK-native/BOM.csv`, `revK-native/IMPORT.txt`, and `measured-parts.json`.
- Fabrication exports: the Gerber and JLC upload ZIPs attached to [Releases](https://github.com/p-CXTN/awesome-nightlight/releases/latest). Re-export after design changes and inspect the fabrication preview; run native ERC/DRC before ordering.

Install `python -m pip install -r hardware/requirements.txt`. From `hardware/revK-native`, run `python check_placement.py` and `python validate.py` to check the supplied design. To regenerate after edits: `python build.py`, `python check_placement.py`, `python route.py`, `python build.py`, `python validate.py`, `python render.py`, `python assembly_plan.py`. Optional PNG previews: `python -m playwright install chromium`, then `python screenshot_local.py`.

`build.py` generates native files from its component/footprint definitions and `routes.json`; it overwrites direct EDA edits. Preserve/export EDA edits before regenerating. Routing may take several minutes. Module offsets and LED fit have remaining measurement limits documented in `IMPORT.txt`; geometric checks do not replace physical verification.
