#include <LedControl.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

// Global Constants
const int mapSize = 16;
const int zoneSize = 8;
const int zoneCount = 4;
const int playerCount = 2;
const int matrixSize = 8;
const int maxBombHits = 24;

const unsigned long bombDurationEasy = 2000;
const unsigned long bombDurationMedium = 1500;
const unsigned long bombDurationHard = 1000;
unsigned long bombDurationMs = bombDurationEasy;

const int trapChanceEasy = 10;
const int trapChanceMedium = 8;
const int trapChanceHard = 6;
int trapChanceValue = trapChanceEasy;

const unsigned long foodRespawnDelayMs = 4000;
const unsigned long foodBlinkIntervalMs = 100;
const unsigned long playerBlinkIntervalMs = 200;
const unsigned long moveIntervalMs = 250;
const unsigned long trapCooldownMs = 1000;
const int ldrEscapeDelta = 100;
const unsigned long zoneFlashDurationMs = 1000;
const unsigned long zoneFlashStepDelayMs = 100;
const unsigned long bombSpawnDelayMs = 1500;
const int bombTickToneHz = 2500;
const int bombTickDurationMs = 80;
const unsigned long bombBlinkIntervalMs = 100;
const int bombHitToneHz = 7000;
const int bombHitDurationMs = 200;
const unsigned long redBlinkOnMs = 300;
const unsigned long redBlinkOffMs = 200;
const unsigned long redBlinkTotalMs = 1000;
const int joystickCalibrateSamples = 20;
const int lcdScoreColumn = 12;
const int moveToneHz = 800;
const int moveToneDurationMs = 100;
const int foodToneHz = 1200;
const int foodToneDurationMs = 100;
const int bombToneHz = 1500;
const int bombToneDurationMs = 100;
const int zoneToneHz = 1000;
const int zoneToneDurationMs = 100;
const int ldrToneHz = 900;
const int ldrToneDurationMs = 100;
const int wallToneHz = 400;
const int wallToneDurationMs = 80;
const int menuMinThreshold = 300;
const int menuMaxThreshold = 700;
const unsigned long menuScrollDelayMs = 500;

// EEPROM addresses
const int matrixBrightnessEepromAddress = 0;
const int lcdBrightnessEepromAddress = 1;
const int gameDifficultyEepromAddress = 2;
const int gameSoundsEepromAddress = 45;
const int firstPlaceScoreBeginAddress = 4;
const int firstPlaceUsernameBeginAddress = 8;
const int secondPlaceScoreBeginAddress = 23;
const int secondPlaceUsernameBeginAddress = 27;
const int thirdPlaceScoreBeginAddress = 40;
const int thirdPlaceUsernameBeginAddress = 44;

// Menu Options
const int menuStartGame = 0;
const int menuHighscore = 1;
const int menuResetHighscore = 2;
const int menuLcdBrightness = 3;
const int menuMatrixBrightness = 4;
const int menuChangeDifficulty = 5;
const int menuHowToPlay = 6;
const int menuAbout = 7;
const int menuEnterName = 8;
const int menuSounds = 9;
const int noSubMenuOption = -1;

enum GameState {
  introMessageState,
  menuState,
  inGameState,
  endGameState
};

enum GameDifficulty {
  easyDifficulty,
  mediumDifficulty,
  hardDifficulty
};

enum GameSounds {
  soundsOn,
  soundsOff
};

// Game Map - 1 represents walls, 0 represents walkable path
byte gameMap[mapSize][mapSize] = {
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
const int dinPin = 42;
const int clockPin = 46;
const int loadPin = 44;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

// LED Matrix 2 Pins
const int dinPin2 = 3;
const int clockPin2 = 5;
const int loadPin2 = 2;
LedControl lc2 = LedControl(dinPin2, clockPin2, loadPin2, 1);

// LCD Display
const int rs = 13;
const int en = 12;
const int d4 = 11;
const int d5 = 10;
const int d6 = 9;
const int d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int lcdColumns = 16;
const unsigned long lcdScrollInterval = 500;
unsigned long lastLcdScroll = 0;
int lcdOffset1 = 0;
int lcdOffset2 = 0;
int lcdDir1 = 1;
int lcdDir2 = 1;
char lcdLine1[48];
char lcdLine2[48];
const int lcdBacklightPin = 4;
int lcdBrightness = 5;
byte matrixBrightness = 8;

// Menu State
GameState gameState = menuState;
int showMenuOption = menuStartGame;
int subMenuOption = noSubMenuOption;
int scrollingTextPos = 0;
unsigned long lastMenuScroll = 0;
bool joyMovedMenuX = false;
bool joyMovedMenuY = false;

// Game Settings
GameDifficulty gameDifficulty = easyDifficulty;
GameDifficulty settingsGameDifficulty = easyDifficulty;
GameSounds gameSounds = soundsOn;
GameSounds settingsGameSounds = soundsOn;

// Reset Button
const int resetButtonPin = 49;
bool resetButtonLastState = HIGH;
bool resetButtonState = HIGH;
bool menuButtonReady = false;
unsigned long lastResetDebounce = 0;
const unsigned long resetDebounceMs = 50;

// Traffic Lights
const int traffic1GreenPin = 26;
const int traffic1YellowPin = 24;
const int traffic1RedPin = 22;
const int traffic2GreenPin = 27;
const int traffic2YellowPin = 25;
const int traffic2RedPin = 23;

// Joystick Pins
const int xPin = A6;
const int yPin = A7;

// Joystick 2 Pins
const int xPin2 = A15;
const int yPin2 = A14;

// Sensors and Actuators
const int ldrPin = A10;
const int buzzerPin = 6;

// Sensors and Actuators 2
const int ldrPin2 = A11;
const int buzzerPin2 = 28;

// Bomb Tick Buzzers
const int buzzerPin3 = 51;
const int buzzerPin4 = 47;

// Player State
int playerX = 1;
int playerY = 14;
const int zoneWidth = zoneSize;
const int zoneHeight = zoneSize;
bool playerVisible = true;
unsigned long lastBlink = 0;

// Player 2 State
int player2X = 14;
int player2Y = 14;
bool player2Visible = true;
unsigned long lastBlink2 = 0;

// Movement Timing
const int minThreshold = 200;
const int maxThreshold = 800;
const int minThreshold1 = 200;
const int maxThreshold1 = 800;
const int joystick1Deadzone = 120;
const int joystick1Samples = 5;
const int joystick1DominantMargin = 60;
int joystick1CenterX = 512;
int joystick1CenterY = 512;
const unsigned long moveInterval = moveIntervalMs;
unsigned long lastMoved = 0;
bool joystickMoved = false;

// Player 2 Movement Timing
unsigned long lastMoved2 = 0;
bool joystickMoved2 = false;

// Trap State
bool trapActive = false;
int ldrAtTrap = 0;
int trapChance = trapChanceValue;
unsigned long lastTrapTime = 0;

// Trap State Player 2
bool trapActive2 = false;
int ldrAtTrap2 = 0;
unsigned long lastTrapTime2 = 0;

// Food State
const int foodsPerZone = 6;
const int foodsPerPlayerTarget = 3;
int zoneFoodRemaining[zoneCount] = {foodsPerZone, foodsPerZone, foodsPerZone, foodsPerZone};
int playerZoneFood[playerCount][zoneCount] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};
bool playerZoneCompleted[playerCount][zoneCount] = {
  {false, false, false, false},
  {false, false, false, false}
};
int foodX[zoneCount] = {0, 0, 0, 0};
int foodY[zoneCount] = {0, 0, 0, 0};
bool foodActive[zoneCount] = {false, false, false, false};
bool foodVisible[zoneCount] = {false, false, false, false};
unsigned long lastFoodBlink = 0;
const unsigned long foodBlinkInterval = foodBlinkIntervalMs;
unsigned long foodRespawnTime[zoneCount] = {0, 0, 0, 0};
int totalFoodCollected1 = 0;
int totalFoodCollected2 = 0;

// Bomb State
bool bombActive = false;
bool bombVisible = false;
int bombX = 0;
int bombY = 0;
int bombZone = -1;
int bombOwner = -1;
unsigned long lastBombTick = 0;
const unsigned long bombTickInterval = 200;
unsigned long bombSpawnTime = 0;
unsigned long bombDetonateTime = 0;
bool bombPlanned = false;
int bombPlannedZone = -1;
int playerBombHits[playerCount] = {0, 0};

// Game End State
bool gameOver = false;
int winnerIndex = -1;

