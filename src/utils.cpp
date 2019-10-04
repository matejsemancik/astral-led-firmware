#include "utils.h"
#include <WiFi.h>
#include <WiFiUdp.h>

bool connect_wifi(const char* ssid, const char* pass) {
  bool state = true;
  int i = 0;

  WiFi.begin(ssid, pass);
  printf("Connecting to WiFi...\n");

  // Wait for connection
  printf("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    printf(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }

  printf("\n");

  if (state) {
    printf("Connected to: %s, IP address: %d.%d.%d.%d\n", ssid, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  } else {
    printf("Connection failed\n");
  }

  return state;
}