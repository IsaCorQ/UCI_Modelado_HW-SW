# ESP8266-Arduino Temperature Motor Controller - Setup Guide

## Overview
This system allows you to control a motor based on temperature thresholds using:
- **Arduino** - reads DHT22 sensor & soil moisture sensor
- **ESP8266** (WiFi microcontroller) - acts as wireless bridge
- **DHT22** (temperature/humidity sensor) - connected to Arduino
- **Soil Moisture Sensor** - connected to Arduino
- **Motor** (DC motor) - controlled via relay connected to Arduino
- **Python GUI App** - remote control via WiFi through ESP8266

**Data Flow:** Sensors → Arduino → ESP8266 (via Serial) → Python App (via WiFi)

---

## Hardware Requirements

### Components Needed:
1. **Arduino Uno/Nano** - ~$5-10
2. **ESP8266 development board** (NodeMCU v2 or Wemos D1 Mini) - ~$5
3. **DHT22 temperature/humidity sensor** - ~$3
4. **Soil Moisture Sensor** (analog) - ~$2
5. **5V relay module** (for motor control) - ~$2
6. **DC motor** - ~$5
7. **Power supply** (5V for Arduino, ESP8266, and motor)
8. **Resistors**: 10kΩ (for DHT22 pull-up)
9. **Capacitor**: 100µF (power supply smoothing)
10. **Breadboard and jumper wires**
11. **USB cables** for programming Arduino and ESP8266

---

## Hardware Connection Diagram

### Arduino to Sensors:
```
Arduino          DHT22 Sensor
Pin 2       -->  Data pin
5V          -->  VCC (+)
GND         -->  GND (-)
(add 10kΩ resistor between Data pin and 5V)

Arduino          Soil Moisture Sensor
A0          -->  Analog output
5V          -->  VCC
GND         -->  GND
```

### Arduino to Motor Relay:
```
Arduino          5V Relay Module      Motor (DC)
Pin 5       -->  Signal (IN)
5V          -->  VCC
GND         -->  GND
                 COM         -->  Motor positive (+)
                 NO (normally open)
                 GND/OUT     -->  Motor negative (-)
```

### Arduino to ESP8266 (Serial Communication):
```
Arduino TX (Pin 1)  -->  ESP8266 RX (GPIO3)
Arduino RX (Pin 0)  -->  ESP8266 TX (GPIO1)
Arduino GND         -->  ESP8266 GND
Arduino 5V          -->  ESP8266 Vin (or use voltage divider for safety)
```

### Power Supply:
```
5V Power Supply
  ├─ Arduino Vin
  ├─ ESP8266 Vin (via voltage regulator or direct if 5V tolerant)
  ├─ Relay Module VCC
  ├─ DHT22 VCC (through resistor)
  ├─ Soil Moisture VCC
  └─ Motor GND connection
```

### Complete Connection Table:

| From | To | Connection | Notes |
|------|----|-----------|----|
| Arduino Pin 2 | DHT22 Data | Yellow | Add 10kΩ pull-up resistor to 5V |
| Arduino 5V | DHT22 VCC | Red | |
| Arduino GND | DHT22 GND | Black | |
| Arduino A0 | Soil Moisture Output | Blue | Analog input |
| Arduino 5V | Soil Moisture VCC | Red | |
| Arduino GND | Soil Moisture GND | Black | |
| Arduino Pin 5 | Relay Signal (IN) | Green | PWM-capable pin |
| Arduino 5V | Relay VCC | Red | |
| Arduino GND | Relay GND | Black | |
| Relay COM | Motor + | Orange | |
| Relay GND/OUT | Motor - | Brown | |
| Arduino TX (Pin 1) | ESP8266 RX | Purple | 5V to 3.3V (voltage divider recommended) |
| Arduino RX (Pin 0) | ESP8266 TX | Gray | 3.3V to 5V (tolerant on most Arduino boards) |
| Arduino GND | ESP8266 GND | Black | **CRITICAL: Share ground** |

---

## Software Setup

### Step 1: Install Arduino IDE and Add Board Support

1. **Install Arduino IDE** (if not already installed)
   - Download from: https://www.arduino.cc/en/software

2. **Install Required Libraries for Both Boards**
   - Open Arduino IDE → Sketch → Include Library → Manage Libraries
   - Search and install:
     - `DHT sensor library` by Adafruit
     - `Adafruit Unified Sensor` by Adafruit
     - `ArduinoJson` by Benoit Blanchon

3. **Add ESP8266 Board Support** (for ESP8266 code)
   - Arduino IDE → File → Preferences
   - Add URL to "Additional Boards Manager URLs":
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search for "ESP8266"
   - Install "esp8266 by ESP8266 Community"

### Step 2: Upload Arduino Sensor Reader Code

1. **Connect Arduino to your computer via USB**

2. **Open** `arduino_sensor_reader.ino` in Arduino IDE

