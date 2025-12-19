#include <LedControl.h>
#include <LiquidCrystal.h>

// Game Map - 1 represents walls, 0 represents walkable path
byte gameMap[16][16] = {
  {1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1},
  {1,1,0,0,1,1,0,0, 0,0,1,1,0,0,0,1},
  {1,0,0,1,1,0,0,1, 1,0,0,0,0,0,0,1},
  {1,0,1,0,1,0,0,0, 0,0,0,1,1,1,0,1},
  {1,0,0,0,0,0,0,1, 1,1,0,1,0,1,0,1},
  {1,0,1,1,1,1,0,0, 0,0,1,0,0,1,0,1},
  {1,0,0,0,0,1,0,1, 1,0,0,0,1,1,0,1},
  {1,0,0,1,0,1,0,0, 0,0,1,0,0,0,0,1},

  {1,0,0,1,0,1,0,0, 0,0,1,0,0,0,0,1},
  {1,0,1,0,0,0,0,1, 1,0,0,1,1,1,0,1},
  {1,0,0,0,0,1,0,1, 1,0,0,1,0,0,0,1},
  {1,1,0,1,0,1,0,1, 1,0,0,1,0,1,0,1},
  {1,0,0,1,0,0,1,0, 0,0,0,1,1,1,0,1},
  {1,0,1,0,1,0,0,0, 0,1,0,0,0,1,0,1},
  {1,0,1,0,0,0,1,0, 0,0,1,1,0,0,0,1},
  {1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1},
};

// LED Matrix Pins
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

// LCD Display
const int rs = 9;
const int en = 8;
const int d4 = 7;
const int d5 = 6;
const int d6 = 5;
const int d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Joystick Pins
const int xPin = A1;
const int yPin = A0;

// Sensors and Actuators
const int ldrPin = A3;
const int buzzerPin = 2;

// Player State
int playerX = 1;
int playerY = 14;
int zoneWidth = 8;
int zoneHeight = 8;
bool playerVisible = true;
unsigned long lastBlink = 0;

// Movement Timing
const int minThreshold = 200;
const int maxThreshold = 800;
const unsigned long moveInterval = 250;
unsigned long lastMoved = 0;
bool joystickMoved = false;

// Trap State
bool trapActive = false;
int ldrAtTrap = 0;
const int trapChance = 10;
unsigned long lastTrapTime = 0;

// Food State
int zoneFood[4] = {3, 3, 3, 3};
int foodX[4] = {0, 0, 0, 0};
int foodY[4] = {0, 0, 0, 0};
bool foodActive[4] = {false, false, false, false};
bool foodVisible[4] = {false, false, false, false};
unsigned long lastFoodBlink = 0;
const unsigned long foodBlinkInterval = 100;
unsigned long foodRespawnTime[4] = {0, 0, 0, 0};
int totalFoodCollected = 0;

// Bomb State
bool bombActive = false;
bool bombVisible = false;
int bombX = 0;
int bombY = 0;
unsigned long bombSpawnTime = 0;
unsigned long bombDetonateTime = 0;
bool bombPlanned = false;

// Sound Functions
void playBuzzerMove() { tone(buzzerPin, 800, 100); }
void playBuzzerFood() { tone(buzzerPin, 1200, 100); }
void playBuzzerBomb() { tone(buzzerPin, 1500, 100); }
void playBuzzerZone() { tone(buzzerPin, 1000, 100); }
void playBuzzerLdr() { tone(buzzerPin, 900, 100); }

void setup() {
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  randomSeed(analogRead(A4));

  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  lcd.begin(16, 2);
  drawZone();
}

// Logic to check if player is at the edge of a zone
bool isOnZoneEdge() {
  bool atVerticalEdge = (playerX % zoneWidth == 0) || (playerX % zoneWidth == zoneWidth - 1);
  bool atHorizontalEdge = (playerY % zoneHeight == 0) || (playerY % zoneHeight == zoneHeight - 1);
  return atVerticalEdge || atHorizontalEdge;
}

