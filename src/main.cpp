#include <Arduino.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "private/private_constants.h"
#include "utils.h"

// FastLED stuff

#define NUM_STRIPS 3
#define NUM_LEDS_PER_STRIP 150
#define NUM_LEDS NUM_STRIPS* NUM_LEDS_PER_STRIP
CRGB leds[NUM_LEDS];
uint8_t led_brightness = 255;

// ArtNet stuff

const char* wifi_ssid = WIFI_SSID;
const char* wifi_pass = WIFI_PASSWORD;
ArtnetWifi artnet;

uint8_t fastled_controller = 0;
uint16_t fastled_offset = 0;
uint16_t num_pixels = 0;

uint32_t packets_received = 0;
uint32_t millis_since_last_record = 0;

void dmx_callback(uint16_t universe,
                  uint16_t length,
                  uint8_t sequence,
                  uint8_t* data) {
  packets_received++;

  fastled_controller = universe;
  fastled_offset = universe * NUM_LEDS_PER_STRIP;

  if (fastled_controller >= NUM_STRIPS) {
    printf("Universe out of bounds: %d\n", universe);
    return;
  }

  for (uint16_t n = 0; n < NUM_LEDS_PER_STRIP; n++) {
    leds[n + fastled_offset] =
        CRGB(data[n * 3], data[n * 3 + 1], data[n * 3 + 2]);
  }

  FastLED[fastled_controller].showLeds(led_brightness);
}

void artnet_task(void* pvparams) {
  printf("ArtNet task started.\n");
  artnet.begin();
  artnet.setArtDmxCallback(dmx_callback);

  FastLED.addLeds<WS2812B, 12, GRB>(leds, NUM_LEDS_PER_STRIP * 0,
                                    NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 14, GRB>(leds, NUM_LEDS_PER_STRIP * 1,
                                    NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 27, GRB>(leds, NUM_LEDS_PER_STRIP * 2,
                                    NUM_LEDS_PER_STRIP);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  while (1) {
    artnet.read();
  }
}

void monitor_task(void* pvparams) {
  printf("Monitor task started.\n");
  while(1) {
    if (millis() > millis_since_last_record + 1000) {
      millis_since_last_record = millis();
      printf("Packet rate: %d packets / s\n", packets_received);
      packets_received = 0;
    }
  }
}

void setup() {
  bool wifi_connected = connect_wifi(wifi_ssid, wifi_pass);

  if (wifi_connected) {
    xTaskCreatePinnedToCore(artnet_task, "ArtNet task", 4096, NULL, 1, NULL,
                            CONFIG_ARDUINO_RUNNING_CORE);

    // xTaskCreatePinnedToCore(monitor_task, "Monitor task", 4096, NULL, 1, NULL,
    //                         CONFIG_ARDUINO_RUNNING_CORE);
  }
}

void loop() {}