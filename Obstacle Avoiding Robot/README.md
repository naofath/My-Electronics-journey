# 🤖 Autonomous Obstacle Avoiding Robot

An Arduino-based autonomous robot designed to detect and navigate around obstacles in real-time using ultrasonic distance sensing and servo-assisted scanning.

---

## 🚀 Overview
This project demonstrates basic autonomous navigation and embedded systems control. The robot continuously scans its surroundings for obstacles, calculates safe clearance distances, and dynamically adjusts its trajectory to avoid collisions without human intervention.

## 🛠️ Hardware Components
* **Microcontroller:** Arduino Uno R3
* **Distance Sensor:** HC-SR04 Ultrasonic Sensor
* **Motor Driver:** L298N Dual H-Bridge Motor Driver Module
* **Actuators:** 
  * 2x / 4x DC Gear Motors with Wheels
  * 1x SG90 Micro Servo Motor (for sensor scanning)
* **Power Supply:** Rechargeable Li-ion Battery Pack (7.4V – 12V)
* **Miscellaneous:** Robot Chassis, Jumper Wires, On/Off Switch

## ⚙️ Working Principle
1. **Forward Navigation:** The robot moves forward while the HC-SR04 sensor continuously measures the distance to any object directly ahead.
2. **Obstacle Detection:** If an obstacle is detected within a predefined threshold distance (e.g., `< 20 cm`), the robot immediately halts.
3. **Environment Scanning:** The SG90 servo motor rotates the ultrasonic sensor to scan both the left and right directions, recording clearance distances.
4. **Decision & Turn:** Comparing the left and right distances, the microcontroller commands the L298N driver to turn the robot toward the path with greater clearance and resumes forward motion.

## 🔌 Pin Configuration

| Component | Arduino Pin | Description |
| :--- | :--- | :--- |
| **HC-SR04 Trig** | `D9` | Trigger pulse output |
| **HC-SR04 Echo** | `D10` | Echo pulse input |
| **Servo Signal** | `D3` (PWM) | Controls sensor angle |
| **L298N IN1 & IN2** | `D4`, `D5` | Left motor direction |
| **L298N IN3 & IN4** | `D6`, `D7` | Right motor direction |
| **L298N ENA & ENB** | `D11`, `D12` | Motor speed control (PWM) |

*(Note: Update pin numbers above if your wiring differs.)*

## 💻 Software & Tools
* **IDE:** Arduino IDE
* **Language:** Embedded C / C++
* **Libraries Used:** `Servo.h`, `NewPing.h` *(if applicable)*

## 🎥 Demo
* **Video Demonstration:** [Watch Demo Video Here](#) *(Replace `#` with your Facebook/YouTube/Drive video link)*

## 🔮 Future Improvements
* Implement path-memory mapping to optimize routes and avoid dead ends.
* Add Bluetooth/Wi-Fi telemetry for manual override and live distance monitoring.
* Upgrade to PID speed control for smoother turns and better traction.

---

### 👨‍💻 Author
**Naofath Alam Borno**  
Department of Electronics & Telecommunication Engineering (ETE), RUET  
* [GitHub Profile](https://github.com/your-username)

