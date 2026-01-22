#include "AnalogRead_task.h"

// Description: Averages value read from an analog pin based off sample rate in config.h
// Parameters: rawanalog pin
// Return: sum value given sample rate
// Note: This will be deprecated if we move to ExpressIf Ide as we can do this via hardware
int readSmooth(int pin)
{
    //--Basic Average
    /*
    long long sum = 0;
    for (int i = 0; i < FLEX_SENSOR_SAMPLE_RATE; i++)
    {
        sum += analogReadMilliVolts(pin);
    }
    return sum / FLEX_SENSOR_SAMPLE_RATE;
    */

    //--Median filter

    // Store all readings in dynamic array and sort then take median value to avoid outliers
    std::vector<int> readings;
    readings.reserve(FLEX_SENSOR_SAMPLE_RATE);
    for (int i = 0; i < FLEX_SENSOR_SAMPLE_RATE; i++)
    {
        readings.push_back(analogReadMilliVolts(pin));
    }

    std::sort(readings.begin(), readings.end());

    // If even
    if (readings.size() % 2 == 0)
    {
        return (readings[readings.size() / 2 - 1] + readings[readings.size() / 2]) / 2;
    }
    // if odd
    else
    {
        return readings[readings.size() / 2];
    }

    /*
    // --Trimmed mean filter
    std::vector<int> readings;
    readings.reserve(FLEX_SENSOR_SAMPLE_RATE);
    for (int i = 0; i < FLEX_SENSOR_SAMPLE_RATE; i++)
    {
        readings.push_back(analogReadMilliVolts(pin));
    }
    std::sort(readings.begin(), readings.end());

    // Remove top and bottom 25% of readings to avoid outliers
    long long sum = 0;
    int trimIndexStart = FLEX_SENSOR_SAMPLE_RATE / 4;
    int trimIndexEnd = FLEX_SENSOR_SAMPLE_RATE - trimIndexStart;
    for (int i = trimIndexStart; i < trimIndexEnd; i++)
    {
        sum += readings[i];
    }
    return sum / (trimIndexEnd - trimIndexStart);
    */
}

// Description: Converts adc range of 400->1250 to 0->4250
// Parameters: rawanalog Value
// Return: value that matches range
int mapFlex(int raw)
{
    // Rought estimates from circuit
    const int RAW_MIN = 400;
    const int RAW_MAX = 1250;

    // Clamp range to range expected for opengloves
    if (raw < RAW_MIN)
        raw = RAW_MIN;
    if (raw > RAW_MAX)
        raw = RAW_MAX;

    // Map to 0–>4095
    return (raw - RAW_MIN) * 4095L / (RAW_MAX - RAW_MIN);
}

