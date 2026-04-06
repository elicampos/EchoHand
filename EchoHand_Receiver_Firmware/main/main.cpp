#include <cstring>
#include <cstdio>
#include <math.h>
#include <string>
#include <WiFi.h>
#include <esp_now.h>
#include "Arduino.h"

// Global to copy analog data
char analog_data[56];

// Is there valid data
bool new_data = false;

// Global for received data(finger angles, buttons and etc)
void on_data_receive(const esp_now_recv_info_t *esp_now_info, const uint8_t *incoming_data, int len)
{
  // Copy into output string
  memcpy(analog_data, incoming_data, len);

  // Print valid data
  new_data = true;
}

extern "C" void app_main()
{
  // Initalize Arduino
  initArduino();

  Serial.begin(115200);

  // Info of other ESP32 connected to PC
  esp_now_peer_info_t peerInfo;

  // Global array for servo angles
  uint8_t servoTargetAngles[5];

  // MAC address of other ESP32
  uint8_t broadcastAddress[] = {0x80, 0xB5, 0x4E, 0xC3, 0x20, 0x2C};

  // Setup ESP32 WIFI Moudle
  WiFi.mode(WIFI_STA);

  // Check if it initalized
  while (esp_now_init() != ESP_OK)
  {
    Serial.println("Error setting up ESP_NOW");
  }

  // Print ESP32 MAC Address
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Clear struct
  memset(&peerInfo, 0, sizeof(peerInfo));

  // Register Peer
  memcpy(peerInfo.peer_addr, broadcastAddress, sizeof(broadcastAddress));
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Set Wi-FI interface to station mode
  peerInfo.ifidx = WIFI_IF_STA;

  while (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer MAC Address.");
  }

  // Register callback for data received
  esp_now_register_recv_cb(on_data_receive);

  // String for Servo payload
  char output_string[256];

  for (;;)
  {
    // Check if we got a servo packet
    // If we have data available to read, parse it and update servo targets
    if (Serial.available() > 10)
    {
      // Use readBytesUntil to safely read a full line into the buffer with a timeout, unlike with just using read
      size_t len = Serial.readBytesUntil('\n', output_string, sizeof(output_string) - 1);

      // make sure last char is null terminator
      output_string[len] = '\0';

      // Send the constructed string as one STRING(+ null terminator)
      // len+1 since arrays are 0 indexed
      esp_now_send(broadcastAddress, (uint8_t *)output_string, len + 1);
    }

    if (new_data == true)
    {
      // Write until size of analog data(56 bytes) or null terminator met
      Serial.write(analog_data, strlen(analog_data));

      // Mark data as old
      new_data = false;
    }

    vTaskDelay(1);
  }
}
