#include "AnalogRead_task.h"

// Description: Averages value read from an analog pin based off sample rate in config.h
// Parameters: rawanalog pin
// Return: sum value given sample rate
// Note: This will be deprecated if we move to ExpressIf Ide as we can do this via hardware
int readSmooth(int pin)
{
    long long sum = 0;
    for (int i = 0; i < FLEX_SENSOR_SAMPLE_RATE; i++)
    {
        sum += abs(analogRead(pin));
    }
    return sum / FLEX_SENSOR_SAMPLE_RATE;
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
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);

    /* Finger Calibration value, max flex
    int thumbClosed  = mapFlex(readSmooth(thumbPin));
    int indexClosed  = mapFlex(readSmooth(indexPin));
    int middleClosed = mapFlex(readSmooth(middlePin));
    int ringClosed   = mapFlex(readSmooth(ringPin));
    int pinkieClosed = mapFlex(readSmooth(pinkiePin));
    */
    // Fetch analog data from sensors forever
    for (;;)
    {

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

        vTaskDelay(pdMS_TO_TICKS(10));
        ;
    }
}