// Description: Reads all Analog Data from sensors
// Parameters: pvParameters which is a place holder for any pointer to any type
// Return: none, it will simply pass the information on to the next core for processing
void TaskAnalogRead(void *pvParameters)
{

    // Pin locations for fingers
    const int thumbPin = 13;
    const int indexPin = 12;
    const int middlePin = 11;
    const int ringPin = 10;
    const int pinkiePin = 9;

    // Controller button pins
    const int joystick_button_pin = 4;
    const int joystick_x_pin = 5;
    const int joystick_y_pin = 6;
    const int a_button_pin = 7;
    const int b_button_pin = 15;

    pinMode(joystick_button_pin, INPUT_PULLUP);
    pinMode(a_button_pin, INPUT);
    pinMode(b_button_pin, INPUT);

    // To not get compiler unused variable error
    (void)pvParameters;

    // Get range from 0.0V to 3.3V
    analogSetAttenuation(ADC_2_5db);
    analogReadResolution(12);

    /* Finger Calibration value, max flex
    int thumbClosed  = mapFlex(readSmooth(thumbPin));
    int indexClosed  = mapFlex(readSmooth(indexPin));
    int middleClosed = mapFlex(readSmooth(middlePin));
    int ringClosed   = mapFlex(readSmooth(ringPin));
    int pinkieClosed = mapFlex(readSmooth(pinkiePin));
    */
    // Fetch analog data from sensors forever

    /*
    // OpenGloves just wants raw adc val, however we will sample each pin 500 times and average the values, and subtract it by the calibration value, and if somehow negative,
    // due to noise, we will simply round it up to 0
    int rawThumb = max(0, (readSmooth(thumbPin)));
    int rawIndex = max(0, (readSmooth(indexPin)));
    int rawMiddle = max(0, (readSmooth(middlePin)));
    int rawRing = max(0, (readSmooth(ringPin)));
    int rawPinkie = max(0, (readSmooth(pinkiePin)));


    // Convert angles to map to desired range for Opengloves(4095->0)
    int thumbAngle = max(0, (4095 - mapFlex(rawThumb)));
    int indexAngle = max(0, (4095 - mapFlex(rawIndex)));
    int middleAngle = max(0, (4095 - mapFlex(rawMiddle)));
    int ringAngle = max(0, (4095 - mapFlex(rawRing)));
    int pinkieAngle = max(0, (4095 - mapFlex(rawPinkie)));
    */

    // Calibration phase - sum values then average
    long long maxThumbValue = 0, maxIndexValue = 0, maxMiddleValue = 0, maxRingValue = 0, maxPinkieValue = 0;
    long long minThumbValue = 0, minIndexValue = 0, minMiddleValue = 0, minRingValue = 0, minPinkieValue = 0;
    long long currentThumb = 0, currentIndex = 0, currentMiddle = 0, currentRing = 0, currentPinkie = 0;

    // For first 5 seconds, assume hand is flexed(open as hard as possible) and record that as max flex
    // Get current time
    unsigned long long startTime = millis();
    long long counter = 0;

    // Check if 5 seconds has passed
    while (millis() - startTime < 5000)
    {
        // Read current values
        currentThumb = analogReadMilliVolts(thumbPin);
        currentIndex = analogReadMilliVolts(indexPin);
        currentMiddle = analogReadMilliVolts(middlePin);
        currentRing = analogReadMilliVolts(ringPin);
        currentPinkie = analogReadMilliVolts(pinkiePin);

        counter++;

        maxThumbValue += currentThumb;
        maxIndexValue += currentIndex;
        maxMiddleValue += currentMiddle;
        maxRingValue += currentRing;
        maxPinkieValue += currentPinkie;
    }
    maxThumbValue /= counter;
    maxIndexValue /= counter;
    maxMiddleValue /= counter;
    maxRingValue /= counter;
    maxPinkieValue /= counter;

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
        currentThumb = analogReadMilliVolts(thumbPin);
        currentIndex = analogReadMilliVolts(indexPin);
        currentMiddle = analogReadMilliVolts(middlePin);
        currentRing = analogReadMilliVolts(ringPin);
        currentPinkie = analogReadMilliVolts(pinkiePin);

        counter++;

        minThumbValue += currentThumb;
        minIndexValue += currentIndex;
        minMiddleValue += currentMiddle;
        minRingValue += currentRing;
        minPinkieValue += currentPinkie;
    }

    minThumbValue /= counter;
    minIndexValue /= counter;
    minMiddleValue /= counter;
    minRingValue /= counter;
    minPinkieValue /= counter;

    // Blink LED to let user know calibration is done
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));

    for (;;)
    {

        // Raw adc voltage values
        int rawThumb = readSmooth(thumbPin);
        int rawIndex = readSmooth(indexPin);
        int rawMiddle = readSmooth(middlePin);
        int rawRing = readSmooth(ringPin);
        int rawPinkie = readSmooth(pinkiePin);

        // Now map the values based on calibration
        // Note flip for opengloves (4095->0) and constrain to valid range
        int thumbAngle = constrain(4095 - static_cast<int>(map(rawThumb, minThumbValue, maxThumbValue, 0, 4095)), 0, 4095);
        int indexAngle = constrain(4095 - static_cast<int>(map(rawIndex, minIndexValue, maxIndexValue, 0, 4095)), 0, 4095);
        int middleAngle = constrain(4095 - static_cast<int>(map(rawMiddle, minMiddleValue, maxMiddleValue, 0, 4095)), 0, 4095);
        int ringAngle = constrain(4095 - static_cast<int>(map(rawRing, minRingValue, maxRingValue, 0, 4095)), 0, 4095);
        int pinkieAngle = constrain(4095 - static_cast<int>(map(rawPinkie, minPinkieValue, maxPinkieValue, 0, 4095)), 0, 4095);

        // Debug print that show's a finger's raw current value and it's minumum and max recorded value during calibration
        /*
        Serial.printf("Thumb Raw: %d Min: %lld Max: %lld Mapped Angle: %d\n", rawThumb, minThumbValue, maxThumbValue, thumbAngle);
        Serial.printf("Index Raw: %d Min: %lld Max: %lld Mapped Angle: %d\n", rawIndex, minIndexValue, maxIndexValue, indexAngle);
        Serial.printf("Middle Raw: %d Min: %lld Max: %lld Mapped Angle: %d\n", rawMiddle, minMiddleValue, maxMiddleValue, middleAngle);
        Serial.printf("Ring Raw: %d Min: %lld Max: %lld Mapped Angle: %d\n", rawRing, minRingValue, maxRingValue, ringAngle);
        Serial.printf("Pinkie Raw: %d Min: %lld Max: %lld Mapped Angle: %d\n", rawPinkie, minPinkieValue, maxPinkieValue, pinkieAngle);
        */
        // Read controller button values
        float joystick_x = map(analogRead(joystick_x_pin), 0, 4095, 0, 1024);
        float joystick_y = map(analogRead(joystick_y_pin), 0, 4095, 0, 1024);
        int joystick_pressed = digitalRead(joystick_button_pin);
        int a_button = digitalRead(a_button_pin);
        int b_button = digitalRead(b_button_pin);

        uint32_t buttonMask = (joystick_pressed << 2) | (a_button << 1) | (b_button);

        // Calculate trigger button passed of value of bending
        if (thumbAngle > 150 && indexAngle > 150)
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

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}