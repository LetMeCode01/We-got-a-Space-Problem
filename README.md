# 🚀 We Got a Space Problem – Final Project Documentation


## This github repository contains/ will contain:

1. source code
2. project description
3. setup images
4. demo video

## 📝 General Description

**We Got a Space Problem** is an interactive arcade-style game built on the Arduino platform. The player controls a spaceship navigating through a galaxy displayed on an LED matrix, collecting tokens while avoiding enemies and hazardous events.

### The game features:

- real-time joystick control
- visual feedback via LED matrices and LCD display
- audio feedback using buzzers
- random in-game events (ship stalling, X-ray bombs)
- physical interaction through sensors (LDR)

**For the final project, the game is expanded to support two players, a larger display area, and more complex hardware interactions.**

## 🧾 BOM (Bill of Materials)

### 🧠 Main Components

- Arduino board (UNO -> MEGA)
- Breadboards
- Connecting wires
- Resistors and capacitors

### 🎮 Input Devices

- 2 × Joysticks
- 2 × LDR photoresistors
- Push buttons (Start, Pause, etc.)

### 📤 Output Devices

- 2 x LED matrices (1088AS)
- 2 x MAX7221 drivers
- LCD display (menu and game information)
- Active and passive buzzers (player feedback, game sounds and theme)


## 📚 Tutorial Source

**This project does not follow a specific tutorial.**

Only small references were used, such as:

1. Arduino official documentation
2. Example code for MAX7221
3. Joystick and LED matrix usage references
4. The game logic, hardware architecture, and gameplay mechanics are original.

### 🔧 What Will Be Changed (Compared to Existing References)

1. Expansion from single-player to multiplayer
2. Larger play area using multiple matrices
3. Dynamic events (bombs, ship stalling, LDR-triggered actions)
4. Score and highscore system
5. More complex audio-visual feedback


## ❓ Project Questions


### 🧱 Q1 – What is the system boundary?

**The system includes:**

- the Arduino microcontroller
- all connected sensors and actuators
- the software running on the board

**The system does not include:**

- external computers
- networking or internet services

**All processing happens locally on the microcontroller.**


### 🧠 Q2 – Where does intelligence live?

The intelligence lives in:

- the software running on the Arduino
- the game logic (state management, collisions, random events)
- the interpretation of player inputs into meaningful game actions


### ⚙️ Q3 – What is the hardest technical problem?

The main technical challenges are:

1. synchronizing multiple input and output devices
2. handling two joysticks and multiple LED matrices
3. managing  memory and I/O pins
4. working with LED drivers
5. scaling the game without losing responsiveness

### 🧪 Q4 – What is the minimum demo?

The minimum working demo includes:

- one LED matrix
- one joystick
- basic spaceship movement (dot)
- obstacle walls
- a sound feedback on movement

#### 🎥 Demo Video 

[![Watch Video](https://img.shields.io/badge/▶️%20Video-red?style=for-the-badge)](https://youtube.com/shorts/0MiaP8hC6qI?feature=share) 

#### 📸 Setup 

![WhatsApp Image 2025-12-19 at 18 41 30](https://github.com/user-attachments/assets/5a47b574-f790-4125-9f7d-875066161042)

**Everything beyond this is an extension of the initial matrix project.**

### 🧩 Q5 – Why is this not just a tutorial?

**Because:**

1. the game logic is original
2. multiple hardware components are combined in a non-trivial way
3. the project involves design decisions in both hardware and software
4. tutorials are used only as references, not as a complete solution

## 🔌 Do You Need an ESP32?

**❌ No, an ESP32 is not required.**

**Reasons:**

- the project does not use WiFi or Bluetooth
- no advanced multitasking is needed
- an Arduino (MEGA) provides sufficient pins and memory
