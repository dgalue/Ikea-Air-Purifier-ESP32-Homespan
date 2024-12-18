#include "HomeSpan.h"

class AirPurifier : public Service::AirPurifier {
  private:
    int mosfetPin;
    int pulsePin;  // Pin for generating pulses
    int ledPin;
    int filterCounterPin;

    SpanCharacteristic *active;
    SpanCharacteristic *currentState;
    SpanCharacteristic *targetState;
    SpanCharacteristic *rotationSpeed;

    unsigned long lastBlinkTime = 0;
    unsigned long lastRPMCheck = 0;
    static volatile int pulseCount; // Count of pulses for RPM calculation
    bool ledState = false;

    const int PULSES_PER_REV = 2; // Assuming 2 pulses per revolution
    const unsigned long RPM_INTERVAL = 2000; // 2 seconds
    const unsigned long FILTER_LIMIT = 3153600; // 6 months in seconds
    float filterAge = 0.0;

  public:
    AirPurifier(int mosfetPin, int pulsePin, int ledPin, int filterCounterPin) : Service::AirPurifier() {
        this->mosfetPin = mosfetPin;
        this->pulsePin = pulsePin;
        this->ledPin = ledPin;
        this->filterCounterPin = filterCounterPin;

        pinMode(mosfetPin, OUTPUT);
        pinMode(pulsePin, OUTPUT);
        pinMode(ledPin, OUTPUT);
        pinMode(filterCounterPin, INPUT_PULLUP); // Use pull-up resistor if required

        attachInterrupt(digitalPinToInterrupt(filterCounterPin), countPulse, RISING); // Interrupt for RPM counting

        digitalWrite(mosfetPin, LOW);
        digitalWrite(pulsePin, LOW);
        digitalWrite(ledPin, LOW);

        active = new Characteristic::Active(0); // Default to Inactive
        currentState = new Characteristic::CurrentAirPurifierState(0); // Default to Inactive
        targetState = new Characteristic::TargetAirPurifierState(1); // Default to Auto
        rotationSpeed = new Characteristic::RotationSpeed(0); // Default Speed 0
    }

    static void countPulse() {
        pulseCount++; // Increment pulse count
    }

    boolean update() {
        if (active->updated()) {
            if (active->getNewVal()) {
                digitalWrite(mosfetPin, HIGH); // Turn on purifier
                currentState->setVal(2); // Purifying
                // Ensure fan starts at the current slider speed
                int speed = rotationSpeed->getVal(); // Get current slider value
                int pulseFrequency = map(speed, 0, 100, 100, 300); // Map speed to pulse frequency
                setFanSpeed(pulseFrequency); // Start fan
            } else {
                digitalWrite(mosfetPin, LOW); // Turn off purifier
                currentState->setVal(0); // Inactive
                stopFan(); // Stop fan by halting pulse generation
            }
        }

        if (rotationSpeed->updated() && active->getVal()) {
            int speed = rotationSpeed->getNewVal();
            int pulseFrequency = map(speed, 0, 100, 100, 300); // Map speed to pulse frequency
            setFanSpeed(pulseFrequency); // Adjust fan speed by setting pulse frequency
        }

        return true; // Indicate update success
    }

    void loop() {
        // Handle LED for purifier status
        if (active->getVal()) {
            digitalWrite(ledPin, HIGH); // Turn LED on when purifier is active
        } else {
            digitalWrite(ledPin, LOW); // Turn LED off when purifier is inactive
        }

        // Handle blinking for filter warning
        if (filterAge >= FILTER_LIMIT) {
            unsigned long currentTime = millis();
            if (currentTime - lastBlinkTime >= 500) { // Blink every 500ms
                ledState = !ledState; // Toggle LED state
                digitalWrite(ledPin, ledState ? HIGH : LOW);
                lastBlinkTime = currentTime;
            }
        }

        // Calculate RPM and update filter age every 2 seconds
        unsigned long currentTime = millis();
        if (currentTime - lastRPMCheck >= RPM_INTERVAL) {
            int rpm = (pulseCount / PULSES_PER_REV) * (60000 / RPM_INTERVAL); // Calculate RPM
            pulseCount = 0; // Reset pulse count
            lastRPMCheck = currentTime;

            // Update filter age based on RPM
            filterAge += rpm * 0.0001; // Scaling factor from YAML
        }
    }

    void setFanSpeed(int frequency) {
        const int dutyCycle = 50; // 50% duty cycle
        int pulseInterval = 1000000 / frequency; // Calculate pulse interval in microseconds

        analogWriteFrequency(pulsePin, frequency); // Set the PWM frequency
        analogWrite(pulsePin, 128); // Set duty cycle to 50% (128 out of 255)
    }

    void stopFan() {
        analogWrite(pulsePin, 0); // Stop pulses by setting duty cycle to 0
    }
};

volatile int AirPurifier::pulseCount = 0; // Define static member

void initHomeSpan() {
    homeSpan.setControlPin(0);
    homeSpan.setStatusPin(2); // Use a separate pin for status LED
    homeSpan.setStatusAutoOff(600);
    homeSpan.setApTimeout(600);
    homeSpan.enableAutoStartAP();
    homeSpan.setApSSID("Air_Purifier_Setup");
    homeSpan.setApPassword("airpurifier123");
    homeSpan.enableOTA("airpurifierOTA123");
    homeSpan.setPairingCode("12345678");
    homeSpan.setQRID("AB12");
    homeSpan.setLogLevel(2);
    homeSpan.setSketchVersion("1.0");
    homeSpan.enableWebLog(50, "pool.ntp.org", "UTC+0", "logs");
    homeSpan.setHostNameSuffix("");
    homeSpan.begin(Category::AirPurifiers, "Air Purifier", "airpurifier");
}

void setup() {
    Serial.begin(115200);

    initHomeSpan(); // Initialize HomeSpan with Air Purifier-specific settings

    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();

    new AirPurifier(16, 17, 21, 22); // Initialize with pins for MOSFET, Pulse, LED, and Filter Counter
}

void loop() {
    homeSpan.poll();
}