// Spawns food in empty map spots within a specific zone
void spawnFood() {
  unsigned long now = millis();
  for (int i = 0; i < 4; i++) {
    bool canSpawn = (zoneFood[i] > 0) && (!foodActive[i]) && (now - foodRespawnTime[i] >= 4000);
    if (canSpawn) {
      int zx = (i % 2) * zoneWidth;
      int zy = (i / 2) * zoneHeight;

      int validX[64];
      int validY[64];
      int count = 0;

      for (int y = zy; y < zy + zoneHeight; y++) {
        for (int x = zx; x < zx + zoneWidth; x++) {
          if (gameMap[y][x] == 0 && !(x == playerX && y == playerY)) {
            validX[count] = x;
            validY[count] = y;
            count++;
          }
        }
      }

      if (count > 0) {
        int choice = random(0, count);
        foodX[i] = validX[choice];
        foodY[i] = validY[choice];
        foodActive[i] = true;
        foodVisible[i] = true;
      }
    }
  }
}

// Handles food blinking effect
void blinkFood() {
  unsigned long now = millis();
  if (now - lastFoodBlink > foodBlinkInterval) {
    lastFoodBlink = now;
    for (int i = 0; i < 4; i++) {
      if (foodActive[i]) {
        foodVisible[i] = !foodVisible[i];
      }
    }
  }
}

// Handles screen flashing when a zone is cleared
void flashZone() {
  unsigned long startFlash = millis();
  while (millis() - startFlash < 1000) {
    for (int y = 0; y < 8; y++) {
      lc.setRow(0, y, 0xFF);
    }
    playBuzzerZone();
    delay(100);
    for (int y = 0; y < 8; y++) {
      lc.setRow(0, y, 0);
    }
    delay(100);
  }
}

// Checks if player collected food
void collectFood() {
  unsigned long now = millis();
  for (int i = 0; i < 4; i++) {
    if (foodActive[i] && playerX == foodX[i] && playerY == foodY[i]) {
      foodActive[i] = false;
      zoneFood[i]--;
      totalFoodCollected++;
      foodRespawnTime[i] = now;
      playBuzzerFood();
      
      if (zoneFood[i] == 0) {
        flashZone();
      }

      int totalLeft = 0;
      for (int j = 0; j < 4; j++) {
        totalLeft += zoneFood[j];
      }
      
      if (!bombPlanned && totalLeft > 0) {
        bombSpawnTime = now + 1500;
        bombPlanned = true;
      }
    }
  }
}

// Logic for bomb spawning and detonation
void updateBomb() {
  unsigned long now = millis();
  int zoneCol = playerX / zoneWidth;
  int zoneRow = playerY / zoneHeight;
  int currentZone = zoneRow * 2 + zoneCol;
  
  int totalFoodLeft = 0;
  for (int j = 0; j < 4; j++) {
    totalFoodLeft += zoneFood[j];
  }

  if (!bombActive && bombPlanned && now >= bombSpawnTime && totalFoodLeft > 0) {
    int zx = (currentZone % 2) * zoneWidth;
    int zy = (currentZone / 2) * zoneHeight;
    int validX[64];
    int validY[64];
    int count = 0;

    for (int y = zy; y < zy + zoneHeight; y++) {
      for (int x = zx; x < zx + zoneWidth; x++) {
        if (gameMap[y][x] == 0 && !(x == playerX && y == playerY)) {
          validX[count] = x;
          validY[count] = y;
          count++;
        }
      }
    }

    if (count > 0) {
      int choice = random(0, count);
      bombX = validX[choice];
      bombY = validY[choice];
      bombActive = true;
      bombDetonateTime = now + 2000;
      bombPlanned = false;
    }
  }

  if (bombActive && now < bombDetonateTime) {
    unsigned long elapsed = now - (bombDetonateTime - 2000);
    bombVisible = (elapsed / 100) % 2 == 0;
  } else {
    bombVisible = false;
  }

  if (bombActive && now >= bombDetonateTime) {
    if (playerX == bombX || playerY == bombY) {
      playBuzzerBomb();
    }
    bombActive = false;
  }
}

