#include <Arduino.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "private/private_constants.h"
#include "utils.h"

// FastLED stuff

#define LEDS_DATA_PIN 12
#define NUM_LEDS 150  // 30 LEDS x 5m
CRGB leds[NUM_LEDS];
uint8_t fading = 0;

// ArtNet stuff

#define UNIVERSE_START 2
const char* wifi_ssid = WIFI_SSID;
const char* wifi_pass = WIFI_PASSWORD;
ArtnetWifi artnet;

int previousDataLength = 0;
const int startUniverse = 0;

void dmx_callback(uint16_t universe,
                  uint16_t length,
                  uint8_t sequence,
                  uint8_t* data) {
                    
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < NUM_LEDS)
    {
      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }
  previousDataLength = length;     
  FastLED.show();
}

void artnet_runner(void* pvparams) {
  printf("ArtNet runner started.\n");
  artnet.begin();
  artnet.setArtDmxCallback(dmx_callback);

  FastLED.addLeds<WS2812B, LEDS_DATA_PIN, GRB>(leds, NUM_LEDS);

  while (1) {
    artnet.read();
  }
}

void setup() {
  bool wifi_connected = connect_wifi(wifi_ssid, wifi_pass);

  if (wifi_connected) {
    xTaskCreatePinnedToCore(artnet_runner, "ArtNet runner", 4096, NULL, 1, NULL,
                            CONFIG_ARDUINO_RUNNING_CORE);
  }
}

void loop() {}