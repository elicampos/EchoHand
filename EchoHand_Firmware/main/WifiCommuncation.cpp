#include "WifiCommuncation.h"

// Global array for servo angles
char incoming_servo_data[256];

// Global array for servo data
bool new_servo_data = false;

// Global for received data(servos)
void on_data_receive(const esp_now_recv_info_t *esp_now_info, const uint8_t *incoming_data, int len)
{
    // Copy servo angles to local array
    memcpy(&incoming_servo_data, incoming_data, sizeof(incoming_servo_data));

    new_servo_data = true;
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
    uint8_t broadcastAddress[] = {0x30, 0xED, 0xA0, 0xBC, 0x0B, 0x34};

    // Setup ESP32 WIFI Moudle
    WiFi.mode(WIFI_STA);

    // Check if it initalized
    if (esp_now_init() != ESP_OK)
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

    // Set Temps for parsing servo data
    char outputsString[56];
    char *currentByte;
    char command;
    int intValue = 0;

    for (;;)
    {
        // If there's new servo data
        if (new_servo_data)
        {
            currentByte = incoming_servo_data;

            // Parse data and populate struct
            // Process the entire buffer received in memory
            while (*currentByte != '\0')
            {
                // Check for Commands A-E (Servo Angles)
                if (*currentByte >= 'A' && *currentByte <= 'E')
                {
                    command = *currentByte;
                    currentByte++;

                    intValue = 0;

                    // Parse integer part
                    while (*currentByte >= '0' && *currentByte <= '9')
                    {
                        intValue = (intValue * 10) + (*currentByte - '0');
                        currentByte++;
                    }

                    // Apply servo angles to persistent state
                    DataBroker::instance().setServoTargetAngle(command - 'A', (180 * intValue) / 1000);

                    // Once last byte of servo has been processed(E, wakeup thread instantly to update value)
                    if (command == 'E' && xServoTaskHandle != NULL)
                    {
                        xTaskNotifyGive(xServoTaskHandle);
                    }
                }
                else
                {
                    ++currentByte;
                }

                new_servo_data = false;
            }
        }
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
                esp_now_send(broadcastAddress, (const uint8_t *)buffer, outputString.size() + 1);
            }
            // update last revision to current
            lastRevision = s.revision;
        }
    }
}