// Renders the current 8x8 zone onto the LED matrix and updates LCD
void drawZone() {
  int zoneX = (playerX / zoneWidth) * zoneWidth;
  int zoneY = (playerY / zoneHeight) * zoneHeight;

  for (int y = 0; y < 8; y++) {
    byte row = 0;
    for (int x = 0; x < 8; x++) {
      int mapX = zoneX + x;
      int mapY = zoneY + y;
      bool pixel = gameMap[mapY][mapX];

      if (mapX == playerX && mapY == playerY && playerVisible) {
        pixel = true;
      }

      for (int i = 0; i < 4; i++) {
        if (foodActive[i] && foodVisible[i] && mapX == foodX[i] && mapY == foodY[i]) {
          pixel = true;
        }
      }

      if (bombActive && bombVisible && mapX == bombX && mapY == bombY) {
        pixel = true;
      }

      if (pixel) {
        row |= (1 << (7 - x));
      }
    }
    lc.setRow(0, y, row);
  }

  int zoneCol = playerX / zoneWidth;
  int zoneRow = playerY / zoneHeight;
  int currentZone = zoneRow * 2 + zoneCol;

  lcd.setCursor(0, 0);
  lcd.print("Z:");
  switch (currentZone) {
    case 0: lcd.print("NW"); break;
    case 1: lcd.print("NE"); break;
    case 2: lcd.print("SW"); break;
    case 3: lcd.print("SE"); break;
  }

  lcd.setCursor(0, 1);
  lcd.print("NW:"); lcd.print(zoneFood[0]);
  lcd.print("NE:"); lcd.print(zoneFood[1]);
  lcd.print("SW:"); lcd.print(zoneFood[2]);
  lcd.print("SE:"); lcd.print(zoneFood[3]);
}

// Checks if the player is allowed to leave the current zone
bool canMoveFromZone(int newX, int newY) {
  int zoneCol = playerX / zoneWidth;
  int zoneRow = playerY / zoneHeight;
  int currentZone = zoneRow * 2 + zoneCol;
  
  if (!foodActive[currentZone] && millis() - foodRespawnTime[currentZone] < 4000) {
    int zx = (currentZone % 2) * zoneWidth;
    int zy = (currentZone / 2) * zoneHeight;
    if (newX < zx || newX >= zx + zoneWidth || newY < zy || newY >= zy + zoneHeight) {
      return false;
    }
  }
  return true;
}

void loop() {
  unsigned long now = millis();

  if (now - lastBlink > 200) {
    playerVisible = !playerVisible;
    lastBlink = now;
  }

  blinkFood();
  spawnFood();
  collectFood();
  updateBomb();

  int xVal = analogRead(xPin);
  int yVal = analogRead(yPin);
  bool moveX = (xVal < minThreshold) || (xVal > maxThreshold);
  bool moveY = (yVal < minThreshold) || (yVal > maxThreshold);

  if (!moveX && !moveY) {
    joystickMoved = false;
  }

  if (!trapActive) {
    if ((moveX || moveY) && !joystickMoved && (now - lastMoved > moveInterval)) {
      int newX = playerX;
      int newY = playerY;

      if (moveX) {
        if (xVal < minThreshold) newX--;
        if (xVal > maxThreshold) newX++;
      } else if (moveY) {
        if (yVal < minThreshold) newY++;
        if (yVal > maxThreshold) newY--;
      }

      if (newX >= 0 && newX < 16 && newY >= 0 && newY < 16) {
        if (gameMap[newY][newX] == 0 && canMoveFromZone(newX, newY)) {
          playerX = newX;
          playerY = newY;
          playBuzzerMove();

          if (!isOnZoneEdge() && now - lastTrapTime > 1000) {
            if (random(0, trapChance) == 0) {
              trapActive = true;
              ldrAtTrap = analogRead(ldrPin);
              playBuzzerLdr();
              lastTrapTime = now;
            }
          }
        }
      }
      joystickMoved = true;
      lastMoved = now;
    }
  }

  // Handle LDR Trap - requires light change to escape
  if (trapActive) {
    int ldrNow = analogRead(ldrPin);
    if (abs(ldrNow - ldrAtTrap) >= 100) {
      trapActive = false;
      joystickMoved = false;
      lastMoved = now;
      playBuzzerLdr();
    }
  }

  drawZone();
}