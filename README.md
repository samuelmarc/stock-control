# Stock Control System

An ESP32-based IoT system for automated warehouse stock tracking using RFID tags and ultrasonic sensors. This system detects items at a storage location, reads their RFID tags, and communicates with a backend API to register stock input/output events.

## Features

- **RFID Tag Detection**: Reads RFID tags from items using an MFRC522 reader
- **Ultrasonic Proximity Sensing**: Detects when an item is placed near the sensor
- **WiFi Connectivity**: Communicates with a remote API over WiFi
- **Real-time Feedback**: 
  - LCD display showing system status and messages
  - Visual feedback with green/red LEDs
  - Audio feedback with buzzer
- **Automatic Item Classification**: Distinguishes between new items (input) and existing items (output)
- **Serial Logging**: Comprehensive debug logging via USB serial connection

## System Requirements

### Hardware
- **ESP32 Development Board** (or compatible ESP32 microcontroller)
- **MFRC522 RFID Reader Module**
- **HC-SR04 Ultrasonic Distance Sensor**
- **16x2 I2C LCD Display**
- **Green LED** (status indicator)
- **Red LED** (error indicator)
- **Buzzer** (audio feedback)
- **SPI connection cables** and **I2C connection cables**

### Software
- **PlatformIO**: Platform for embedded systems development
- **VS Code** (with PlatformIO extension)

## Getting Started

### 1. Install PlatformIO

If you haven't already, install PlatformIO IDE for VS Code:
1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install

### 2. Clone/Open the Project

Open the project folder in VS Code. PlatformIO will automatically detect and initialize the project.

### 3. Configure Hardware Credentials

Create or update `include/secrets.h` based on `include/secrets.h.example`:

```cpp
#define WIFI_SSID "MyNetworkSSID"
#define WIFI_PASS "MyNetworkPassword1234"
#define API_BASE "http://HOST:PORT/api/v1/warehouse"
```

### 4. Configure Hardware Pin Mappings

In `include/secrets.h`, adjust the GPIO pin definitions if your hardware setup differs:

| Constant | Default Pin | Purpose |
|----------|-------------|---------|
| `SS_PIN` | 21 | SPI Chip Select (MFRC522) |
| `RST_PIN` | 22 | RFID Reader Reset |
| `TRIG_PIN` | 32 | Ultrasonic Sensor Trigger |
| `ECHO_PIN` | 34 | Ultrasonic Sensor Echo |
| `DISTANCE_THRESHOLD` | 5 | Distance in cm to trigger detection |
| `GREEN_LED_PIN` | 2 | Success indicator LED |
| `RED_LED_PIN` | 4 | Error indicator LED |
| `BUZZER_PIN` | 5 | Audio feedback buzzer |
| `I2C_SDA_PIN` | 25 | I2C Data line (LCD) |
| `I2C_SCL_PIN` | 26 | I2C Clock line (LCD) |
| `LCD_ADDRESS` | 0x27 | I2C address of LCD (check with I2C scanner if unknown) |
| `LCD_COLS` | 16 | LCD column count |
| `LCD_ROWS` | 2 | LCD row count |

### 5. Hardware Wiring Summary

**RFID Reader (MFRC522) - SPI Connection:**
- VCC → 3.3V
- GND → GND
- SCK → GPIO 18
- MOSI → GPIO 23
- MISO → GPIO 19
- SS → GPIO 21
- RST → GPIO 22

**Ultrasonic Sensor (HC-SR04):**
- VCC → 5V
- GND → GND
- TRIG → GPIO 32
- ECHO → GPIO 34

**LCD Display (16x2 I2C):**
- VCC → 5V
- GND → GND
- SDA → GPIO 25
- SCL → GPIO 26

**LEDs and Buzzer:**
- Green LED (with 220Ω resistor) → GPIO 2
- Red LED (with 220Ω resistor) → GPIO 4
- Buzzer → GPIO 5 (and GND)

### 6. Build and Upload

In VS Code with PlatformIO:

1. **Build the project:**
   ```
   Press Ctrl+Alt+B or use PlatformIO: Build
   ```

2. **Upload to ESP32:**
   - Connect ESP32 to your computer via USB
   - Press Ctrl+Alt+U or use PlatformIO: Upload

3. **Monitor Serial Output:**
   - Press Ctrl+Alt+S or use PlatformIO: Monitor to see debug logs

## API Integration

The system communicates with a backend API at the endpoint defined in `API_BASE`. The API must support the following endpoints:

### Check if Item Exists
```
GET /api/v1/warehouse/{tag}
```
- **Returns 200**: Item exists (will register as output)
- **Returns 404 or other**: Item doesn't exist (will register as input)

### Register Input (New Item)
```
POST /api/v1/warehouse/box/input/{tag}
```
- **Expected response code**: 201 (Created)

### Register Output (Existing Item)
```
POST /api/v1/warehouse/box/output/{tag}
```
- **Expected response code**: 200 (OK)

## Operation

1. **Power on** the ESP32 - system boots and connects to WiFi
2. **Place an item** near the ultrasonic sensor (within the distance threshold)
3. **Hold the RFID tag** near the reader
4. **System reads the tag** and sends it to the API
5. **Automatic classification**:
   - If tag exists: registered as **output** (item leaving)
   - If tag is new: registered as **input** (item entering)
6. **Feedback**:
   - ✓ Green LED + buzzer = Success
   - ✗ Red LED + multiple buzzes = Error (see LCD for details)
7. LCD displays status messages throughout the process

## Troubleshooting

| Issue | Solution |
|-------|----------|
| **Can't connect to WiFi** | Check SSID and password in `secrets.h` |
| **RFID reader not detected** | Verify SPI pins and power supply, try I2C scanner for RFID |
| **LCD shows nothing** | Check I2C address (default 0x27), use I2C scanner tool |
| **Ultrasonic not detecting** | Verify GPIO pins, check sensor power and wiring |
| **API requests failing** | Verify API_BASE URL, check network connectivity, review serial logs |
| **Can't upload firmware** | Ensure ESP32 is recognized via USB, try pressing BOOT button while uploading |

## Serial Debug Output

Connect to the serial monitor (115200 baud) to view:
- System startup sequence
- WiFi connection status
- RFID tag reads
- API requests and responses
- Error messages

## Project Structure

```
stock-control/
├── platformio.ini          # PlatformIO configuration
├── include/
│   ├── secrets.h.example   # Configuration template
│   ├── secrets.h           # Your actual configuration (git-ignored)
│   └── README
├── src/
│   └── main.cpp            # Main firmware code
├── lib/
│   └── README
├── test/
│   └── README
└── README.md               # This file
```

## Dependencies

- **MFRC522** (v1.4.12+): RFID reader library
- **LiquidCrystal_I2C** (v1.1.4+): I2C LCD display library
- ESP32 Arduino Framework (built-in)

These are automatically managed by PlatformIO as specified in `platformio.ini`.

## License

This project is part of a stock control warehouse system.

---

For questions or issues, check the serial monitor output and review the configuration values in `include/secrets.h`.
