#include "utils.h"
#include <WiFi.h>
#include <WiFiUdp.h>

bool connect_wifi(const char* ssid, const char* pass) {
  bool state = true;
  int i = 0;

  WiFi.begin(ssid, pass);
  printf("Connecting to %s ...\n", ssid);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    printf(".\n");
    // printf(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }

  if (state) {
    printf("Connected to: %s, IP address: %d.%d.%d.%d\n", ssid, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  } else {
    printf("Connection failed\n");
  }

  return state;
}