// Red Blink State (Bomb Hit)
bool redBlinkActive[playerCount] = {false, false};
bool redBlinkOn[playerCount] = {false, false};
unsigned long redBlinkStart[playerCount] = {0, 0};
unsigned long redBlinkLastToggle[playerCount] = {0, 0};

// Highscore State
const int maxScoreValue = 0;
const int maxPossibleScore = 6000;
const int firstPlace = 1;
const int secondPlace = 2;
const int thirdPlace = 3;
const int noLeaderBoardPosition = -1;
int leaderBoardPosition = noLeaderBoardPosition;
int firstPlaceScore = maxScoreValue;
int secondPlaceScore = maxScoreValue;
int thirdPlaceScore = maxScoreValue;
const int usernameLength = 10;
const int noUserLength = 8;
char firstPlaceUsername[usernameLength + 1];
char secondPlaceUsername[usernameLength + 1];
char thirdPlaceUsername[usernameLength + 1];
char firstAlphabetChar = 'a';
char lastAlphabetChar = 'z';
char currentUsernameChar = 'a';
int currentUsernamePosition = 0;
String userName = "";
int winnerScore = 0;

// End Screen Letters
const byte letterW[8] = {
  B10000001,
  B10000001,
  B10000001,
  B10011001,
  B10100101,
  B11000011,
  B10000001,
  B10000001
};

const byte letterL[8] = {
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B11111111
};

// LCD Custom Characters
const byte progressBarCharacter[matrixSize] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};
const int progressBarCharacterIndex = 0;

const byte customArrow[matrixSize] = {
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
};
const int customArrowIndex = 2;

const byte customSlashFirstRow[matrixSize] = {
  0b00000,
  0b00000,
  0b00000,
  0b00001,
  0b00010,
  0b00100,
  0b01000,
  0b10000
};
const int customSlashFirstRowIndex = 3;

const byte customSlashSecondRow[matrixSize] = {
  0b00001,
  0b00010,
  0b00100,
  0b01000,
  0b10000,
  0b00000,
  0b00000,
  0b00000
};
const int customSlashSecondRowIndex = 4;

const byte customBackslashFirstRow[matrixSize] = {
  0b00000,
  0b00000,
  0b00000,
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00001
};
const int customBackslashFirstRowIndex = 5;

const byte customBackslashSecondRow[matrixSize] = {
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00001,
  0b00000,
  0b00000,
  0b00000
};
const int customBackslashSecondRowIndex = 6;

// Menu Icons
const byte playIcon[matrixSize] PROGMEM = {
  B00100000,
  B00110000,
  B00111000,
  B00111100,
  B00111000,
  B00110000,
  B00100000,
  B00000000
};

const byte aboutIcon[matrixSize] PROGMEM = {
  B00000000,
  B00011000,
  B00011000,
  B01000010,
  B01111110,
  B00011000,
  B00011000,
  B00000000
};

const byte gameSoundsIcon[matrixSize] PROGMEM = {
  B00000000,
  B00010000,
  B00111000,
  B11111110,
  B00111000,
  B00111000,
  B00010000,
  B00000000
};

const byte leaderboardIcon[matrixSize] PROGMEM = {
  B00000000,
  B10000000,
  B11100000,
  B11111000,
  B11111110,
  B11111000,
  B11100000,
  B10000000
};

const byte resetScoreIcon[matrixSize] PROGMEM = {
  B00000000,
  B00111100,
  B01000010,
  B10000001,
  B10000001,
  B10011001,
  B01000010,
  B00111100
};

const byte lcdIcon[matrixSize] PROGMEM = {
  B00000000,
  B00011000,
  B00111100,
  B00111100,
  B00111100,
  B00111100,
  B00011000,
  B00000000
};

const byte matrixIcon[matrixSize] PROGMEM = {
  B11111111,
  B10000001,
  B10100101,
  B10000001,
  B10100101,
  B10000001,
  B11111111,
  B00000000
};

const byte difficultyIcon[matrixSize] PROGMEM = {
  B00000000,
  B00011000,
  B00111100,
  B01111110,
  B01111110,
  B00111100,
  B00011000,
  B00000000
};

const byte questionMarkIcon[matrixSize] PROGMEM = {
  B00111100,
  B01000010,
  B00000010,
  B00001100,
  B00010000,
  B00000000,
  B00010000,
  B00000000
};

// -- SOUND --

void playBuzzerMove() 
{ if (gameSounds == soundsOn) tone(buzzerPin, moveToneHz, moveToneDurationMs); }
void playBuzzerFood() 
{ if (gameSounds == soundsOn) tone(buzzerPin, foodToneHz, foodToneDurationMs); }
void playBuzzerBomb() 
{ if (gameSounds == soundsOn) tone(buzzerPin, bombToneHz, bombToneDurationMs); }
void playBuzzerZone() 
{ if (gameSounds == soundsOn) tone(buzzerPin, zoneToneHz, zoneToneDurationMs); }
void playBuzzerLdr() 
{ if (gameSounds == soundsOn) tone(buzzerPin, ldrToneHz, ldrToneDurationMs); }
void playBuzzerWall() 
{ if (gameSounds == soundsOn) tone(buzzerPin, wallToneHz, wallToneDurationMs); }

// Play move sound for player 2.
void playBuzzerMove2() { if (gameSounds == soundsOn) tone(buzzerPin2, moveToneHz, moveToneDurationMs); }
void playBuzzerFood2() { if (gameSounds == soundsOn) tone(buzzerPin2, foodToneHz, foodToneDurationMs); }
void playBuzzerBomb2() { if (gameSounds == soundsOn) tone(buzzerPin2, bombToneHz, bombToneDurationMs); }
void playBuzzerZone2() { if (gameSounds == soundsOn) tone(buzzerPin2, zoneToneHz, zoneToneDurationMs); }
void playBuzzerLdr2() { if (gameSounds == soundsOn) tone(buzzerPin2, ldrToneHz, ldrToneDurationMs); }
void playBuzzerWall2() { if (gameSounds == soundsOn) tone(buzzerPin2, wallToneHz, wallToneDurationMs); }
void playBuzzerBomb3() { if (gameSounds == soundsOn) tone(buzzerPin3, bombHitToneHz, bombHitDurationMs); }
void playBuzzerBomb4() { if (gameSounds == soundsOn) tone(buzzerPin4, bombHitToneHz, bombHitDurationMs); }

// -- TRAFFIC LIGHTS --
// Start red blink window for a player.
void startRedBlink(int playerIndex, unsigned long now) {
  if (playerIndex < 0 || playerIndex >= playerCount) return;
  redBlinkActive[playerIndex] = true;
  redBlinkOn[playerIndex] = true;
  redBlinkStart[playerIndex] = now;
  redBlinkLastToggle[playerIndex] = now;
}

// Update traffic lights based on game state and traps.
void updateTrafficLights(unsigned long now) {
  bool inMenuOrEnd = (gameState == menuState || gameState == endGameState);

  bool playerBlocked[2] = {trapActive, trapActive2};

  if (inMenuOrEnd) {
    digitalWrite(traffic1GreenPin, LOW);
    digitalWrite(traffic1YellowPin, HIGH);
    digitalWrite(traffic2GreenPin, LOW);
    digitalWrite(traffic2YellowPin, HIGH);
  } else {
    digitalWrite(traffic1GreenPin, playerBlocked[0] ? LOW : HIGH);
    digitalWrite(traffic1YellowPin, playerBlocked[0] ? HIGH : LOW);
    digitalWrite(traffic2GreenPin, playerBlocked[1] ? LOW : HIGH);
    digitalWrite(traffic2YellowPin, playerBlocked[1] ? HIGH : LOW);
  }

  for (int i = 0; i < playerCount; i++) {
    int redPin = (i == 0) ? traffic1RedPin : traffic2RedPin;

    if (redBlinkActive[i]) {
      if (now - redBlinkStart[i] >= redBlinkTotalMs) {
        redBlinkActive[i] = false;
        redBlinkOn[i] = false;
        digitalWrite(redPin, LOW);
      } else {
        unsigned long interval = redBlinkOn[i] ? redBlinkOnMs : redBlinkOffMs;
        if (now - redBlinkLastToggle[i] >= interval) {
          redBlinkOn[i] = !redBlinkOn[i];
          redBlinkLastToggle[i] = now;
        }
        digitalWrite(redPin, redBlinkOn[i] ? HIGH : LOW);
      }
    } else {
      digitalWrite(redPin, LOW);
    }
  }
}

