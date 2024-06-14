# Divergence Meter

![Divergence Meter](https://github.com/Lyttr/divergence-meter/blob/main/divergencemeter.jpg)

## Description

The Divergence Meter is a project based on Arduino and ESP32, designed to mimic the appearance of a divergence meter from the anime "Steins;Gate." This project utilizes a DS3231 RTC module for accurate timekeeping and HC595 shift registers to control the display of digits. The divergence value is displayed on neon-like numerical tubes, providing an aesthetic reminiscent of the original device from the anime.

## Features

- Real-time clock (RTC) functionality using the DS3231 module
- Wireless configuration through an embedded web server
- Neon-like numerical tube display
- Adjustable divergence value settings

## Installation

1. Clone the repository: `git clone https://github.com/your_username/divergence-meter.git`
2. Install the necessary Arduino libraries:
   - WiFi.h
   - WebServer.h
   - ESPmDNS.h
   - RTClib.h
3. Upload the code to your Arduino or ESP32 board.

## Usage

1. Connect your Arduino or ESP32 board to power.
2. Connect to the access point (AP) created by the device (SSID: `divergencemeter`, Password: `12345678`).
3. Open a web browser and navigate to `http://192.168.4.1` to access the settings page.
4. Adjust the time settings as desired and click "Confirm" to save changes.
5. The divergence value will be displayed on the numerical tubes.

## License

This project is licensed under the [Creative Commons Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/).

## Credits

- [Open-source libraries used](#)
- Contributors: [Your Name](https://github.com/your_username)


