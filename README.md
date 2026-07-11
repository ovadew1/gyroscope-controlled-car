# Gyroscope-Controlled Car

A two-board ESP32 project: tilt a handheld "controller" board and a separate "car" board drives its motors accordingly — wirelessly, via ESP-NOW (no Wi-Fi router needed).

## How it works

- **Controller** — an ESP32 with an MPU6050 accelerometer/gyroscope reads tilt on two axes and converts it into left/right motor speed + direction values.
- **Car** — a second ESP32 wired to a dual H-Bridge motor driver receives those values over ESP-NOW and drives the motors directly.
- If the car stops receiving packets for more than 500ms (controller out of range, powered off, etc.), it automatically stops both motors as a failsafe.
- Tilt forward/back drives both wheels forward/backward together. Tilt left/right spins the wheels in opposite directions (in place turning).

## Hardware requirements

**Controller board:**
- ESP32 dev board (tested against `esp32doit-devkit-v1`)
- MPU6050 accelerometer/gyroscope (I2C)
- Onboard LED used for status blinks (GPIO 2)

**Car board:**
- ESP32 dev board (tested against `esp32doit-devkit-v1`)
- Dual H-Bridge motor driver (e.g. L298N or similar)
- 2x DC motors

## Car wiring

| Signal                    | Pin      |
|----------------------------|----------|
| Motor A – In1               | GPIO 32  |
| Motor A – In2               | GPIO 13  |
| Motor A – Enable (speed)    | GPIO 14  |
| Motor B – In3               | GPIO 26  |
| Motor B – In4               | GPIO 25  |
| Motor B – Enable (speed)    | GPIO 27  |

## Setup

This is a PlatformIO project with two build environments — `controller` and `car` — built from the same repo using source filters, so each board only compiles its own code.

1. Clone the repo:
   ```bash
   git clone https://github.com/ovadew1/Gyroscope-Controlled-Car.git
   cd Gyroscope-Controlled-Car
   ```

2. Flash the car board first so you can read its MAC address:
   ```bash
   pio run -e car --target upload
   pio device monitor
   ```
   Note the printed `ESP32 Car MAC Address`.

3. Open `src/controller/main.cpp` and set `receiverAddress[]` to the car's MAC address you just read.

4. Flash the controller board:
   ```bash
   pio run -e controller --target upload
   ```

5. Power both boards. The controller's LED blinks 5 times once ESP-NOW is initialized and ready. Tilt it to drive the car.

## Dependencies

Declared in `platformio.ini` and fetched automatically by PlatformIO:

- **Adafruit MPU6050** (controller only) — accelerometer/gyroscope driver

## Project structure

```
Gyroscope-Controlled-Car/
├── platformio.ini          # Two environments: controller, car
├── src/
│   ├── controller/
│   │   └── main.cpp        # Reads MPU6050 tilt, sends motor commands via ESP-NOW
│   └── car/
│       └── main.cpp        # Receives commands, drives motors, failsafe stop
└── LICENSE                 # MIT
```

## Notes

- The controller's `receiverAddress` is hardcoded — if you swap in a different car board, you'll need to re-flash the controller with the new MAC.
- Tilt threshold and speed mapping are tuned in `src/controller/main.cpp` (`threshold` and the `map()` call) if you want a more/less sensitive response.

## License

Licensed under MIT.
