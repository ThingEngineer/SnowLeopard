ESP32-C3 Pin,Connected To,Peripheral Pin,Notes / Function
5V (or V5),Relay Module,DC+,Power supply for Relay (5V required)
GND,COMMON GROUND,GND / DC- / Common,Connect all Grounds together
GND,Relay Module,DC-,Ground return for Relay power
GND,AHT30 #1 (Internal),GND,Ground return for Sensor
GND,AHT30 #2 (External),GND,Ground return for Sensor
GND,Button (Up),Terminal A,Button connects GPIO to GND when pressed
GND,Button (Down),Terminal A,Button connects GPIO to GND when pressed
GND,Piezo Speaker,-,Piezo negative terminal
3V3 (or V3),AHT30 #1 (Internal),VIN,3.3V Power for Sensor
3V3 (or V3),AHT30 #2 (External),VIN,3.3V Power for Sensor
GPIO 5 (SDA),OLED Display,SDA,I2C bus for the onboard 0.42-inch OLED
GPIO 6 (SCL),OLED Display,SCL,I2C bus for the onboard 0.42-inch OLED
GPIO 5 (SDA),AHT30 #2 (External),SDA,Shared I2C bus with the onboard OLED
GPIO 6 (SCL),AHT30 #2 (External),SCL,Shared I2C bus with the onboard OLED
GPIO 8 (SDA),AHT30 #1 (Internal),SDA,Hardware I2C Bus for the internal sensor
GPIO 9 (SCL),AHT30 #1 (Internal),SCL,Hardware I2C Bus for the internal sensor
GPIO 7,Relay Module,IN (Trigger),Control signal for compressor (Active High)
GPIO 2,Button (Up),Terminal B,Pull-up input (Low when pressed)
GPIO 3,Button (Down),Terminal B,Pull-up input (Low when pressed)
GPIO 4,Piezo Speaker,+,Audible temperature threshold alarm tone output

Notes

- Relay contact wiring default for compressor control is COM + NO (fail-safe OFF if controller power is lost).
- NC is optional and only for use-cases that require the load energized while relay is de-energized.
- The firmware now uses the OLED on GPIO 5/6 together with the external AHT30 on the same bus, and the internal AHT30 stays on GPIO 8/9.
- The settings page contains editable temperature unit, relay mode, set temperature, alarm thresholds, on/off deltas, and minimum off-time lockout.
- See wiring-diagram.mmd for the complete Mermaid wiring map.
