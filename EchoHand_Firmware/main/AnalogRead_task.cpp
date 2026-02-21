#include "AnalogRead_task.h"

// Description: Averages value read from an analog pin based off sample rate in config.h
// Parameters: rawanalog pin
// Return: sum value given sample rate
// Note: This will be deprecated if we move to ExpressIf Ide as we can do this via hardware
int readSmooth(int pin)
{
    switch (POLL_METHOD)
    {
    case 0:
        return analogReadMilliVolts(pin);
    case 1:
    {
        //--Basic Average
        long long sum = 0;
        for (int i = 0; i < POT_SAMPLE_RATE; i++)
        {
            sum += analogReadMilliVolts(pin);
        }
        return sum / POT_SAMPLE_RATE;
    }
    case 2:
    {
        //--Median filter

        // Store all readings in dynamic array and sort then take median value to avoid outliers
        std::vector<int> readings;
        readings.reserve(POT_SAMPLE_RATE);
        for (int i = 0; i < POT_SAMPLE_RATE; i++)
        {
            readings.push_back(analogReadMilliVolts(pin));
        }

        // Get middle index(round down to middle value if even)
        uint8_t n = readings.size() / 2;

        // Sort vector to only have that value sorted
        // Makes values to the left less than median and values to the right greater(still not ordered)
        std::nth_element(readings.begin(), readings.begin() + n, readings.end());

        int median = readings[n];

        // If even, average two elements that would be in the mean
        if (!(readings.size() & 1))
        {
            auto max_it = std::max_element(readings.begin(), readings.begin() + n);
            median = (*max_it + median) / 2;
        }

        return median;
    }
    case 3:
    {
        // --Trimmed mean filter
        std::vector<int> readings;
        readings.reserve(POT_SAMPLE_RATE);
        for (int i = 0; i < POT_SAMPLE_RATE; i++)
        {
            readings.push_back(analogReadMilliVolts(pin));
        }
        std::sort(readings.begin(), readings.end());

        // Remove top and bottom 25% of readings to avoid outliers
        long long sum = 0;
        int trimIndexStart = POT_SAMPLE_RATE / 4;
        int trimIndexEnd = POT_SAMPLE_RATE - trimIndexStart;
        for (int i = trimIndexStart; i < trimIndexEnd; i++)
        {
            sum += readings[i];
        }
        return sum / (trimIndexEnd - trimIndexStart);
    }
    default:
        return analogReadMilliVolts(pin);
    }
}

