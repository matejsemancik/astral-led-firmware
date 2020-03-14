#include <Arduino.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "private/private_constants.h"
#include "utils.h"

// FastLED stuff

#define NUM_BRANCHES 3
#define LEDS_PER_BRANCH 150
CRGB leds[NUM_BRANCHES][LEDS_PER_BRANCH];

// ArtNet stuff

#define UNIVERSE_START 2
const char* wifi_ssid = WIFI_SSID;
const char* wifi_pass = WIFI_PASSWORD;
ArtnetWifi artnet;

int prev_data_len = 0;
const int start_universe = 0;

void dmx_callback(uint16_t universe,
                  uint16_t length,
                  uint8_t sequence,
                  uint8_t* data) {
  for (int i = 0; i < length / 3; i++) {
    int led = i + (universe - start_universe) * (prev_data_len / 3);
    if (led < LEDS_PER_BRANCH) {
      leds[universe][led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }
  prev_data_len = length;
  FastLED.show();
}

void artnet_task(void* pvparams) {
  printf("ArtNet task started.\n");
  artnet.begin();
  artnet.setArtDmxCallback(dmx_callback);

  FastLED.addLeds<WS2812B, 12, GRB>(leds[0], LEDS_PER_BRANCH);
  FastLED.addLeds<WS2812B, 14, GRB>(leds[1], LEDS_PER_BRANCH);
  FastLED.addLeds<WS2812B, 27, GRB>(leds[2], LEDS_PER_BRANCH);

  while (1) {
    artnet.read();
  }
}

void setup() {
  bool wifi_connected = connect_wifi(wifi_ssid, wifi_pass);

  if (wifi_connected) {
    xTaskCreatePinnedToCore(artnet_task, "ArtNet task", 4096, NULL, 1, NULL,
                            CONFIG_ARDUINO_RUNNING_CORE);
  }
}

void loop() {}