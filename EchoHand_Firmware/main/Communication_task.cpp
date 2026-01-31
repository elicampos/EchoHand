#include "Communication_task.h"

// Defining packed payloads
#pragma pack(push, 1) // pack the structs tp prevent padding between fields. this way the size of the struct is always the same.
struct InputsPayload
{
  int fingerAngles[5];
  int joystickXY[2];
  uint32_t buttonsBitmask;
  uint8_t batteryPercent;
};
struct OutputsPayload
{
  uint16_t vibrationRPMs[5];
  int servoTargetAngles[5];
};
#pragma pack(pop)

void TaskCommunication(void *pvParameters)
{
  // uses parameter to avoid compiler error
  (void)pvParameters;

  // Initialize Bluetooth Serial if enabled in config.h
  HardwareSerial *mySerial = &Serial;

  if (USE_BLUETOOTH_SERIAL)
  {
    // Print to usb that we are starting bluetooth serial
    mySerial->println("Starting Bluetooth Serial on another COM port...");
    mySerial = &Serial1;
    mySerial->begin(38400, SERIAL_8N1, 18, 17);

    // Time to config
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  EchoStateSnapshot s;
  InputsPayload in{};
  char outputsString[56];
  char *currentByte;
  char command;
  int intValue = 0;
  float floatValue = 0.0f;
  int8_t charCount = 0;

  // set finger splay and leave it
  mySerial->printf("(AB)511(BB)511(CB)511(DB)511(EB)511\n");

  // bluetooth task loop
  uint32_t lastRevision = 0;
  for (;;)
  {

    if (!DEBUG_PRINT)
    {
      // If we have data available to read, parse it and update servo targets and vibration RPMs
      if (mySerial->available())
      {
        // Use readBytesUntil to safely read a full line into the buffer with a timeout, unlike with just using read
        size_t len = mySerial->readBytesUntil('\n', outputsString, sizeof(outputsString) - 1);

        // make sure last char is null terminator
        outputsString[len] = '\0';

        // Point to start of buffer
        currentByte = outputsString;

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
          }

          // Check for Command F (Vibration)
          else if (*currentByte == 'F')
          {
            currentByte++;

            floatValue = 0.0f;
            float fraction = 0.1f;
            bool isFraction = false;

            // Parse float part
            while ((*currentByte >= '0' && *currentByte <= '9') || *currentByte == '.')
            {
              if (*currentByte == '.')
              {
                isFraction = true;
              }
              else
              {
                if (!isFraction)
                {
                  floatValue = (floatValue * 10) + (*currentByte - '0');
                }
                else
                {
                  floatValue += (*currentByte - '0') * fraction;
                  fraction *= 0.1f;
                }
              }
              currentByte++;
            }

            // Set all motors to the value expected (in RPM)
            for (int i = 0; i < 5; i++)
            {
              DataBroker::instance().setVibrationRPM(i, floatValue * 60);
            }
          }
          // If not a recognized command, skip the byte
          else
          {
            currentByte++;
          }
        }
      }

      // Let's take a screenshot of the current persistent state
      DataBroker::instance().takeSnapshot(s);

      // If the revision has changed, update the input payload and send it over serial
      if (s.revision != lastRevision)
      {
        // update packed inputs
        for (uint8_t i = 0; i < 5; ++i)
        {
          in.fingerAngles[i] = s.fingerAngles[i];
        }
        in.joystickXY[0] = s.joystickXY[0];
        in.joystickXY[1] = s.joystickXY[1];
        in.buttonsBitmask = s.buttonsBitmask;
        in.batteryPercent = s.batteryPercent;

        // Send over payload over serial if not in debug print mode
        if (!DEBUG_PRINT)
        {
          // Build string and then send
          std::string outputString = "A" + std::to_string(in.fingerAngles[0]) +
                                     "B" + std::to_string(in.fingerAngles[1]) +
                                     "C" + std::to_string(in.fingerAngles[2]) +
                                     "D" + std::to_string(in.fingerAngles[3]) +
                                     "E" + std::to_string(in.fingerAngles[4]) +
                                     "F" + std::to_string(in.joystickXY[0]) +
                                     "G" + std::to_string(in.joystickXY[1]);
          if ((in.buttonsBitmask & TRIGGER_BUTTON_BITMASK))
            outputString += "L";
          if (!(in.buttonsBitmask & JOYSTICK_BUTTON_BITMASK))
            outputString += "H";
          if ((in.buttonsBitmask & A_BUTTON_BITMASK))
            outputString += "J";
          if ((in.buttonsBitmask & B_BUTTON_BITMASK))
            outputString += "K";
          outputString += "\n";

          // Send the constructed string as one STRING(SUPER SUPER IMPORTANT for bluetooth serial)
          mySerial->print(outputString.c_str());
        }
        // update last revision to current
        lastRevision = s.revision;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}