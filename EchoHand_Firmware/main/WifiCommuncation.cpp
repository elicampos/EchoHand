#include "WifiCommuncation.h"

// Global array for servo angles
uint8_t servoTargetAngles[5];

// Global for received data(servos)
void on_data_receive(const esp_now_recv_info_t *esp_now_info, const uint8_t *incoming_data, int len)
{
    // Copy servo angles to local array
    memcpy(&servoTargetAngles, incoming_data, sizeof(servoTargetAngles));

    // Apply servo angles to persistent state
    for (int i = 0; i < 5; ++i)
    {
        DataBroker::instance().setServoTargetAngle(i, (180 * servoTargetAngles[i]) / 1000);
    }

    // Wake up Core 0 to update actual servos
    xTaskNotifyGive(xServoTaskHandle);
}

void TaskWifiCommunication(void *pvParameters)
{
    // uses parameter to avoid compiler error
    (void)pvParameters;

    // Data to send to other ESP32
    InputsPayload analog_read_info;

    // Echohand snapshot
    EchoStateSnapshot s;

    // Last recivison
    uint32_t lastRevision = 0;

    // Info of other ESP32 connected to PC
    esp_now_peer_info_t peerInfo;

    // MAC address of other ESP32
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Setup ESP32 WIFI Moudle
    WiFi.mode(WIFI_STA);

    // Check if it initalized
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error setting up ESP_NOW");
        return;
    }

    // Register Peer
    memcpy(peerInfo.peer_addr, broadcastAddress, sizeof(broadcastAddress));
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Can't find other ESP32!");
        return;
    }
    // Register callback for data received
    esp_now_register_recv_cb(on_data_receive);

    for (;;)
    {
        // Update Persistant State
        // Let's take a screenshot of the current persistent state
        DataBroker::instance().takeSnapshot(s);

        // If the revision has changed, update the input payload and send it over serial
        if (s.revision != lastRevision)
        {
            // update packed inputs
            for (uint8_t i = 0; i < 5; ++i)
            {
                analog_read_info.fingerAngles[i] = s.fingerAngles[i];
            }
            analog_read_info.joystickXY[0] = s.joystickXY[0];
            analog_read_info.joystickXY[1] = s.joystickXY[1];
            analog_read_info.buttonsBitmask = s.buttonsBitmask;

            // Send over payload over serial if not in debug print mode
            if (!DEBUG_PRINT)
            {
                // Build string and then send
                std::string outputString = "A" + std::to_string(analog_read_info.fingerAngles[0]) +
                                           "B" + std::to_string(analog_read_info.fingerAngles[1]) +
                                           "C" + std::to_string(analog_read_info.fingerAngles[2]) +
                                           "D" + std::to_string(analog_read_info.fingerAngles[3]) +
                                           "E" + std::to_string(analog_read_info.fingerAngles[4]) +
                                           "F" + std::to_string(analog_read_info.joystickXY[0]) +
                                           "G" + std::to_string(analog_read_info.joystickXY[1]);
                if ((analog_read_info.buttonsBitmask & TRIGGER_BUTTON_BITMASK))
                    outputString += "L";
                if (!(analog_read_info.buttonsBitmask & JOYSTICK_BUTTON_BITMASK))
                    outputString += "H";
                if ((analog_read_info.buttonsBitmask & A_BUTTON_BITMASK))
                    outputString += "J";
                if ((analog_read_info.buttonsBitmask & B_BUTTON_BITMASK))
                    outputString += "K";
                outputString += "\n";

                // C-Style
                const char *buffer = outputString.c_str();

                // Send the constructed string as one STRING(+ null terminator)
                esp_now_send(broadcastAddress, (uint8_t *)&buffer, outputString.size() + 1);
            }
            // update last revision to current
            lastRevision = s.revision;
        }
    }
}