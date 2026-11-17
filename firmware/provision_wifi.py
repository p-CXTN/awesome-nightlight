"""Save the Nightlight's home Wi-Fi settings over USB serial."""

import argparse
import getpass
import time

import serial


def wait_for(port, markers, seconds):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        line = port.readline().decode("utf-8", errors="replace").strip()
        if any(marker in line for marker in markers):
            return line
    raise TimeoutError("No response. Check the COM port and close other serial monitors.")


def report_scan(port):
    port.write(b"\nwifi-scan\n")
    deadline = time.monotonic() + 10
    while time.monotonic() < deadline:
        line = port.readline().decode("utf-8", errors="replace").strip()
        if line.startswith(("Scanning for ", "Home Wi-Fi candidate:", "Home Wi-Fi scan:")):
            print(line)
        if line.startswith("Home Wi-Fi scan:"):
            return
    print("No Wi-Fi scan result received.")


def provision(port_name, ssid, passphrase, temporary):
    with serial.Serial(port_name, 115200, timeout=0.25) as port:
        port.write(b"\nwifi-test\n" if temporary else b"\nwifi-setup\n")
        wait_for(port, ("Wi-Fi SSID:",), 5)
        port.write(ssid + b"\n")
        wait_for(port, ("Wi-Fi passphrase:",), 5)
        port.write(passphrase + b"\n")
        result = wait_for(port, ("Wi-Fi saved.", "Wi-Fi test started.", "Wi-Fi setup failed."), 5)
        print(result)
        if result.startswith("Wi-Fi setup failed."):
            return 1

        prefix = "Test Wi-Fi:" if temporary else "Home Wi-Fi:"
        reason = ""
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            line = port.readline().decode("utf-8", errors="replace").strip()
            if line.startswith(prefix) and " | IP: " in line:
                print(line)
                return 0
            if line.startswith("Wi-Fi disconnected:"):
                reason = line
        if reason:
            print(reason)
        print("The board did not get an IP address within 30 seconds.")
        if temporary:
            report_scan(port)
        return 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", help="The SuperMini serial port, for example COM3")
    parser.add_argument("--temporary", action="store_true", help="Try a network until restart without replacing saved credentials")
    args = parser.parse_args()
    ssid = input("2.4 GHz Wi-Fi name (SSID): ").encode("utf-8")
    passphrase = getpass.getpass("Wi-Fi passphrase: ").encode("utf-8")
    if not 1 <= len(ssid) <= 32 or not 8 <= len(passphrase) <= 63:
        parser.error("SSID must be 1–32 bytes and passphrase must be 8–63 bytes.")
    try:
        return provision(args.port, ssid, passphrase, args.temporary)
    except (OSError, TimeoutError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    raise SystemExit(main())