// -- SETUP --
// Initialize hardware and initial menu state.
void setup() 
{

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  pinMode(buzzerPin2, OUTPUT);
  digitalWrite(buzzerPin2, LOW);
  pinMode(buzzerPin3, OUTPUT);
  digitalWrite(buzzerPin3, LOW);
  pinMode(buzzerPin4, OUTPUT);
  digitalWrite(buzzerPin4, LOW);

  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(lcdBacklightPin, OUTPUT);
  pinMode(resetButtonPin, INPUT_PULLUP);

  pinMode(traffic1GreenPin, OUTPUT);
  pinMode(traffic1YellowPin, OUTPUT);
  pinMode(traffic1RedPin, OUTPUT);
  pinMode(traffic2GreenPin, OUTPUT);
  pinMode(traffic2YellowPin, OUTPUT);
  pinMode(traffic2RedPin, OUTPUT);
  digitalWrite(traffic1GreenPin, LOW);
  digitalWrite(traffic1YellowPin, HIGH);
  digitalWrite(traffic1RedPin, LOW);
  digitalWrite(traffic2GreenPin, LOW);
  digitalWrite(traffic2YellowPin, HIGH);
  digitalWrite(traffic2RedPin, LOW);

  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  lc2.shutdown(0, false);
  lc2.setIntensity(0, 8);
  lc2.clearDisplay(0);

  long centerSumX = 0;
  long centerSumY = 0;
  for (int i = 0; i < joystickCalibrateSamples; i++) {
    centerSumX += analogRead(xPin);
    centerSumY += analogRead(yPin);
  }
  joystick1CenterX = centerSumX / joystickCalibrateSamples;
  joystick1CenterY = centerSumY / joystickCalibrateSamples;

  lcd.begin(16, 2);
  lcd.createChar(progressBarCharacterIndex, progressBarCharacter);
  lcd.createChar(customArrowIndex, customArrow);
  lcd.createChar(customSlashFirstRowIndex, customSlashFirstRow);
  lcd.createChar(customSlashSecondRowIndex, customSlashSecondRow);
  lcd.createChar(customBackslashFirstRowIndex, customBackslashFirstRow);
  lcd.createChar(customBackslashSecondRowIndex, customBackslashSecondRow);

  readGameVariablesFromEEPROM();
  gameState = menuState;
  showMenuOption = menuStartGame;
  subMenuOption = noSubMenuOption;
  lcd.clear();
  displayMenuOptions();
}

// -- GAMEPLAY --
// Check if player 1 is on a zone edge.
bool isOnZoneEdge() {
  bool atVerticalEdge = (playerX % zoneWidth == 0) || (playerX % zoneWidth == zoneWidth - 1);
  bool atHorizontalEdge = (playerY % zoneHeight == 0) || (playerY % zoneHeight == zoneHeight - 1);
  return atVerticalEdge || atHorizontalEdge;
}

