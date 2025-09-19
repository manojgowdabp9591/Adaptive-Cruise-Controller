IoT-Based Adaptive Speed Controller
This project simulates an Adaptive Cruise Control (ACC) system using low-cost hardware and IoT integration. The system automatically adjusts motor speed based on obstacle distance, ensuring safe following distance and real-time monitoring.

🔧 Features
ESP32 microcontroller as central controller
HC-SR04 Ultrasonic Sensor for obstacle detection
L298N Motor Driver + DC Motor for speed control
PWM-based closed-loop control
16x2 LCD Display for live distance & speed data
Blynk IoT App for real-time wireless monitoring
Automatic stop & variable speed control depending on proximity

📊 Results
Stops the motor when objects are ≤15 cm
Runs at controlled speed when objects are moderately close
Runs at full speed when path is clear
Real-time speed and distance monitoring via Blynk App and LCD

✅ Advantages
Enhances safety with automatic speed adjustment
Hands-free operation, reducing driver workload
Cost-effective, scalable, and modular design
Suitable for academic, prototype, and IoT-based automation projects

📂 Tech Stack
Hardware: ESP32, HC-SR04, L298N, DC Motor, LCD
Software: Arduino IDE, Blynk IoT Platform
Languages: Embedded C / C++
