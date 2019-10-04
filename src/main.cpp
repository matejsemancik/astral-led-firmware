#include <Arduino.h>
#include <FastLED.h>

// #include <FastLED.h>

#define LEDS_DATA_PIN 27
#define NUM_LEDS 150  // 30 LEDS x 5m on strip
#define FPS 60
CRGB pixels[NUM_LEDS];

void led_runner(void* pvparams) {
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

void setup() {
  printf("Running LEDs...\n");
  xTaskCreatePinnedToCore(led_runner, "FastLED runner", 4096, NULL, 2, NULL,
                          CONFIG_ARDUINO_RUNNING_CORE);
}

void loop() {}