// Spawn food in each zone when needed.
void spawnFood() {
  unsigned long now = millis();
  for (int i = 0; i < zoneCount; i++) {
    if (zoneFoodRemaining[i] == 0) {
      foodActive[i] = false;
      foodVisible[i] = false;
      continue;
    }

    bool canSpawn = (zoneFoodRemaining[i] > 0) && (!foodActive[i]) && (now - foodRespawnTime[i] >= foodRespawnDelayMs);
    if (canSpawn) {
      
      int zx = (i % 2) * zoneWidth;
      int zy = (i / 2) * zoneHeight;
      int validX[zoneSize * zoneSize];
      int validY[zoneSize * zoneSize];
      int count = 0;

      for (int y = zy; y < zy + zoneHeight; y++) {
        for (int x = zx; x < zx + zoneWidth; x++) {
          if (gameMap[y][x] == 0 && !(x == playerX && y == playerY) && !(x == player2X && y == player2Y)) {
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

// Toggle food blinking visibility.
void blinkFood() {
  unsigned long now = millis();
  if (now - lastFoodBlink > foodBlinkInterval) {
    lastFoodBlink = now;
    for (int i = 0; i < zoneCount; i++) {
      if (foodActive[i])
        foodVisible[i] = !foodVisible[i];
    }
  }
}

// Flash player 1 zone on completion.
void flashZone() {
  unsigned long startFlash = millis();
  while (millis() - startFlash < zoneFlashDurationMs) {
    for (int y = 0; y < matrixSize; y++)
      lc.setRow(0, y, 0xFF);
    playBuzzerZone();
    delay(zoneFlashStepDelayMs);
    for (int y = 0; y < matrixSize; y++)
      lc.setRow(0, y, 0);
    delay(zoneFlashStepDelayMs);
  }
}

// Flash player 2 zone on completion.
void flashZonePlayer2() {
  unsigned long startFlash = millis();
  while (millis() - startFlash < zoneFlashDurationMs) {
    for (int y = 0; y < matrixSize; y++)
      lc2.setRow(0, y, 0xFF);
    playBuzzerZone2();
    delay(zoneFlashStepDelayMs);
    for (int y = 0; y < matrixSize; y++)
      lc2.setRow(0, y, 0);
    delay(zoneFlashStepDelayMs);
  }
}

// Handle food collection for both players.
void collectFood() {
  unsigned long now = millis();
  for (int i = 0; i < zoneCount; i++) {
    if (foodActive[i] && playerX == foodX[i] && playerY == foodY[i]) {
      if (playerZoneCompleted[0][i]) {
        playBuzzerWall();
        continue;
      }
      foodActive[i] = false;
      zoneFoodRemaining[i]--;
      playerZoneFood[0][i]++;
      totalFoodCollected1++;
      foodRespawnTime[i] = now;
      playBuzzerFood();

      if (!playerZoneCompleted[0][i] && playerZoneFood[0][i] >= foodsPerPlayerTarget) {
        playerZoneCompleted[0][i] = true;
        flashZone();
      }

      if (isPlayerCompletedAll(0)) {
        gameOver = true;
        winnerIndex = 0;
        winnerScore = getPlayerScore(0);
        evaluateHighscore(winnerScore);
        gameState = endGameState;
      }

      int totalLeft = 0;
      for (int j = 0; j < zoneCount; j++) {
        totalLeft += zoneFoodRemaining[j];
      }

      if (!bombPlanned && totalLeft > 0) {
        bombSpawnTime = now + bombSpawnDelayMs;
        bombPlanned = true;
        bombPlannedZone = i;
        bombOwner = 0;
      }
    } else if (foodActive[i] && player2X == foodX[i] && player2Y == foodY[i]) {
      if (playerZoneCompleted[1][i]) {
        playBuzzerWall2();
        continue;
      }
      foodActive[i] = false;
      zoneFoodRemaining[i]--;
      playerZoneFood[1][i]++;
      totalFoodCollected2++;
      foodRespawnTime[i] = now;
      playBuzzerFood2();

      if (!playerZoneCompleted[1][i] && playerZoneFood[1][i] >= foodsPerPlayerTarget) {
        playerZoneCompleted[1][i] = true;
        flashZonePlayer2();
      }

      if (isPlayerCompletedAll(1)) {
        gameOver = true;
        winnerIndex = 1;
        winnerScore = getPlayerScore(1);
        evaluateHighscore(winnerScore);
        gameState = endGameState;
      }

      int totalLeft = 0;
      for (int j = 0; j < zoneCount; j++)
        totalLeft += zoneFoodRemaining[j];

      if (!bombPlanned && totalLeft > 0) {
        bombSpawnTime = now + bombSpawnDelayMs;
        bombPlanned = true;
        bombPlannedZone = i;
        bombOwner = 1;
      }
    }
  }
}

// Update bomb spawn, blink, and detonation effects.
void updateBomb() {
  unsigned long now = millis();
  int totalFoodLeft = 0;
  for (int j = 0; j < zoneCount; j++) {
    totalFoodLeft += zoneFoodRemaining[j];
  }

  if (!bombActive && bombPlanned && now >= bombSpawnTime && totalFoodLeft > 0) {
    int zx = (bombPlannedZone % 2) * zoneWidth;
    int zy = (bombPlannedZone / 2) * zoneHeight;
    int validX[64];
    int validY[64];
    int count = 0;

    for (int y = zy; y < zy + zoneHeight; y++) {
      for (int x = zx; x < zx + zoneWidth; x++) {
        if (gameMap[y][x] == 0 && !(x == playerX && y == playerY) && !(x == player2X && y == player2Y)) {
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
      bombZone = bombPlannedZone;
      bombDetonateTime = now + bombDurationMs;
      bombPlanned = false;
      bombPlannedZone = -1;
      lastBombTick = now;
    }
  }

  if (bombActive && now < bombDetonateTime) {
    unsigned long elapsed = now - (bombDetonateTime - bombDurationMs);
    bombVisible = (elapsed / bombBlinkIntervalMs) % 2 == 0;

    if (now - lastBombTick >= bombTickInterval) {
      lastBombTick = now;
      int player1Zone = getZoneIndex(playerX, playerY);
      int player2Zone = getZoneIndex(player2X, player2Y);
      int tickPin = (bombOwner == 0) ? buzzerPin3 : buzzerPin4;

      if (player1Zone == bombZone && player2Zone == bombZone) {
        tickPin = buzzerPin3;
      }

      if (gameSounds == soundsOn) {
        tone(tickPin, bombTickToneHz, bombTickDurationMs);
      }
    }
  } else  bombVisible = false;

  if (bombActive && now >= bombDetonateTime) {
    int player1Zone = getZoneIndex(playerX, playerY);
    int player2Zone = getZoneIndex(player2X, player2Y);
    int tickPin = (bombOwner == 0) ? buzzerPin3 : buzzerPin4;

    if (player1Zone == bombZone && player2Zone == bombZone) 
      tickPin = buzzerPin3;

    if (gameSounds == soundsOn) 
      tone(tickPin, bombTickToneHz, bombTickDurationMs);

    if (player1Zone == bombZone && (playerX == bombX || playerY == bombY)) {
      playBuzzerBomb3();
      if (playerBombHits[0] < maxBombHits) playerBombHits[0]++;
      startRedBlink(0, now);
    }
    if (player2Zone == bombZone && (player2X == bombX || player2Y == bombY)) {
      playBuzzerBomb4();
      if (playerBombHits[1] < maxBombHits) playerBombHits[1]++;
      startRedBlink(1, now);
    }
    bombActive = false;
    bombZone = -1;
    bombOwner = -1;
  }
}

// -- RENDERING --
// Render player 1 zone to matrix and LCD.
void drawZone() {
  int zoneX = (playerX / zoneWidth) * zoneWidth;
  int zoneY = (playerY / zoneHeight) * zoneHeight;
  bool sameZone = (getZoneIndex(playerX, playerY) == getZoneIndex(player2X, player2Y));

  for (int y = 0; y < matrixSize; y++) {
    byte row = 0;
    for (int x = 0; x < matrixSize; x++) {
      int mapX = zoneX + x;
      int mapY = zoneY + y;
      bool pixel = gameMap[mapY][mapX];

      if (mapX == playerX && mapY == playerY && playerVisible) {
        pixel = true;
      }

      if (sameZone && mapX == player2X && mapY == player2Y && player2Visible) {
        pixel = true;
      }

      for (int i = 0; i < zoneCount; i++) {
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

  updateLcdStatus();
}

// Renders the current 8x8 zone for player 2 onto the second LED matrix
// Render player 2 zone to matrix.
void drawZonePlayer2() {
  int zoneX = (player2X / zoneWidth) * zoneWidth;
  int zoneY = (player2Y / zoneHeight) * zoneHeight;
  bool sameZone = (getZoneIndex(playerX, playerY) == getZoneIndex(player2X, player2Y));

  for (int y = 0; y < matrixSize; y++) {
    byte row = 0;
    for (int x = 0; x < matrixSize; x++) {
      int mapX = zoneX + x;
      int mapY = zoneY + y;
      bool pixel = gameMap[mapY][mapX];

      if (mapX == player2X && mapY == player2Y && player2Visible) {
        pixel = true;
      }

      if (sameZone && mapX == playerX && mapY == playerY && playerVisible) {
        pixel = true;
      }

      for (int i = 0; i < zoneCount; i++) {
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
    lc2.setRow(0, y, row);
  }
}

// Checks if the player is allowed to leave the current zone
// -- MOVEMENT RULES --
// Check if movement is blocked by bomb zone boundaries.
bool canMoveFromZone(int newX, int newY) {
  int zoneCol = playerX / zoneWidth;
  int zoneRow = playerY / zoneHeight;
  int currentZone = zoneRow * 2 + zoneCol;
  
  if (!foodActive[currentZone] && millis() - foodRespawnTime[currentZone] < foodRespawnDelayMs) {
    int zx = (currentZone % 2) * zoneWidth;
    int zy = (currentZone / 2) * zoneHeight;
    if (newX < zx || newX >= zx + zoneWidth || newY < zy || newY >= zy + zoneHeight) {
      return false;
    }
  }
  return true;
}

// Get zone index for map coordinates.
int getZoneIndex(int x, int y) {
  int zoneCol = x / zoneWidth;
  int zoneRow = y / zoneHeight;
  return zoneRow * 2 + zoneCol;
}

// Check if food exists at coordinates in a zone.
bool isFoodAt(int x, int y, int zone) {
  return foodActive[zone] && foodX[zone] == x && foodY[zone] == y;
}

// Check if movement is allowed while bomb is active.
bool canMoveWithBomb(int currentX, int currentY, int newX, int newY) {
  if (!bombActive || bombZone < 0) {
    return true;
  }

  int currentZone = getZoneIndex(currentX, currentY);
  int targetZone = getZoneIndex(newX, newY);

  if (currentZone == bombZone && targetZone != bombZone) {
    return false;
  }

  if (currentZone != bombZone && targetZone == bombZone) {
    return false;
  }

  return true;
}

// -- SETTINGS --
// Apply difficulty to bomb and trap parameters.
void applyDifficultySettings() {
  switch (gameDifficulty) {
    case easyDifficulty:
      bombDurationMs = bombDurationEasy;
      trapChanceValue = trapChanceEasy;
      trapChance = trapChanceValue;
      break;
    case mediumDifficulty:
      bombDurationMs = bombDurationMedium;
      trapChanceValue = trapChanceMedium;
      trapChance = trapChanceValue;
      break;
    case hardDifficulty:
      bombDurationMs = bombDurationHard;
      trapChanceValue = trapChanceHard;
      trapChance = trapChanceValue;
      break;
  }
}

// -- DISPLAY / EEPROM --
// Draw a PROGMEM icon on both matrices.
void displayIconDual(const byte* icon) {
  for (int row = 0; row < matrixSize; row++) {
    byte rowData = pgm_read_byte(&icon[row]);
    for (int col = 0; col < matrixSize; col++) {
      bool state = rowData & (1 << (matrixSize - 1 - col));
      lc.setLed(0, row, col, state);
      lc2.setLed(0, row, col, state);
    }
  }
}

// Apply LCD backlight brightness.
void applyLcdBacklight() {
  if (lcdBacklightPin >= 0) {
    int brightnessValue = constrain(lcdBrightness, 1, lcdColumns);
    analogWrite(lcdBacklightPin, map(brightnessValue, 1, lcdColumns, 0, 255));
  }
}

// Apply matrix intensity to both matrices.
void setMatrixBrightness() {
  matrixBrightness = constrain(matrixBrightness, 0, 15);
  lc.setIntensity(0, matrixBrightness);
  lc2.setIntensity(0, matrixBrightness);
}

// Load settings and highscores from EEPROM.
void readGameVariablesFromEEPROM() {
  EEPROM.get(matrixBrightnessEepromAddress, matrixBrightness);
  if (matrixBrightness > 15) {
    matrixBrightness = 8;
  }
  setMatrixBrightness();

  EEPROM.get(lcdBrightnessEepromAddress, lcdBrightness);
  applyLcdBacklight();

  EEPROM.get(gameSoundsEepromAddress, gameSounds);
  if (gameSounds != soundsOn && gameSounds != soundsOff) {
    gameSounds = soundsOn;
  }

  EEPROM.get(gameDifficultyEepromAddress, gameDifficulty);
  applyDifficultySettings();

  EEPROM.get(firstPlaceScoreBeginAddress, firstPlaceScore);
  EEPROM.get(secondPlaceScoreBeginAddress, secondPlaceScore);
  EEPROM.get(thirdPlaceScoreBeginAddress, thirdPlaceScore);
  if (firstPlaceScore < 0 || firstPlaceScore > maxPossibleScore) firstPlaceScore = maxScoreValue;
  if (secondPlaceScore < 0 || secondPlaceScore > maxPossibleScore) secondPlaceScore = maxScoreValue;
  if (thirdPlaceScore < 0 || thirdPlaceScore > maxPossibleScore) thirdPlaceScore = maxScoreValue;
  for (int i = 0; i <= usernameLength; i++) {
    firstPlaceUsername[i] = EEPROM.read(firstPlaceUsernameBeginAddress + i);
    secondPlaceUsername[i] = EEPROM.read(secondPlaceUsernameBeginAddress + i);
    thirdPlaceUsername[i] = EEPROM.read(thirdPlaceUsernameBeginAddress + i);
  }

  for (int i = 0; i <= usernameLength; i++) {
    if (firstPlaceUsername[i] == (char)0xFF) firstPlaceUsername[i] = '\0';
    if (secondPlaceUsername[i] == (char)0xFF) secondPlaceUsername[i] = '\0';
    if (thirdPlaceUsername[i] == (char)0xFF) thirdPlaceUsername[i] = '\0';
  }

  if (strlen(firstPlaceUsername) == 0) strcpy(firstPlaceUsername, "null");
  if (strlen(secondPlaceUsername) == 0) strcpy(secondPlaceUsername, "null");
  if (strlen(thirdPlaceUsername) == 0) strcpy(thirdPlaceUsername, "null");
}

// Save difficulty setting to EEPROM.
void writeDifficultyToEeprom() {
  EEPROM.put(gameDifficultyEepromAddress, gameDifficulty);
}

// Save sound setting to EEPROM.
void writeGameSoundsToEeprom() {
  EEPROM.put(gameSoundsEepromAddress, gameSounds);
}

// Scroll a long text on LCD row 2.
void displayScrollingText(const String& text) {
  unsigned long now = millis();
  if (now - lastMenuScroll > menuScrollDelayMs) {
    lastMenuScroll = now;
    lcd.setCursor(0, 1);
    int maxIndex = max(0, (int)text.length() - lcdColumns);
    lcd.print(text.substring(scrollingTextPos, min(scrollingTextPos + lcdColumns, (int)text.length())));
    scrollingTextPos = (scrollingTextPos >= maxIndex) ? 0 : (scrollingTextPos + 1);
  }
}

// Build and scroll leaderboard text.
void displayHighscore() {
  String message = "1. ";
  message.concat(firstPlaceUsername);
  message.concat(" S:");
  message.concat(firstPlaceScore);
  message.concat(" 2. ");
  message.concat(secondPlaceUsername);
  message.concat(" S:");
  message.concat(secondPlaceScore);
  message.concat(" 3. ");
  message.concat(thirdPlaceUsername);
  message.concat(" S:");
  message.concat(thirdPlaceScore);
  message.concat("  ");
  displayScrollingText(message);
}

// Reset leaderboard values in EEPROM.
void resetHighscores() {
  EEPROM.put(firstPlaceScoreBeginAddress, maxScoreValue);
  EEPROM.put(secondPlaceScoreBeginAddress, maxScoreValue);
  EEPROM.put(thirdPlaceScoreBeginAddress, maxScoreValue);
  firstPlaceScore = maxScoreValue;
  secondPlaceScore = maxScoreValue;
  thirdPlaceScore = maxScoreValue;

  const char noUserValue[] = "null";
  for (int i = 0; i <= usernameLength; i++) {
    char c = (i < (int)strlen(noUserValue)) ? noUserValue[i] : '\0';
    firstPlaceUsername[i] = c;
    secondPlaceUsername[i] = c;
    thirdPlaceUsername[i] = c;
    EEPROM.put(firstPlaceUsernameBeginAddress + i, c);
    EEPROM.put(secondPlaceUsernameBeginAddress + i, c);
    EEPROM.put(thirdPlaceUsernameBeginAddress + i, c);
  }
}

// -- MENU --
// Render main menu option line and icon.
void displayNoSubMenuOptions() {
  lcd.setCursor(0, 0);
  lcd.write((byte)customSlashFirstRowIndex);
  lcd.print("     MENU     ");
  lcd.setCursor(lcdColumns - 1, 0);
  lcd.write((byte)customBackslashFirstRowIndex);
  lcd.setCursor(0, 1);
  lcd.write((byte)customBackslashSecondRowIndex);

  switch (showMenuOption) {
    case menuStartGame:
      lcd.print("  Start game  ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(playIcon);
      break;
    case menuHighscore:
      lcd.print(" Leaderboard  ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(leaderboardIcon);
      break;
    case menuResetHighscore:
      lcd.print(" Reset Scores ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(resetScoreIcon);
      break;
    case menuLcdBrightness:
      lcd.print("     LCD      ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(lcdIcon);
      break;
    case menuMatrixBrightness:
      lcd.print("   Matrix     ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(matrixIcon);
      break;
    case menuChangeDifficulty:
      lcd.print("  Difficulty  ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(difficultyIcon);
      break;
    case menuSounds:
      lcd.print("  Game sounds ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(gameSoundsIcon);
      break;
    case menuHowToPlay:
      lcd.print(" How to Play? ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(questionMarkIcon);
      break;
    case menuAbout:
      lcd.print("  About game  ");
      lcd.setCursor(lcdColumns - 1, 1);
      lcd.write((byte)customSlashSecondRowIndex);
      displayIconDual(aboutIcon);
      break;
  }
}

// Render difficulty submenu state.
void displayChangeDifficulty() {
  lcd.setCursor(2, 0);
  lcd.print(" Difficulty ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  switch (settingsGameDifficulty) {
    case easyDifficulty:
      lcd.print("   EASY   ");
      break;
    case mediumDifficulty:
      lcd.print("  MEDIUM  ");
      break;
    case hardDifficulty:
      lcd.print("   HARD   ");
      break;
  }
  lcd.setCursor(lcdColumns - 1, 1);
  lcd.write((byte)customArrowIndex);
}

// Render sounds submenu state.
void displaySoundsOption() {
  lcd.setCursor(1, 0);
  lcd.print(" Game Sounds ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  switch (settingsGameSounds) {
    case soundsOn:
      lcd.print("    ON    ");
      break;
    case soundsOff:
      lcd.print("    OFF   ");
      break;
  }
  lcd.setCursor(lcdColumns - 1, 1);
  lcd.write((byte)customArrowIndex);
}

// Render username entry screen.
void enterUsername() {
  lcd.setCursor(2, 0);
  lcd.print("Username");
  lcd.setCursor(lcdColumns - 3, 0);
  lcd.write((byte)customArrowIndex);
  lcd.setCursor(1, 1);
  lcd.print(userName);
  lcd.setCursor(userName.length() + 1, 1);
  lcd.print(currentUsernameChar);
}

// Move username character backward.
void changeCharUp() {
  if (currentUsernameChar == firstAlphabetChar) {
    currentUsernameChar = lastAlphabetChar;
  } else {
    currentUsernameChar--;
  }
}

// Move username character forward.
void changeCharDown() {
  if (currentUsernameChar == lastAlphabetChar) {
    currentUsernameChar = firstAlphabetChar;
  } else {
    currentUsernameChar++;
  }
}

// Accept current username character.
void commitUsernameChar() {
  if (currentUsernamePosition < usernameLength) {
    userName.concat(currentUsernameChar);
    currentUsernamePosition++;
    currentUsernameChar = firstAlphabetChar;
  }
}

// Render current menu or submenu.
void displayMenuOptions() {
  switch (subMenuOption) {
    case noSubMenuOption:
      displayNoSubMenuOptions();
      break;
    case menuEnterName:
      enterUsername();
      break;
    case menuLcdBrightness:
      lcd.setCursor(0, 0);
      lcd.print(" LCD Brightness");
      lcdBrightness = constrain(lcdBrightness, 1, lcdColumns);
      applyLcdBacklight();
      lcd.setCursor(0, 1);
      for (int i = 0; i < lcdColumns; i++) {
        lcd.print(" ");
      }
      for (int i = 0; i < lcdBrightness; i++) {
        lcd.setCursor(i, 1);
        lcd.write((byte)progressBarCharacterIndex);
      }
      break;
    case menuMatrixBrightness:
      lcd.setCursor(0, 0);
      lcd.print("Matrix Brightness");
      matrixBrightness = constrain(matrixBrightness, 0, 15);
      setMatrixBrightness();
      lcd.setCursor(0, 1);
      for (int i = 0; i < lcdColumns; i++) {
        if (i < matrixBrightness) {
          lcd.write((byte)progressBarCharacterIndex);
        } else {
          lcd.print(" ");
        }
      }
      break;
    case menuHowToPlay:
      lcd.setCursor(2, 0);
      lcd.print(" How to Play? ");
      displayScrollingText("Collect tokens while avoiding bombs. Be faster than your friend!");
      break;
    case menuAbout:
      lcd.setCursor(2, 0);
      lcd.print(" About game ");
      displayScrollingText("We got a Space Problem! , Author: Sima Mihai, Github: LetMeCode01   ");
      break;
    case menuChangeDifficulty:
      displayChangeDifficulty();
      break;
    case menuSounds:
      displaySoundsOption();
      break;
    case menuHighscore:
      lcd.setCursor(1, 0);
      lcd.print(" Leaderboard ");
      displayHighscore();
      break;
    case menuResetHighscore:
      lcd.setCursor(0, 0);
      lcd.print("Scores reseted!");
      break;
  }
}

// Handle menu left navigation.
void changeMenuOptionLeft() {
  if (gameSounds == soundsOn) tone(buzzerPin, moveToneHz, 60);
  switch (subMenuOption) {
    case noSubMenuOption:
      switch (showMenuOption) {
        case menuStartGame: showMenuOption = menuAbout; break;
        case menuHighscore: showMenuOption = menuStartGame; break;
        case menuResetHighscore: showMenuOption = menuHighscore; break;
        case menuLcdBrightness: showMenuOption = menuResetHighscore; break;
        case menuMatrixBrightness: showMenuOption = menuLcdBrightness; break;
        case menuChangeDifficulty: showMenuOption = menuMatrixBrightness; break;
        case menuSounds: showMenuOption = menuChangeDifficulty; break;
        case menuHowToPlay: showMenuOption = menuSounds; break;
        case menuAbout: showMenuOption = menuHowToPlay; break;
      }
      break;
    case menuLcdBrightness:
      if (lcdBrightness > 1) lcdBrightness--;
      break;
    case menuMatrixBrightness:
      if (matrixBrightness > 0) matrixBrightness--;
      break;
    case menuChangeDifficulty:
      settingsGameDifficulty = (settingsGameDifficulty == easyDifficulty) ? hardDifficulty : (GameDifficulty)(settingsGameDifficulty - 1);
      break;
  }
}

// Handle menu right navigation.
void changeMenuOptionRight() {
  if (gameSounds == soundsOn) tone(buzzerPin, moveToneHz, 60);
  switch (subMenuOption) {
    case noSubMenuOption:
      switch (showMenuOption) {
        case menuStartGame: showMenuOption = menuHighscore; break;
        case menuHighscore: showMenuOption = menuResetHighscore; break;
        case menuResetHighscore: showMenuOption = menuLcdBrightness; break;
        case menuLcdBrightness: showMenuOption = menuMatrixBrightness; break;
        case menuMatrixBrightness: showMenuOption = menuChangeDifficulty; break;
        case menuChangeDifficulty: showMenuOption = menuSounds; break;
        case menuSounds: showMenuOption = menuHowToPlay; break;
        case menuHowToPlay: showMenuOption = menuAbout; break;
        case menuAbout: showMenuOption = menuStartGame; break;
      }
      break;
    case menuLcdBrightness:
      if (lcdBrightness < lcdColumns) lcdBrightness++;
      break;
    case menuMatrixBrightness:
      if (matrixBrightness < 15) matrixBrightness++;
      break;
    case menuChangeDifficulty:
      settingsGameDifficulty = (settingsGameDifficulty == hardDifficulty) ? easyDifficulty : (GameDifficulty)(settingsGameDifficulty + 1);
      break;
    case menuEnterName:
      commitUsernameChar();
      break;
  }
}

// Read menu joystick and update selections.
void readMenuJoystick() {
  int activeXPin = xPin2;
  int activeYPin = yPin2;

  if (subMenuOption == menuEnterName && winnerIndex == 0) {
    activeXPin = xPin;
    activeYPin = yPin;
  }

  bool invertMenuDirections = (activeXPin == xPin2 && activeYPin == yPin2);

  int joyXVal = analogRead(activeXPin);
  int joyYVal = analogRead(activeYPin);

  if (joyXVal < menuMinThreshold && !joyMovedMenuX) {
    joyMovedMenuX = true;
    if (invertMenuDirections) {
      changeMenuOptionRight();
    } else {
      changeMenuOptionLeft();
    }
  }
  if (joyXVal > menuMaxThreshold && !joyMovedMenuX) {
    joyMovedMenuX = true;
    if (invertMenuDirections) {
      changeMenuOptionLeft();
    } else {
      changeMenuOptionRight();
    }
  }
  if (joyXVal >= menuMinThreshold && joyXVal <= menuMaxThreshold) {
    joyMovedMenuX = false;
  }

  if (joyYVal < menuMinThreshold && !joyMovedMenuY) {
    joyMovedMenuY = true;
    if (subMenuOption == menuEnterName) {
      if (invertMenuDirections) {
        changeCharDown();
      } else {
        changeCharUp();
      }
    } else if (subMenuOption == menuChangeDifficulty) {
      if (invertMenuDirections) {
        settingsGameDifficulty = (settingsGameDifficulty == hardDifficulty) ? easyDifficulty : (GameDifficulty)(settingsGameDifficulty + 1);
      } else {
        settingsGameDifficulty = (settingsGameDifficulty == easyDifficulty) ? hardDifficulty : (GameDifficulty)(settingsGameDifficulty - 1);
      }
    } else if (subMenuOption == menuSounds) {
      settingsGameSounds = (settingsGameSounds == soundsOn) ? soundsOff : soundsOn;
    }
  }
  if (joyYVal > menuMaxThreshold && !joyMovedMenuY) {
    joyMovedMenuY = true;
    if (subMenuOption == menuEnterName) {
      if (invertMenuDirections) {
        changeCharUp();
      } else {
        changeCharDown();
      }
    } else if (subMenuOption == menuChangeDifficulty) {
      if (invertMenuDirections) {
        settingsGameDifficulty = (settingsGameDifficulty == easyDifficulty) ? hardDifficulty : (GameDifficulty)(settingsGameDifficulty - 1);
      } else {
        settingsGameDifficulty = (settingsGameDifficulty == hardDifficulty) ? easyDifficulty : (GameDifficulty)(settingsGameDifficulty + 1);
      }
    } else if (subMenuOption == menuSounds) {
      settingsGameSounds = (settingsGameSounds == soundsOn) ? soundsOff : soundsOn;
    }
  }
  if (joyYVal >= menuMinThreshold && joyYVal <= menuMaxThreshold) {
    joyMovedMenuY = false;
  }
}

// Save winner score and username to EEPROM.
void writeScoreToEeprom() {
  switch (leaderBoardPosition) {
    case thirdPlace:
      EEPROM.put(thirdPlaceScoreBeginAddress, winnerScore);
      for (int i = 0; i <= usernameLength; i++) {
        EEPROM.put(thirdPlaceUsernameBeginAddress + i, userName[i]);
        thirdPlaceUsername[i] = userName[i];
      }
      thirdPlaceScore = winnerScore;
      break;
    case secondPlace:
      EEPROM.put(thirdPlaceScoreBeginAddress, secondPlaceScore);
      for (int i = 0; i <= usernameLength; i++) {
        EEPROM.put(thirdPlaceUsernameBeginAddress + i, secondPlaceUsername[i]);
        thirdPlaceUsername[i] = secondPlaceUsername[i];
      }
      thirdPlaceScore = secondPlaceScore;
      EEPROM.put(secondPlaceScoreBeginAddress, winnerScore);
      for (int i = 0; i <= usernameLength; i++) {
        EEPROM.put(secondPlaceUsernameBeginAddress + i, userName[i]);
        secondPlaceUsername[i] = userName[i];
      }
      secondPlaceScore = winnerScore;
      break;
    case firstPlace:
      EEPROM.put(thirdPlaceScoreBeginAddress, secondPlaceScore);
      for (int i = 0; i <= usernameLength; i++) {
        EEPROM.put(thirdPlaceUsernameBeginAddress + i, secondPlaceUsername[i]);
        thirdPlaceUsername[i] = secondPlaceUsername[i];
      }
      thirdPlaceScore = secondPlaceScore;
      EEPROM.put(secondPlaceScoreBeginAddress, firstPlaceScore);
      for (int i = 0; i <= usernameLength; i++) {
        EEPROM.put(secondPlaceUsernameBeginAddress + i, firstPlaceUsername[i]);
        secondPlaceUsername[i] = firstPlaceUsername[i];
      }
      secondPlaceScore = firstPlaceScore;
      EEPROM.put(firstPlaceScoreBeginAddress, winnerScore);
      for (int i = 0; i <= usernameLength; i++) {
        EEPROM.put(firstPlaceUsernameBeginAddress + i, userName[i]);
        firstPlaceUsername[i] = userName[i];
      }
      firstPlaceScore = winnerScore;
      break;
  }
}

// Evaluate if score qualifies for leaderboard.
void evaluateHighscore(int scoreValue) {
  leaderBoardPosition = noLeaderBoardPosition;
  if (scoreValue > thirdPlaceScore) {
    if (scoreValue > secondPlaceScore) {
      if (scoreValue > firstPlaceScore) {
        leaderBoardPosition = firstPlace;
      } else {
        leaderBoardPosition = secondPlace;
      }
    } else {
      leaderBoardPosition = thirdPlace;
    }
  }
}

// Handle menu button actions.
void handleMenuButtonPress() {
  switch (subMenuOption) {
    case noSubMenuOption:
      switch (showMenuOption) {
        case menuStartGame:
          gameState = inGameState;
          resetGame();
          lcd.clear();
          break;
        case menuHighscore:
          subMenuOption = menuHighscore;
          scrollingTextPos = 0;
          lcd.clear();
          break;
        case menuResetHighscore:
          subMenuOption = menuResetHighscore;
          resetHighscores();
          lcd.clear();
          break;
        case menuLcdBrightness:
          subMenuOption = menuLcdBrightness;
          lcd.clear();
          break;
        case menuMatrixBrightness:
          subMenuOption = menuMatrixBrightness;
          lcd.clear();
          break;
        case menuHowToPlay:
          subMenuOption = menuHowToPlay;
          scrollingTextPos = 0;
          lcd.clear();
          break;
        case menuAbout:
          subMenuOption = menuAbout;
          scrollingTextPos = 0;
          lcd.clear();
          break;
        case menuChangeDifficulty:
          subMenuOption = menuChangeDifficulty;
          settingsGameDifficulty = gameDifficulty;
          lcd.clear();
          break;
        case menuSounds:
          subMenuOption = menuSounds;
          settingsGameSounds = gameSounds;
          lcd.clear();
          break;
      }
      break;
    case menuLcdBrightness:
      EEPROM.update(lcdBrightnessEepromAddress, lcdBrightness);
      subMenuOption = noSubMenuOption;
      lcd.clear();
      break;
    case menuMatrixBrightness:
      EEPROM.update(matrixBrightnessEepromAddress, matrixBrightness);
      subMenuOption = noSubMenuOption;
      lcd.clear();
      break;
    case menuChangeDifficulty:
      gameDifficulty = settingsGameDifficulty;
      applyDifficultySettings();
      writeDifficultyToEeprom();
      subMenuOption = noSubMenuOption;
      lcd.clear();
      break;
    case menuSounds:
      gameSounds = settingsGameSounds;
      writeGameSoundsToEeprom();
      subMenuOption = noSubMenuOption;
      lcd.clear();
      break;
    case menuEnterName:
      writeScoreToEeprom();
      readGameVariablesFromEEPROM();
      currentUsernamePosition = 0;
      userName = "";
      currentUsernameChar = firstAlphabetChar;
      subMenuOption = noSubMenuOption;
      showMenuOption = menuStartGame;
      lcd.clear();
      break;
    case menuHighscore:
    case menuResetHighscore:
    case menuHowToPlay:
    case menuAbout:
      subMenuOption = noSubMenuOption;
      lcd.clear();
      break;
  }
}

// -- LCD STATUS --
// Convert zone index to label.
const char* getZoneLabel(int zone) {
  switch (zone) {
    case 0: return "NW";
    case 1: return "NE";
    case 2: return "SW";
    case 3: return "SE";
    default: return "??";
  }
}

// Compute remaining food for a player in zone.
int getRemainingFoodForPlayer(int playerIndex, int zone) {
  int remaining = foodsPerPlayerTarget - playerZoneFood[playerIndex][zone];
  return (remaining < 0) ? 0 : remaining;
}

// Build one LCD status line for a player.
void buildStatusLine(int playerIndex, int currentZone, char* buffer, int bufferSize) {
  const char* zoneLabel = getZoneLabel(currentZone);
  snprintf(
    buffer,
    bufferSize,
    "P%d: Z:%s NW:%d NE:%d SW:%d SE:%d HIT:%d",
    playerIndex + 1,
    zoneLabel,
    getRemainingFoodForPlayer(playerIndex, 0),
    getRemainingFoodForPlayer(playerIndex, 1),
    getRemainingFoodForPlayer(playerIndex, 2),
    getRemainingFoodForPlayer(playerIndex, 3),
    playerBombHits[playerIndex]
  );
}

// Update scroll offset for one line.
void updateScrollOffset(int textLength, int& offset, int& direction) {
  if (textLength <= lcdColumns) {
    offset = 0;
    direction = 1;
    return;
  }

  int maxOffset = textLength - lcdColumns;
  if (offset >= maxOffset) {
    direction = -1;
  } else if (offset <= 0) {
    direction = 1;
  }

  offset += direction;
  if (offset < 0) offset = 0;
  if (offset > maxOffset) offset = maxOffset;
}

// Render a scrolling window to LCD.
void renderScrollLine(int row, const char* text, int textLength, int offset) {
  char window[lcdColumns + 1];
  for (int i = 0; i < lcdColumns; i++) {
    int idx = offset + i;
    window[i] = (idx < textLength) ? text[idx] : ' ';
  }
  window[lcdColumns] = '\0';
  lcd.setCursor(0, row);
  lcd.print(window);
}

// Update scrolling LCD status for both players.
void updateLcdStatus() {
  int zoneP1 = getZoneIndex(playerX, playerY);
  int zoneP2 = getZoneIndex(player2X, player2Y);

  buildStatusLine(0, zoneP1, lcdLine1, sizeof(lcdLine1));
  buildStatusLine(1, zoneP2, lcdLine2, sizeof(lcdLine2));

  int len1 = strlen(lcdLine1);
  int len2 = strlen(lcdLine2);

  unsigned long now = millis();
  if (now - lastLcdScroll >= lcdScrollInterval) {
    lastLcdScroll = now;
    updateScrollOffset(len1, lcdOffset1, lcdDir1);
    updateScrollOffset(len2, lcdOffset2, lcdDir2);
  }

  renderScrollLine(0, lcdLine1, len1, lcdOffset1);
  renderScrollLine(1, lcdLine2, len2, lcdOffset2);
}

// -- SCORING / ENDGAME --
// Compute score for a player.
int getPlayerScore(int playerIndex) {
  int opponentIndex = (playerIndex == 0) ? 1 : 0;
  int base = 12 + (maxBombHits - playerBombHits[playerIndex]) + playerBombHits[opponentIndex];
  return base * 100;
}

// Check if a player cleared all zones.
bool isPlayerCompletedAll(int playerIndex) {
  for (int i = 0; i < zoneCount; i++) {
    if (!playerZoneCompleted[playerIndex][i]) {
      return false;
    }
  }
  return true;
}

// Render a letter on one matrix.
void renderLetter(LedControl& target, const byte* letter) {
  for (int y = 0; y < matrixSize; y++) {
    target.setRow(0, y, letter[y]);
  }
}

// Draw endgame screens and scores.
void drawEndScreens() {
  if (winnerIndex == 0) {
    renderLetter(lc, letterW);
    renderLetter(lc2, letterL);
  } else {
    renderLetter(lc, letterL);
    renderLetter(lc2, letterW);
  }

  lcd.setCursor(0, 0);
  if (winnerIndex == 0) {
    lcd.print("DONE! Score:");
  } else {
    lcd.print("LOSER");
    lcd.print("             ");
  }

  lcd.setCursor(0, 1);
  if (winnerIndex == 1) {
    lcd.print("DONE! Score:");
  } else {
    lcd.print("LOSER");
    lcd.print("             ");
  }

  if (winnerIndex == 0) {
    lcd.setCursor(lcdScoreColumn, 0);
    lcd.print(getPlayerScore(0));
  } else {
    lcd.setCursor(lcdScoreColumn, 1);
    lcd.print(getPlayerScore(1));
  }
}

// Reset all game state for a new round.
void resetGame() {
  playerX = 1;
  playerY = 14;
  player2X = 14;
  player2Y = 14;
  playerVisible = true;
  player2Visible = true;
  lastBlink = 0;
  lastBlink2 = 0;

  for (int i = 0; i < zoneCount; i++) {
    zoneFoodRemaining[i] = foodsPerZone;
    foodActive[i] = false;
    foodVisible[i] = false;
    foodRespawnTime[i] = 0;
    playerZoneFood[0][i] = 0;
    playerZoneFood[1][i] = 0;
    playerZoneCompleted[0][i] = false;
    playerZoneCompleted[1][i] = false;
  }

  totalFoodCollected1 = 0;
  totalFoodCollected2 = 0;

  bombActive = false;
  bombVisible = false;
  bombX = 0;
  bombY = 0;
  bombZone = -1;
  bombOwner = -1;
  bombPlanned = false;
  bombPlannedZone = -1;
  bombSpawnTime = 0;
  bombDetonateTime = 0;
  lastBombTick = 0;

  trapActive = false;
  ldrAtTrap = 0;
  lastTrapTime = 0;
  trapActive2 = false;
  ldrAtTrap2 = 0;
  lastTrapTime2 = 0;

  playerBombHits[0] = 0;
  playerBombHits[1] = 0;

  gameOver = false;
  winnerIndex = -1;
  winnerScore = 0;
  leaderBoardPosition = noLeaderBoardPosition;

  for (int i = 0; i < playerCount; i++) {
    redBlinkActive[i] = false;
    redBlinkOn[i] = false;
    redBlinkStart[i] = 0;
    redBlinkLastToggle[i] = 0;
  }

  lcdOffset1 = 0;
  lcdOffset2 = 0;
  lcdDir1 = 1;
  lcdDir2 = 1;
  lastLcdScroll = 0;

  lc.clearDisplay(0);
  lc2.clearDisplay(0);
  updateLcdStatus();
}

// Main game loop.
void loop() {
  unsigned long now = millis();

  updateTrafficLights(now);

  bool resetReading = digitalRead(resetButtonPin);
  if (resetReading != resetButtonLastState) {
    lastResetDebounce = now;
    resetButtonLastState = resetReading;
  }

  bool resetPressedEdge = false;
  if (now - lastResetDebounce > resetDebounceMs && resetReading != resetButtonState) {
    resetPressedEdge = (resetButtonState == HIGH && resetReading == LOW);
    resetButtonState = resetReading;
  }

  if (!menuButtonReady && resetReading == HIGH) {
    menuButtonReady = true;
  }

  if (gameState == menuState) {
    displayMenuOptions();
    readMenuJoystick();
    if (menuButtonReady && resetPressedEdge) {
      handleMenuButtonPress();
    }
    return;
  }

  if (gameState == endGameState) {
    drawEndScreens();
    if (menuButtonReady && resetPressedEdge) {
      if (leaderBoardPosition != noLeaderBoardPosition) {
        subMenuOption = menuEnterName;
        gameState = menuState;
        lcd.clear();
      } else {
        subMenuOption = noSubMenuOption;
        showMenuOption = menuStartGame;
        gameState = menuState;
        lcd.clear();
      }
    }
    return;
  }

  if (now - lastBlink > playerBlinkIntervalMs) {
    playerVisible = !playerVisible;
    lastBlink = now;
  }

  if (now - lastBlink2 > playerBlinkIntervalMs) {
    player2Visible = !player2Visible;
    lastBlink2 = now;
  }

  blinkFood();
  spawnFood();
  collectFood();
  updateBomb();

  // My joystick 1 is a little faulty and gives noisy readings, so I average multiple samples to get more stable input.
  long xSum = 0;
  long ySum = 0;
  for (int i = 0; i < joystick1Samples; i++) {
    xSum += analogRead(xPin);
    ySum += analogRead(yPin);
  }
  int xVal = xSum / joystick1Samples;
  int yVal = ySum / joystick1Samples;
  int dx = xVal - joystick1CenterX;
  int dy = yVal - joystick1CenterY;
  bool moveX = (abs(dx) > joystick1Deadzone) && (abs(dx) > abs(dy) + joystick1DominantMargin);
  bool moveY = (abs(dy) > joystick1Deadzone) && (abs(dy) > abs(dx) + joystick1DominantMargin);

  int xVal2 = analogRead(xPin2);
  int yVal2 = analogRead(yPin2);
  bool moveX2 = (xVal2 < minThreshold) || (xVal2 > maxThreshold);
  bool moveY2 = (yVal2 < minThreshold) || (yVal2 > maxThreshold);

  if (!moveX && !moveY) {
    joystickMoved = false;
  }

  if (!moveX2 && !moveY2) {
    joystickMoved2 = false;
  }

  if (!trapActive) {
    if ((moveX || moveY) && !joystickMoved && (now - lastMoved > moveInterval)) {
      int newX = playerX;
      int newY = playerY;

      if (moveX) {
        if (dx < 0) newX--;
        if (dx > 0) newX++;
      } else if (moveY) {
        if (dy < 0) newY++;
        if (dy > 0) newY--;
      }

      if (newX >= 0 && newX < mapSize && newY >= 0 && newY < mapSize) {
        int targetZone = getZoneIndex(newX, newY);
        if (playerZoneCompleted[0][targetZone] && isFoodAt(newX, newY, targetZone)) {
          playBuzzerWall();
        } else if (!canMoveWithBomb(playerX, playerY, newX, newY)) {
          playBuzzerWall();
        } else if (newX == player2X && newY == player2Y) {
          playBuzzerWall();
        } else if (gameMap[newY][newX] == 0 && canMoveFromZone(newX, newY)) {
          playerX = newX;
          playerY = newY;
          playBuzzerMove();

          if (!isOnZoneEdge() && now - lastTrapTime > trapCooldownMs) {
            if (random(0, trapChance) == 0) {
              trapActive = true;
              ldrAtTrap = analogRead(ldrPin);
              playBuzzerLdr();
              lastTrapTime = now;
            }
          }
        } else if (gameMap[newY][newX] == 1) {
          playBuzzerWall();
        }
      }
      joystickMoved = true;
      lastMoved = now;
    }
  }

  if (!trapActive2 && (moveX2 || moveY2) && !joystickMoved2 && (now - lastMoved2 > moveInterval)) {
    int newX2 = player2X;
    int newY2 = player2Y;

    if (moveX2) {
      if (xVal2 < minThreshold) newX2++;
      if (xVal2 > maxThreshold) newX2--;
    } else if (moveY2) {
      if (yVal2 < minThreshold) newY2--;
      if (yVal2 > maxThreshold) newY2++;
    }

    if (newX2 >= 0 && newX2 < mapSize && newY2 >= 0 && newY2 < mapSize) {
      int targetZone2 = getZoneIndex(newX2, newY2);
      if (playerZoneCompleted[1][targetZone2] && isFoodAt(newX2, newY2, targetZone2)) {
        playBuzzerWall2();
      } else if (!canMoveWithBomb(player2X, player2Y, newX2, newY2)) {
        playBuzzerWall2();
      } else if (newX2 == playerX && newY2 == playerY) {
        playBuzzerWall2();
      } else if (gameMap[newY2][newX2] == 0) {
        player2X = newX2;
        player2Y = newY2;
        playBuzzerMove2();

        if (!isOnZoneEdge() && now - lastTrapTime2 > trapCooldownMs) {
          if (random(0, trapChance) == 0) {
            trapActive2 = true;
            ldrAtTrap2 = analogRead(ldrPin2);
            playBuzzerLdr2();
            lastTrapTime2 = now;
          }
        }
      } else if (gameMap[newY2][newX2] == 1) {
        playBuzzerWall2();
      }
    }

    joystickMoved2 = true;
    lastMoved2 = now;
  }

  // Handle LDR Trap - requires light change to escape
  if (trapActive) {
    int ldrNow = analogRead(ldrPin);
    if (abs(ldrNow - ldrAtTrap) >= ldrEscapeDelta) {
      trapActive = false;
      joystickMoved = false;
      lastMoved = now;
      playBuzzerLdr();
    }
  }

  if (trapActive2) {
    int ldrNow2 = analogRead(ldrPin2);
    if (abs(ldrNow2 - ldrAtTrap2) >= ldrEscapeDelta) {
      trapActive2 = false;
      joystickMoved2 = false;
      lastMoved2 = now;
      playBuzzerLdr2();
    }
  }

  drawZone();
  drawZonePlayer2();
}
