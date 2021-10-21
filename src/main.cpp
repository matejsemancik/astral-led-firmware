#include <Arduino.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "private/private_constants.h"
#include "utils.h"

const char *wifi_ssid = WIFI_SSID;
const char *wifi_pass = WIFI_PASSWORD;
ArtnetWifi artnet;

struct mapping_t {
  uint8_t universe;
  uint16_t buffer_start;
  uint16_t universe_start;
  uint16_t pixel_count;
};

// Let's not push buck converters too much. This still yelds super bright light
// and can drive 5m of WS2815s (300 pixels) just under 2.8 A
#define FASTLED_BRIGHTNESS 200

// 1 pixel = RGB = 3 Art-Net fixture channels
#define COLOR_CHANNELS 3

// Red channel byte offset
#define R_OFFSET 0
// Green channel byte offset
#define G_OFFSET 1
// Blue channel byte offset
#define B_OFFSET 2

// Number of universes that the ArtNet server will use
#define NUM_UNIVERSES 9

// Max number of LEDS in a single universe, must be <= 170 (<= 512 / 3)
#define NUM_LEDS_PER_UNIVERSE 120
#define BUFFER_SIZE NUM_UNIVERSES *NUM_LEDS_PER_UNIVERSE

CRGB led_buffer[BUFFER_SIZE];

// Make sure you have a mapping for each universe
mapping_t mappings[NUM_UNIVERSES] = {
    // Strand 1
    {.universe = 0, .buffer_start = 0, .universe_start = 0, .pixel_count = 120},
    {.universe = 1,
     .buffer_start = 120,
     .universe_start = 0,
     .pixel_count = 120},
    {.universe = 2,
     .buffer_start = 240,
     .universe_start = 0,
     .pixel_count = 60},

    // Strand 2
    {.universe = 3,
     .buffer_start = 300,
     .universe_start = 0,
     .pixel_count = 120},
    {.universe = 4,
     .buffer_start = 420,
     .universe_start = 0,
     .pixel_count = 120},
    {.universe = 5,
     .buffer_start = 540,
     .universe_start = 0,
     .pixel_count = 60},

    // Strand 3
    {.universe = 6,
     .buffer_start = 600,
     .universe_start = 0,
     .pixel_count = 120},
    {.universe = 7,
     .buffer_start = 720,
     .universe_start = 0,
     .pixel_count = 120},
    {.universe = 8,
     .buffer_start = 840,
     .universe_start = 0,
     .pixel_count = 60}};

void dmx_callback(uint16_t universe, uint16_t length, uint8_t sequence,
                  uint8_t *data) {

  if (universe >= NUM_UNIVERSES) {
    printf("Universe (%d) out of bounds\n", universe);
    return;
  }

  mapping_t mapping = mappings[universe];
  for (uint16_t pixel = 0; pixel < mapping.pixel_count; pixel++) {
    led_buffer[mapping.buffer_start + pixel] = CRGB(
        data[mapping.universe_start + (pixel * COLOR_CHANNELS + R_OFFSET)],
        data[mapping.universe_start + (pixel * COLOR_CHANNELS + G_OFFSET)],
        data[mapping.universe_start + (pixel * COLOR_CHANNELS + B_OFFSET)]);
  }
}

void artnet_task(void *pvparams) {
  printf("ArtNet task started.\n");
  artnet.begin();
  artnet.setArtDmxCallback(dmx_callback);

  // LED Mapping - 3 strands of LEDs per 300LEDs on 3 different pins, sharing
  // the same buffer
  FastLED.addLeds<WS2812B, 12, GRB>(led_buffer, 0, 300);
  FastLED.addLeds<WS2812B, 14, GRB>(led_buffer, 300, 300);
  FastLED.addLeds<WS2812B, 27, GRB>(led_buffer, 600, 300);

  // Global Brightness
  FastLED.setBrightness(FASTLED_BRIGHTNESS);

  // Clean up canvas
  fill_solid(led_buffer, BUFFER_SIZE, CRGB::Black);
  FastLED.show();

  while (1) {
    artnet.read();
  }
}

void fastled_task(void *pvparams) {
  printf("FastLED task started.\n");

  while (1) {
    FastLED.show();
    delay(10);
  }
}

void setup() {
  bool wifi_connected = connect_wifi(wifi_ssid, wifi_pass);

  if (wifi_connected) {
    xTaskCreatePinnedToCore(artnet_task, "ArtNet task", 4096, NULL, 1, NULL,
                            CONFIG_ARDUINO_RUNNING_CORE);

    xTaskCreatePinnedToCore(fastled_task, "FastLED task", 4096, NULL, 1, NULL,
                            CONFIG_ARDUINO_RUNNING_CORE);
  }
}

void loop() {}