3. **Select Board and Port**
   - Tools → Board → Arduino → Arduino Uno (or your Arduino model)
   - Tools → Port → Select your COM port

4. **Upload the sketch**
   - Click Upload button (→ icon)
   - Wait for "Done uploading" message

5. **Open Serial Monitor** (Tools → Serial Monitor, 9600 baud)
   - You should see:
     ```
     ===== ARDUINO SENSOR READER =====
     Initializing sensors...
     Ready to communicate with ESP8266
     Sending sensor data every 2 seconds...
     [ARDUINO] Temp: 25.5°C | Humidity: 60% | Soil: 400 | Motor: OFF
     ```
   - Arduino is now ready and waiting for ESP8266

### Step 3: Configure and Upload ESP8266 Code

1. **Disconnect Arduino from USB** (so it's not sending data during ESP8266 programming)

2. **Connect ESP8266 to your computer via USB**

3. **Open4: Setup Python Application

1. **Install Python** (if not already installed)
   - Download from: https://www.python.org/

2. **Install Required Python Packages**
   ```bash
   pip install requests
   ```

3. **Update Python App** - Edit `temperature_controller.py`
   - Line 19: Replace `192.168.1.100` with your ESP8266's IP address
   ```python
   self.esp_ip = "192.168.1.100"  # Change to your ESP8266 IP (from step 3)
   ```

4. **Reconnect Arduino via USB** (after ESP8266 is programmed and running)
   - Arduino and ESP8266 now communicate via Serial connection
   - Arduino sends sensor data, ESP8266 forwards it to Python

5. **Open Serial Monitor** (Tools → Serial Monitor, 9600 baud - **IMPORTANT: must match Arduino**)
   - You should see:
     ```
     ===== ESP8266 ARDUINO BRIDGE =====
     ESP8266 starting as Arduino WiFi bridge...
     Connecting to WiFi: YOUR_WIFI_SSID
     WiFi connected!
     IP Address: 192.168.1.XXX
     Web server started on port 80
     Setup complete! Waiting for Arduino data...
     ```
   - **Note the IP Address** (e.g., 192.168.1.100) - you'll need it for the Python app

### Step 3: Setup Python Application

1. **Install Python** (if not already installed)
   - Download from: https://www.python.org/

2. **Install Required Python Packages**
   ```bash
   pip install requests
   ```

3. **Update Python App** - Edit `temperature_controller.py`
   - Line 19: Replace `192.168.1.100` with your ESP8266's IP address
   ```python
   self.esp_ip = "192.168.1.100"  # Change to your ESP8266 IP
   ```

4. **Run the Python App**
   ```bash
   python temperature_controller.py
   ```

---Step 1: Physical Setup
1. **Wire all components** according to the connection diagram above
2. **Connect Arduino via USB** to your computer
3. **Connect ESP8266 via USB** to your computer (or same power supply)
4. **Ensure Arduino and ESP8266 serial pins are connected** (TX-RX)

### Step 2: Upload Code
1. **Upload Arduino code** first (`arduino_sensor_reader.ino`)
2. **Upload ESP8266 code** second (`esp8266_motor_controller_arduino_bridge.ino`)
3. **Verify both Serial Monitors** show proper startup messages

### Step 3: Python GUI Application

1. **Start the app**: `python temperature_controller.py`

2. **Connection Setup**:
   - The app displays the ESP8266 IP address field
   - Update with your ESP8266's IP (from step 3 above) and click "Update IP"
   - Status should show "✅ Connected" in green

3. **Monitor Sensors**:
   - Current temperature and humidity display (from Arduino DHT22)
   - Soil moisture value displays
   - Motor status shows ON/OFF state
   - Auto-refresh enabled by default (updates every 2 seconds)

4. **Set Temperature Threshold**:
   - Use the spinbox to set desired temperature (e.g., 28°C)
   - Click "Set Threshold"
   - Motor will **automatically activate** when Arduino temperature exceeds threshold
   - Motor deactivates when temp drops 2°C below threshold (hysteresis)
   - Arduino controls the actual motor relay
Arduino Not Sending Data:
- Verify Serial Monitor shows startup messages at **9600 baud**
- Check DHT22 sensor wiring (pin 2, 5V, GND)
- Verify 10kΩ pull-up resistor between DHT22 data and 5V
- Try removing and reinserting DHT22 sensor
- Check soil moisture sensor is connected to A0

### Arduino-ESP8266 Serial Communication Not Working:
- **Check baud rates match**: Both must be 9600
- Verify TX-RX connections (Arduino TX→ESP RX, Arduino RX→ESP TX)
- **Share ground**: Connect Arduino GND to ESP8266 GND (critical!)
- Use voltage divider on Arduino TX to ESP8266 RX (5V to 3.3V)
- Check USB cables are fully connected
- Try different USB port

### ESP8266 Not Connecting to WiFi:
- Double-check WiFi SSID and password
- Ensure WiFi is 2.4GHz (ESP8266 doesn't support 5GHz)
- Try moving closer to router
- Check ESP8266 Serial Monitor shows "Connecting to WiFi..."

### Python App Can't Connect to ESP8266:
- Verify ESP8266 IP address (check ESP8266 Serial Monitor)
- Ensure computer and ESP8266 are on same WiFi network
- Test connection: Open browser and go to `http://192.168.1.XXX` (your ESP8266 IP)
- Verify Python app has correct IP in line 19

### DHT22 Sensor Not Reading:
- Check wiring: Pin 2 (data), 5V (power), GND (ground)
- Verify 10kΩ pull-up resistor is connected between data pin and 5V
- TrArduino Pinout Reference (Uno/Nano):
```
Pin 0 = RX (receives from ESP8266 TX)
Pin 1 = TX (transmits to ESP8266 RX) ← Voltage divider recommended
Pin 2 = DHT22 Data pin ← Using this
Pin 5 = Motor Relay Signal (PWM-capable) ← Using this
A0 = Soil Moisture Analog input ← Using this
```

### ESP8266 Pinout Reference:
```
RX (GPIO3)  = Receives from Arduino TX (5V to 3.3V converter needed)
TX (GPIO1)  = Transmits to Arduino RX (3.3V tolerant on most boards)
D0 (GPIO16) = Status LED ← Using this
GND         = SHARED ground with Arduino (CRITICAL!)
```

### Voltage Divider Circuit (Arduino TX to ESP8266 RX):
```
Arduino TX (5V) ---|▯|--- ESP8266 RX (3.3V)
                   R1=1kΩ
                   |
                   |▯|--- GND
                   R2=2kΩ
```
Formula: Output voltage = 5V × (2kΩ/(1kΩ+2kΩ)) ≈ 3.33V

### Communication Protocol:
- **Arduino → ESP8266**: JSON format every 2 seconds
  ```json
  {"temp":25.5,"humidity":60,"soil":400,"motor":"OFF"}
  ```
- **ESP8266 → Arduino**: JSON commands when Python app requests
  ```json
  {"action":"motor_on"}
  ```

### Security Note:
- This setup transmits data on your local WiFi network
- For remote access over internet, consider adding:
  - Basic authentication
  - HTTPS encryption
  - MQTT broker for secure communication
  - VPN tunnel

### Future Enhancements:
- Add data logging to store temperature history (SD card module)
- Integrate with Home Assistant or other smart home platforms
- Add web dashboard for detailed monitoring
- Implement email/SMS alerts for temperature warnings
- Add multiple sensors support (light, CO2, etc.)
- Implement OTA (Over-The-Air) updates for ESP8266
- Add mobile app instead of desktop Python appol in Python app
- Verify motor power supply is connected
- Check motor is not mechanically jammed

### Serial Monitor Shows Garbage:
- Ensure baud rate is set to **115200**
- Try different USB cable
- Restart Arduino IDE

---

## Additional Notes

### ESP8266 Pinout Reference:
```
D0  = GPIO16 (No PWM)
D1  = GPIO5  (I2C SCL)
D2  = GPIO4  (I2C SDA)
D3  = GPIO0  (Boot selection)
D4  = GPIO2  (DHT Data) ← Using this for DHT22
D5  = GPIO14 (SPI Clock)
D6  = GPIO12 (SPI MOSI)
D7  = GPIO13 (SPI MISO)
D8  = GPIO15 (Motor Control) ← Using this for motor relay
```

### Security Note:
- This setup transmits data on your local WiFi network
- For remote access over internet, consider adding:
  - Basic authentication
  - HTTPS encryption
  - MQTT broker for secure communication

### Future Enhancements:
- Add data logging to store temperature history
- Integrate with Home Assistant or other smart home platforms
- Add web dashboard for more detailed monitoring
- Implement email/SMS alerts for temperature warnings
- Add multiple sensors support

---

## Quick Start Checklist

- [ ] Hardware wired correctly (use diagram above)
- [ ] Arduino IDE installed with ESP8266 board support
- [ ] DHT, Unified Sensor, and ArduinoJson libraries installed
- [ ] WiFi credentials updated in ESP8266 code
- [ ] ESP8266 code uploaded successfully
- [ ] ESP8266 IP address noted from Serial Monitor
- [ ] Python app configured with correct ESP8266 IP
- [ ] Python app runs without connection errors
- [ ] Temperature threshold can be set and adjusted
- [ ] Motor responds to manual control
- [ ] Motor activates automatically when temp exceeds threshold

---

## Support

For questions about:
- **DHT22 sensor**: https://learn.adafruit.com/dht
- **ESP8266 documentation**: https://github.com/esp8266/Arduino
- **ArduinoJson library**: https://arduinojson.org/
- **Python requests**: https://requests.readthedocs.io/
