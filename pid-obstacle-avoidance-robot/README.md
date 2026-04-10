#PID Obstacle Avoidance Robot

## Overview

This project consists of the design and implementation of a **mobile robot capable of avoiding obstacles using a PID controller**.
The system integrates hardware design, sensor data processing, and control theory to achieve stable and responsive behavior in real-world conditions.

## Objectives

* Design a mobile robotic platform from scratch
* Implement obstacle detection using ultrasonic sensors
* Develop an empirical model of the system
* Design and tune a **PID controller** for distance regulation
* Validate the system through real-world testing

## System Architecture

The system is composed of:

* **Microcontroller:** Arduino
* **Sensors:** Ultrasonic distance sensor
* **Actuators:** DC motors
* **Control Strategy:** PID controller

The robot continuously measures the distance to obstacles and adjusts its motion to maintain a safe distance.

## Control Strategy

A **PID (Proportional–Integral–Derivative) controller** is used to regulate the distance between the robot and obstacles.

* **Proportional (P):** Reacts to current error
* **Integral (I):** Eliminates steady-state error
* **Derivative (D):** Improves system stability and response

This approach allows smooth and stable obstacle avoidance behavior.

---

## Repository Structure

```
pid-obstacle-avoidance-robot/
├── src/        # Arduino source code
├── docs/       # Project documentation and reports
├── hardware/   # CAD files and mechanical design
└── media/      # Images and demonstration assets
```

##  Getting Started

1. Clone the repository:

```bash
git clone https://github.com/your-username/your-repo.git
```

2. Open the project in Arduino IDE

3. Upload:

* `Carrito_Evasor_PID.ino` (basic version)
* `Carrito_Evasor_PID_con_giro.ino` (enhanced version)


## Demo

```
https://youtu.be/Hhj05II4Fk0

```

---

## Results

* Stable distance regulation achieved
* Improved response time using PID tuning
* Successful obstacle avoidance in dynamic scenarios


