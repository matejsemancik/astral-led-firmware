# Astral LEDs
ESP32 made into Art-Net DMX512 node, that drives bunch of WS2812B LED strips over WiFi. Uses [PlatformIO](https://platformio.org/install/cli) build system with Arduino framework & FastLED library.

## Build & run
1. Get yourself the [Platformio CLI](https://platformio.org/install/cli) toolchain
2. Define `WIFI_SSID` and `WIFI_PASSWORD` constants in `include/private/private_constants.h` file.

	```
	// private_constants.h

	#define WIFI_SSID "YourHotspot"
	#define WIFI_PASSWORD "YourPassword"
	```

3. Configure `upload_port` in `platformio.ini` file.
4. Build & upload firmware `pio run -t upload`