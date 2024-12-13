# Ikea-Air-Purifier
A HomeKit-enabled Ikea air purifier project using the HomeSpan library and ESP32. This project supports dynamic fan speed control, RPM monitoring for filter usage tracking.

## Features
- **HomeKit Integration**: Control the purifier using Apple HomeKit.
- **Fan Speed Control**: Adjustable speed via a HomeKit slider.
- **Filter Age Tracking**: Monitors filter usage and alerts when replacement is needed.

## Getting Started

### Hardware Requirements
- ESP32 microcontroller
- 3.3 to 5v Level Shifter for CLK and Interruptor
- IKEA Förnuftig air purifier
- Resistor and wiring components

### Wiring Diagram
![Wiring Diagram](Pictures/IMG_9646.JPEG)

### Software Requirements
- Arduino IDE
- [HomeSpan Library](https://github.com/HomeSpan/HomeSpan)