// Description: Reads all Analog Data from sensors
// Parameters: pvParameters which is a place holder for any pointer to any type
// Return: none, it will simply pass the information on to the next core for processing
void TaskAnalogRead(void *pvParameters)
{

    // To not get compiler unused variable error
    (void)pvParameters;

    // Setup pins as INPUT just in case
    pinMode(THUMB_POT, INPUT);
    pinMode(INDEX_POT, INPUT);
    pinMode(MIDDLE_POT, INPUT);
    pinMode(RING_POT, INPUT);
    pinMode(PINKIE_POT, INPUT);
    pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);
    pinMode(A_BUTTON, INPUT);
    pinMode(B_BUTTON, INPUT);

    // Get range from 0.0V to 3.3V
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);

    // Calibration phase
    long long maxThumbValue = 4095, maxIndexValue = 4095, maxMiddleValue = 4095, maxRingValue = 4095, maxPinkieValue = 4095;
    long long minThumbValue = 0, minIndexValue = 0, minMiddleValue = 0, minRingValue = 0, minPinkieValue = 0;
    long long currentThumb = 0, currentIndex = 0, currentMiddle = 0, currentRing = 0, currentPinkie = 0;

    // If not simulation don't calibration
    if (!SIMULATION)
    {

        maxThumbValue = 0, maxIndexValue = 0, maxMiddleValue = 0, maxRingValue = 0, maxPinkieValue = 0;
        minThumbValue = 0, minIndexValue = 0, minMiddleValue = 0, minRingValue = 0, minPinkieValue = 0;
        currentThumb = 0, currentIndex = 0, currentMiddle = 0, currentRing = 0, currentPinkie = 0;

        // For first 5 seconds, assume hand is flexed(open as hard as possible) and record that as max flex
        // Get current time
        unsigned long long startTime = millis();
        long long counter = 0;

        // If we are getting the extreme set a values to opposites
        if (CALIBRATION_METHOD == 1)
        {
            maxThumbValue = LONG_LONG_MIN;
            maxIndexValue = LONG_LONG_MIN;
            maxMiddleValue = LONG_LONG_MIN;
            maxRingValue = LONG_LONG_MIN;
            maxPinkieValue = LONG_LONG_MIN;

            minThumbValue = LONG_LONG_MAX;
            minIndexValue = LONG_LONG_MAX;
            minMiddleValue = LONG_LONG_MAX;
            minRingValue = LONG_LONG_MAX;
            minPinkieValue = LONG_LONG_MAX;
        }

        // Check if 5 seconds has passed
        while (millis() - startTime < 5000)
        {
            // Read current values
            currentThumb = readSmooth(THUMB_POT);
            currentIndex = readSmooth(INDEX_POT);
            currentMiddle = readSmooth(MIDDLE_POT);
            currentRing = readSmooth(RING_POT);
            currentPinkie = readSmooth(PINKIE_POT);

            if (CALIBRATION_METHOD == 0)
            {
                counter++;

                maxThumbValue += currentThumb;
                maxIndexValue += currentIndex;
                maxMiddleValue += currentMiddle;
                maxRingValue += currentRing;
                maxPinkieValue += currentPinkie;
            }
            else
            {
                // Capture Observed Min and Max for each finger and update it ehere

                // Update Maximums
                if (currentThumb > maxThumbValue)
                    maxThumbValue = currentThumb;
                if (currentIndex > maxIndexValue)
                    maxIndexValue = currentIndex;
                if (currentMiddle > maxMiddleValue)
                    maxMiddleValue = currentMiddle;
                if (currentRing > maxRingValue)
                    maxRingValue = currentRing;
                if (currentPinkie > maxPinkieValue)
                    maxPinkieValue = currentPinkie;
            }
        }
        if (CALIBRATION_METHOD == 0)
        {
            maxThumbValue /= counter;
            maxIndexValue /= counter;
            maxMiddleValue /= counter;
            maxRingValue /= counter;
            maxPinkieValue /= counter;
        }

        // Blink LED to let user know to start closing hand
        digitalWrite(LED_BUILTIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));

        // For next 5 seconds, assume hand is closed(clench as hard as possible) and record that as min flex
        // Get current time
        startTime = millis();
        counter = 0;

        // Check if 5 seconds has passed
        while (millis() - startTime < 5000)
        {
            // Read current values
            currentThumb = readSmooth(THUMB_POT);
            currentIndex = readSmooth(INDEX_POT);
            currentMiddle = readSmooth(MIDDLE_POT);
            currentRing = readSmooth(RING_POT);
            currentPinkie = readSmooth(PINKIE_POT);

            if (CALIBRATION_METHOD == 0)
            {
                counter++;

                minThumbValue += currentThumb;
                minIndexValue += currentIndex;
                minMiddleValue += currentMiddle;
                minRingValue += currentRing;
                minPinkieValue += currentPinkie;
            }
            else
            {
                // Update Minimums
                if (currentThumb < minThumbValue)
                    minThumbValue = currentThumb;
                if (currentIndex < minIndexValue)
                    minIndexValue = currentIndex;
                if (currentMiddle < minMiddleValue)
                    minMiddleValue = currentMiddle;
                if (currentRing < minRingValue)
                    minRingValue = currentRing;
                if (currentPinkie < minPinkieValue)
                    minPinkieValue = currentPinkie;
            }
        }
        if (CALIBRATION_METHOD == 0)
        {
            minThumbValue /= counter;
            minIndexValue /= counter;
            minMiddleValue /= counter;
            minRingValue /= counter;
            minPinkieValue /= counter;
        }

        // Blink LED to let user know calibration is done
        digitalWrite(LED_BUILTIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    for (;;)
    {

        // Raw adc voltage values // Use smoothed read or raw voltage
        int rawThumb = readSmooth(THUMB_POT);
        int rawIndex = readSmooth(INDEX_POT);
        int rawMiddle = readSmooth(MIDDLE_POT);
        int rawRing = readSmooth(RING_POT);
        int rawPinkie = readSmooth(PINKIE_POT);

        // Now map the values based on calibration
        // Note flip for opengloves (4095->0) and constrain to valid range
        int thumbAngle = constrain(map(rawThumb, minThumbValue, maxThumbValue, 0, 4095), 0, 4095);
        int indexAngle = constrain(map(rawIndex, minIndexValue, maxIndexValue, 0, 4095), 0, 4095);
        int middleAngle = constrain(map(rawMiddle, minMiddleValue, maxMiddleValue, 0, 4095), 0, 4095);
        int ringAngle = constrain(map(rawRing, minRingValue, maxRingValue, 0, 4095), 0, 4095);
        int pinkieAngle = constrain(map(rawPinkie, minPinkieValue, maxPinkieValue, 0, 4095), 0, 4095);

        if (CALIBRATION_METHOD == 0)
        {
            thumbAngle = max(0, 4095 - thumbAngle);
            indexAngle = max(0, 4095 - indexAngle);
            middleAngle = max(0, 4095 - middleAngle);
            ringAngle = max(0, 4095 - ringAngle);
            pinkieAngle = max(0, 4095 - pinkieAngle);
        }
        // Debug print that show's a finger's raw current value and it's minumum and max recorded value during calibration

        // Arduino Serial Plotter format (label:value pairs)
        /*
        Serial.printf("Thumb:%d,Index:%d,Middle:%d,Ring:%d,Pinkie:%d\n",
                      rawThumb, rawIndex, rawMiddle, rawRing, rawPinkie);
        */

        // Read controller button values
        float joystick_x = analogRead(JOYSTICK_X);
        float joystick_y = analogRead(JOYSTICK_Y);
        int joystick_pressed = digitalRead(JOYSTICK_BUTTON);
        int a_button = digitalRead(A_BUTTON);
        int b_button = digitalRead(B_BUTTON);

        uint32_t buttonMask = (joystick_pressed << 2) | (a_button << 1) | (b_button);

        // Calculate trigger button passed of value of bending
        // If index and thumb is more than 50% bent
        if (thumbAngle > 4095 * 0.33 && indexAngle > 4095 * 0.33)
        {
            // Set current 4 bit of bit mask to have trigger button
            buttonMask |= (1 << 3);
        }

        // Send Data to Persistant State
        DataBroker::instance().setFingerAngle(0, (thumbAngle));
        DataBroker::instance().setFingerAngle(1, (indexAngle));
        DataBroker::instance().setFingerAngle(2, (middleAngle));
        DataBroker::instance().setFingerAngle(3, (ringAngle));
        DataBroker::instance().setFingerAngle(4, (pinkieAngle));
        DataBroker::instance().setJoystick(joystick_x, joystick_y);
        DataBroker::instance().setButtonsBitmask(buttonMask);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
