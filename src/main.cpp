#include <Arduino.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "private/private_constants.h"
#include "utils.h"

// FastLED stuff

#define LEDS_DATA_PIN 27
#define NUM_LEDS 150  // 30 LEDS x 5m on strip
#define FPS 60
CRGB leds[NUM_LEDS];

// ArtNet stuff

const char* wifi_ssid = WIFI_SSID;
const char* wifi_pass = WIFI_PASSWORD;
ArtnetWifi artnet;
const uint16_t num_channels = NUM_LEDS * 3;  // 1 LED = 3 channels

void led_runner(void* pvparams) {
  printf("FastLED runner started.\n");

  FastLED.addLeds<WS2812B, LEDS_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);

  while (1) {
    FastLED.setBrightness(200);
    FastLED.show();
    FastLED.delay(1000 / FPS);
  }
}

void dmx_callback(uint16_t universe,
                  uint16_t length,
                  uint8_t sequence,
                  uint8_t* data) {
  
  printf("universe: %d, len: %d\n", universe, length);
  if (universe == 0) {
    // FFT data incoming
    return;
  }

  if (universe == 1 && length <= NUM_LEDS * 3) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (uint16_t i = 0; i < length / 3; i++) {
      leds[i] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }
}

void artnet_runner(void* pvparams) {
  printf("ArtNet runner started.\n");
  artnet.begin();
  artnet.setArtDmxCallback(dmx_callback);

  while (1) {
    artnet.read();
  }
}

void setup() {
  bool wifi_connected = connect_wifi(wifi_ssid, wifi_pass);

  if (wifi_connected) {
    xTaskCreatePinnedToCore(led_runner, "FastLED runner", 4096, NULL, 2, NULL,
                            CONFIG_ARDUINO_RUNNING_CORE);

    xTaskCreatePinnedToCore(artnet_runner, "ArtNet runner", 4096, NULL, 2, NULL,
                            CONFIG_ARDUINO_RUNNING_CORE);
  }
}

void loop() {}