# 🚀 We got a Space Problem!

## 📖 Backstory/ Introduction
This project started as a classroom Arduino challenge to build a two‑player game on LED matrices. I chose a competitive co‑op maze game because it fits the hardware well (joysticks, LED matrices, buzzer feedback) and allows interesting rules like bombs, traps, and per‑zone objectives. I built it by combining my original gameplay ideas with a menu/EEPROM system, then iterating on controls, sound, and end‑game logic until it felt complete and fair for two players.

## 🕹️ Game Description
The game is a fast‑paced two‑player maze battle on a 16x16 grid (split into four 8x8 zones). Each player sees their own 8x8 zone on a dedicated LED matrix. Players collect food tokens in each zone, avoid bombs, and manage temporary traps. The first player to complete all zone objectives wins, and scores are stored in EEPROM for a persistent leaderboard.

## 📘 How to Play
- Use your joystick to move through the maze.
- Collect food tokens in each zone (you must collect your designated food per zone).
- Avoid bombs; if you are aligned with a bomb (same row or column in the same zone) you take a hit.
- Ypu can also take a hit by bombs detonated by another player! You cannot leave the zone until the bomb explodes.
- Traps can temporarily block movement; release yourself by shadowing the LDR.
- The LCD shows current zone, remaining objectives, and hit count.
- When the game ends, the winner’s score may be saved to the leaderboard.

## 🧩 Used Components
- Arduino Mega 2560
- 1x MAX7219, 1x MAX7221 LED matrix modules
- 2x 1088AS matrices
- 2x analog joysticks
- 2x LDR sensors (trap mechanics)
- 4x buzzers (movement, hits, and effects)
- 1x LCD
- Push button (menu/confirm)
- Condensators
- Resistors, wires, and breadboard


## 🎥 Demo Video #1 - menu

[![Watch Video](https://img.shields.io/badge/▶️%20Video-red?style=for-the-badge)](https://youtube.com/shorts/GrOWqEmcQI0?feature=share)

## 🎥 Demo Video #2 - the game itself, other features

[![WAtch video](https://img.shields.io/badge/▶️%20Video-red?style=for-the-badge)](https://youtu.be/S09biIpuNjM)


## 📸 Setup

![final robotics game project](https://github.com/user-attachments/assets/aa3d99a8-6c4a-483f-8fde-bdd951bbf98a)


## 💻 Code

[![View Code](https://img.shields.io/badge/💻%20View%20Code-blue?style=for-the-badge)](https://github.com/LetMeCode01/We-got-a-Space-Problem/blob/main/Sima_Mihai_Final_Project.ino)
