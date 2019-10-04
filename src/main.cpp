#include <Arduino.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "passwords.h"
#include "utils.h"

// FastLED stuff

#define LEDS_DATA_PIN 27
#define NUM_LEDS 150  // 30 LEDS x 5m on strip
#define FPS 60
CRGB pixels[NUM_LEDS];

// ArtNet stuff

const char* wifi_ssid = WIFI_SSID;
const char* wifi_pass = WIFI_PASSWORD;
// Total number of DMX channels (1 LED = 3 channels)
const uint16_t num_channels = NUM_LEDS * 3;

ArtnetWifi artnet;
const int start_universe = 0;

void led_runner(void* pvparams) {
  printf("FastLED runner started.\n");

  FastLED.addLeds<WS2812B, LEDS_DATA_PIN, RGB>(pixels, NUM_LEDS);
  FastLED.setBrightness(255);

  while (1) {
    uint8_t time = millis() / 10;
    uint8_t sin = sin8(time);
    fill_solid(pixels, NUM_LEDS, CHSV(0, 255, sin));
    FastLED.show();
    FastLED.delay(1000 / FPS);
  }
}

void dmx_callback(uint16_t universe,
                  uint16_t length,
                  uint8_t sequence,
                  uint8_t* data) {
                    // TODO add implementation
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