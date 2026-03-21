# 🐟 AquaFeed - Automatic Fish Feeder

An ESP32-C3 based automatic aquarium feeder with a built-in web interface for control and monitoring.

## ✨ Features

- 🤖 Scheduled automatic feeding
- 📱 Web interface for device control
- 🔋 Battery monitoring
- ⚡ Power saving mode
- 🔘 Manual feeding with a button
- 📊 Status and runtime monitoring

## 🔧 Hardware

- ESP32-C3 Super Mini
- Servo motor (for example SG90)
- MH Electronic battery voltage sensor
- Button for manual feeding

## 📦 Project Structure

```
fish_eat/
├── src/                    # ESP32-C3 firmware source
│   ├── main.cpp           # Main entry point
│   ├── api_handlers.*     # API endpoints
│   ├── servo_controller.* # Servo control
│   ├── battery_monitor.*  # Battery monitoring
│   ├── feeding_scheduler.*# Feeding schedule logic
│   └── wifi_manager.*     # WiFi management
├── platformio.ini         # PlatformIO configuration
└── README.md              # This file
```

## 🚀 Quick Start

### 1. Installation

1. Install [PlatformIO](https://platformio.org/).
2. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/fish_eat.git
   cd fish_eat
   ```

### 2. Build and Upload

1. Connect ESP32-C3 to your computer.
2. Open the project in PlatformIO.
3. Upload the firmware:
   ```
   PlatformIO: Upload
   ```

### 3. WiFi Setup

1. On first boot, ESP32 starts the `AquaFeed-Setup` access point.
2. Connect to it (password: `12345678`).
3. Open `192.168.4.1` in your browser.
4. Select your WiFi network and enter the password.

### 4. Access the Interface

After connecting to WiFi, open:
- `http://fish-eat.local` (via mDNS)
- or `http://192.168.1.XXX` (check the exact IP in Serial Monitor)

## 🌐 API Endpoints

### GET `/api/status`
Get current device status:
```json
{
  "angle": 90,
  "speed": 20,
  "feedRepeats": 1,
  "powerSaveMode": true,
  "batteryVoltage": 3.7,
  "batteryPercent": 75,
  "feedTimes": ["08:00", "20:00"]
}
```

### POST `/api/feed`
Manual feed:
```
POST /api/feed
Body: {"repeats": 1}
```

### POST `/api/speed`
Set servo speed:
```
POST /api/speed
Body: {"speed": 20}
```

### POST `/api/schedule`
Set feeding schedule:
```
POST /api/schedule
Body: {"times": ["08:00", "20:00"]}
```

## 🔄 Development Path

### Option 1: All on ESP32 (current version)
- Firmware and API run on the microcontroller.
- Web interface is embedded into firmware.

### Option 2: Cloud frontend (recommended for scaling)
- Backend (API) stays on ESP32.
- Frontend is deployed to GitHub Pages / Netlify / Vercel.

## 📝 License

MIT License

## 🤝 Contributing

Pull requests and issues are welcome.

## 📧 Contacts

Built with ❤️ for your fish.

