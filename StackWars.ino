// STACKWARS GAME
#include <avr/wdt.h>
#include "pitches.h"
#include <LedControl.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define NUM_DEVICES 4

// Player 1 pins
#define P1_DATA_PIN 11
#define P1_CLK_PIN  13
#define P1_CS_PIN   5
#define P1_BUTTON   2

// Player 2 pins
#define P2_DATA_PIN 10
#define P2_CLK_PIN  12
#define P2_CS_PIN   4
#define P2_BUTTON   3

#define SPECIAL_BUTTON 6

#define BUZZER_PIN 7

LiquidCrystal_I2C lcd(0x27, 16, 2);

bool running_P1 = false;
int y_P1, yStep_P1 = 1;
bool down_P1 = true;
int lastY_P1 = 0;
unsigned long prevMillis_P1 = 0;
int dynamicDelay_P1 = 150; // Set a default
int lowestY_P1 = 0;
bool running_P2 = false;
int y_P2, yStep_P2 = 1;
bool down_P2 = true;
int lastY_P2 = 0;
unsigned long prevMillis_P2 = 0;
int dynamicDelay_P2 = 150;
int lowestY_P2 = 0;

// Separate LedControl objects for each player
LedControl lc1 = LedControl(P1_DATA_PIN, P1_CLK_PIN, P1_CS_PIN, NUM_DEVICES);
LedControl lc2 = LedControl(P2_DATA_PIN, P2_CLK_PIN, P2_CS_PIN, NUM_DEVICES);

// Game variables
int DELAY_MS = 400, game_level = 0, high_score = 0;
bool game_on = false, sign = true;

bool pendingGameOver_P1 = false;
bool pendingGameOver_P2 = false;

int brightness = 0; //0 lowest 15 highest

bool buttonPressed = false;
volatile bool buttonPressed_P1 = false;
volatile bool buttonPressed_P2 = false;

// Player 1 variables
int stopping_Y_P1 = 9, reference_Y_P1 = 9, first_time_P1 = 0, x_pos_P1 = 0,
    score_P1 = 0, blc_num_P1 = 0, x_P1 = 0, size_P1 = 4;

// Player 2 variables
int stopping_Y_P2 = 9, reference_Y_P2 = 9, first_time_P2 = 0, x_pos_P2 = 0,
    score_P2 = 0, blc_num_P2 = 0, x_P2 = 0, size_P2 = 4;

// MILLIS VARIABLES
unsigned long previousTimeInterruptP1 = 0;
unsigned long previousTimeInterruptP2 = 0;

void buttonPressed_check_P1() {
  unsigned long currentTime = millis();
  if (currentTime - previousTimeInterruptP1 > 250) {
    buttonPressed_P1 = true;
    previousTimeInterruptP1 = currentTime;
  }
}
void buttonPressed_check_P2() {
  unsigned long currentTime = millis();
  if (currentTime - previousTimeInterruptP2 > 250) {
    buttonPressed_P2 = true;
    previousTimeInterruptP2 = currentTime;
  }
}

int loseStarwarsNotes[] = {
  NOTE_A4, NOTE_A4, NOTE_F4, NOTE_C5,
  NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4,
  
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_F5, NOTE_C5, NOTE_GS4, NOTE_A4,
  
  NOTE_F4, NOTE_C5, NOTE_A4,
};

int loseStarwarsDurations[] = {
  500, 500, 350, 150,
  500, 350, 150, 650,
  
  500, 500, 500,
  350, 150, 500, 350,
  
  150, 650, 500
};

int winStarwarsNotes[] = {
  NOTE_C5, NOTE_G4, NOTE_A4,
  NOTE_F4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_C5,

  NOTE_G4, NOTE_E5, NOTE_F5,
  NOTE_D5, NOTE_B4, NOTE_C5, NOTE_A4,

  NOTE_G4, NOTE_C5, NOTE_G5
};

int winStarwarsDurations[] = {
  400, 200, 400,
  300, 300, 300, 300, 500,

  300, 300, 400,
  300, 300, 300, 300,

  300, 600, 700
};

int loseNotes[] = {NOTE_A4, NOTE_G4, NOTE_F4, NOTE_E4};
int loseDurations[] = {200, 200, 200, 400};

int winNotes[] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_G5, NOTE_A5};
int winDurations[] = {150, 150, 150, 150, 400};

int levelSelectNotes[] = { 400, 500, 600 };
int levelSelectDurations[] = { 100, 100, 100 };

void setup() {
  // Welcome screen
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Welcome to the");
  lcd.setCursor(3, 1);
  lcd.print("Stack Wars!");
  delay(2000);  // Show welcome message for 3 seconds
  
// Setup buttons
  pinMode(SPECIAL_BUTTON, INPUT_PULLUP);

  pinMode(P1_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(P1_BUTTON), buttonPressed_check_P1, FALLING);
  
  pinMode(P2_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(P2_BUTTON), buttonPressed_check_P2, FALLING);

  for (int y = 0; y < NUM_DEVICES; y++) {
    lc1.shutdown(y, false);
    lc1.setIntensity(y, brightness);
    lc1.clearDisplay(y);
    lc2.shutdown(y, false);
    lc2.setIntensity(y, brightness);
    lc2.clearDisplay(y);
  }

  lcd.clear();  // Clear welcome before game begins
  Serial.begin(9600);
}

void loop() {
  if (!game_on) {
    set_level();  // This is blocking until level selected, sets game_on = true inside?
    game_on = true;  // After level is selected, set game_on to true to start game
  }

  if (game_on) {
    updateGame_P1();
    updateGame_P2();
  }
  updateMusic();
  handleGameOverMusic();
  handleReset();
}

void set_level() {
  int i = 0;
  const int max_i = 32;
  unsigned long previousMillis = 0;
  const unsigned long interval = 1000;  // 1 second per animation step
  bool levelSelected = false;
  
  Serial.println("Starting level selection");
  lcd.clear();
  clearAll();  // clear once before starting animation
  playMusic(4); // intro or lose tune

  while (!levelSelected) {
    updateMusic();
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      if (i >= max_i) {
        // After showing all 8 levels, clear all and restart cycle
        clearAll();
        i = 0;
      }

      int current_level = (i / 4) + 1;

      // Draw animation for current level (like lighting columns)
      for (int j = 1; j < 7; j++) {
        for (int k = 0; k < 3; k++) {
          lc1.setLed(i / 8, j, 7 - ((i + k) % 8), true);
          lc2.setLed(i / 8, j, 7 - ((i + k) % 8), true);
        }
      }

      // Update LCD only once per step
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Select Level");
      lcd.setCursor(5, 1);
      lcd.print("LVL: ");
      lcd.print(current_level);

      i += 4;
    }

    // Check button press anytime
    if (digitalRead(SPECIAL_BUTTON) == LOW) {
      delay(250);  // debounce

      int current_level = ((i - 4) / 4) + 1;  // Use previous step's level, because i was incremented already
      if (current_level < 1) current_level = 1;  // clamp
      if (current_level > 8) current_level = 8;

      game_level = current_level;
      DELAY_MS = 450 - game_level * 50;

      clearAll();
      updateLCD();

      buttonPressed_P1 = false;
      buttonPressed_P2 = false;

      levelSelected = true;
      playMusic(4); // intro or lose tune
    }
  }
}

int state_P1 = 0;
bool gameOver_P1 = false;
bool gameOver_P2 = false;

  void updateGame_P1() {
    if (gameOver_P1) return;  // ⛔ Skip updates if Player 1 already lost
    switch (state_P1) {
      case 0:
        coordinate_update_P1();  // Make sure to update X before run
        startRunArray_P1();    // Start moving the block
        state_P1 = 1;
        break;

      case 1:
        if (updateRunArray_P1()) {  // Block has stopped moving

          if (first_time_P1 == 0) { 
            reference_Y_P1 = lowestY_P1;  // First block, set reference
            first_time_P1 = 1;
            stopping_Y_P1 = reference_Y_P1;
            keepon_P1();                 // Keep the block on screen
            playMusic(5);               // Success tune
          } else {
            stopping_Y_P1 = lowestY_P1;  // Get new block position
            check_alignment_P1();       // Check against reference_Y_P1
            if (!gameOver_P1) {
              keepon_P1();              // ✅ Only keep block if still in game
            }
          }
        if (!gameOver_P1) {
          score_P1 += size_P1;
          updateLCD();
        }   
          

          if (x_pos_P1 >= 32) win_reset();  // Win condition (full stack height)

          state_P1 = 0;  // Prepare for next block
        }
        break;
    }
  }

int state_P2 = 0;

void updateGame_P2() {
  if (gameOver_P2) return;  // ⛔ Skip updates if Player 2 already lost
  switch (state_P2) {
    case 0:
      coordinate_update_P2();  // Make sure to update X before run
      startRunArray_P2();      // Start moving the block
      state_P2 = 1;
      break;

    case 1:
      if (updateRunArray_P2()) {  // Block has stopped moving

        if (first_time_P2 == 0) { 
          Serial.println(first_time_P2);
          reference_Y_P2 = lowestY_P2;   // First block, set reference
          first_time_P2 = 1;
          stopping_Y_P2 = reference_Y_P2;
          keepon_P2();                  // Keep the block on screen
          playMusic(5);                // Success tune
        } else {
          stopping_Y_P2 = lowestY_P2;   // Get new block position
          check_alignment_P2();        // Check against reference_Y_P2
          if (!gameOver_P2) {
            keepon_P2();               // ✅ Only keep block if still in game
          }
        }

        if (!gameOver_P2 && first_time_P2 != 0 ) {
          Serial.println(first_time_P2);
          score_P2 += size_P2;
          updateLCD();
        }

        if (x_pos_P2 >= 32) win_reset();  // Win condition (full stack height)

        state_P2 = 0;  // Prepare for next block
      }
      break;
  }
}

void coordinate_update_P1() {
  Serial.println("Checking Coordinates for P1");
  x_P1 = 7 - (x_pos_P1 % 8);
  Serial.println("PLAYER 1 X POSITION: " + String(x_P1));
  blc_num_P1 = x_pos_P1 / 8;
  x_pos_P1 += 2;
  buttonPressed_P1 = false;
}

void coordinate_update_P2() {
  Serial.println("Checking Coordinates for P2");
  x_P2 = 7 - (x_pos_P2 % 8);
  Serial.println("PLAYER 2 X POSITION: " + String(x_P2));
  blc_num_P2 = x_pos_P2 / 8;
  x_pos_P2 += 2;
  buttonPressed_P2 = false;
}

int getDynamicDelay__P1() {
  int baseDelay = 400 - game_level * 50;
  if (baseDelay < 100) baseDelay = 100;

  float decayFactor = 0.95;
  int delayValue = (int)(baseDelay * pow(decayFactor, x_pos_P1));
  return (delayValue < 50) ? 50 : delayValue;
}

int getDynamicDelay_P2() {
  int baseDelay = 400 - game_level * 50;
  if (baseDelay < 100) baseDelay = 100;

  float decayFactor = 0.95;
  int delayValue = (int)(baseDelay * pow(decayFactor, x_pos_P2));
  return (delayValue < 50) ? 50 : delayValue;
}

void startRunArray_P1() {
  y_P1 = random(8 - size_P1);
  yStep_P1 = 1;
  down_P1 = true;
  lastY_P1 = y_P1;
  prevMillis_P1 = millis();
  dynamicDelay_P1 = getDynamicDelay__P1();
  running_P1 = true;
}

bool updateRunArray_P1() {
  if (!running_P1) return false;

  unsigned long currentMillis = millis();
  if (currentMillis - prevMillis_P1 >= dynamicDelay_P1) {
    prevMillis_P1 = currentMillis;

      // Clear previous LEDs
      for (int i = 0; i < 8; i++) {
        if (x_P1 >= 0 && x_P1 < 8) lc1.setLed(blc_num_P1, i, x_P1, false);
        if (x_P1 - 1 >= 0 && x_P1 - 1 < 8) lc1.setLed(blc_num_P1, i, x_P1 - 1, false);
      } 

    lastY_P1 = y_P1;

      // Draw current block
      for (int i = y_P1; i < y_P1 + size_P1; i++) {
        if (i >= 0 && i < 8) {
          if (x_P1 >= 0) lc1.setLed(blc_num_P1, i, x_P1, true);
          if (x_P1 - 1 >= 0) lc1.setLed(blc_num_P1, i, x_P1 - 1, true);
        }
      }

      // Move to next y
      y_P1 += down_P1 ? yStep_P1 : -yStep_P1;
      if (y_P1 >= 7 - size_P1 + 1) {
        y_P1 = 7 - size_P1 + 1;
        down_P1 = false;
      } else if (y_P1 <= 0) {
        y_P1 = 0;
        down_P1 = true;
      }

    dynamicDelay_P1 = getDynamicDelay__P1();
  }

  // Check for stop
  if (buttonPressed_P1) {
    buttonPressed_P1 = false;
    lowestY_P1 = lastY_P1;
    running_P1 = false;
    return true;  // finished
  }

  return false; // still running
}

void startRunArray_P2() {
  y_P2 = random(8 - size_P2);
  yStep_P2 = 1;
  down_P2 = true;
  lastY_P2 = y_P2;
  prevMillis_P2 = millis();
  dynamicDelay_P2 = getDynamicDelay_P2();
  running_P2 = true;
}

bool updateRunArray_P2() {
  if (!running_P2) return false;

  unsigned long currentMillis = millis();
  if (currentMillis - prevMillis_P2 >= dynamicDelay_P2) {
    prevMillis_P2 = currentMillis;

    // Clear previous LEDs
    for (int i = 0; i < 8; i++) {
      if (x_P2 >= 0 && x_P2 < 8) lc2.setLed(blc_num_P2, i, x_P2, false);
      if (x_P2 - 1 >= 0 && x_P2 - 1 < 8) lc2.setLed(blc_num_P2, i, x_P2 - 1, false);
    }

    lastY_P2 = y_P2;

    // Draw current block
    for (int i = y_P2; i < y_P2 + size_P2; i++) {
      if (i >= 0 && i < 8) {
        if (x_P2 >= 0) lc2.setLed(blc_num_P2, i, x_P2, true);
        if (x_P2 - 1 >= 0) lc2.setLed(blc_num_P2, i, x_P2 - 1, true);
      }
    }

    // Move to next y
    y_P2 += down_P2 ? yStep_P2 : -yStep_P2;
    if (y_P2 >= 7 - size_P2 + 1) {
      y_P2 = 7 - size_P2 + 1;
      down_P2 = false;
    } else if (y_P2 <= 0) {
      y_P2 = 0;
      down_P2 = true;
    }

    dynamicDelay_P2 = getDynamicDelay_P2(); // Separate function or shared logic
  }

  // Check for stop
  if (buttonPressed_P2) {
    buttonPressed_P2 = false;
    lowestY_P2 = lastY_P2;
    running_P2 = false;
    return true;  // finished
  }

  return false; // still running
}


void keepon_P1() {
  Serial.println("Keeping stack for P1");
  for (int i = 0; i < 8; i++) {
    if (x_P1 >= 0) lc1.setLed(blc_num_P1, i, x_P1, false);
    if (x_P1 - 1 >= 0) lc1.setLed(blc_num_P1, i, x_P1 - 1, false);
  }

  for (int i = reference_Y_P1; i < reference_Y_P1 + size_P1; i++) {
    if (i >= 0 && i < 8) {
      if (x_P1 >= 0) lc1.setLed(blc_num_P1, i, x_P1, true);
      if (x_P1 - 1 >= 0) lc1.setLed(blc_num_P1, i, x_P1 - 1, true);
    }
  }
}

void keepon_P2() {
  Serial.println("Keeping stack for P2");
  for (int i = 0; i < 8; i++) {
    if (x_P2 >= 0) lc2.setLed(blc_num_P2, i, x_P2, false);
    if (x_P2 - 1 >= 0) lc2.setLed(blc_num_P2, i, x_P2 - 1, false);
  }

  for (int i = reference_Y_P2; i < reference_Y_P2 + size_P2; i++) {
    if (i >= 0 && i < 8) {
      if (x_P2 >= 0) lc2.setLed(blc_num_P2, i, x_P2, true);
      if (x_P2 - 1 >= 0) lc2.setLed(blc_num_P2, i, x_P2 - 1, true);
    }
  }
}

void check_alignment_P1() {
  Serial.println("Checking Alignment for P1");
  int diff = abs(reference_Y_P1 - stopping_Y_P1);

  if (diff == 0) {
    playMusic(5); // success blip
  } else if (diff < size_P1) {
    size_P1 -= diff;
    if (reference_Y_P1 < stopping_Y_P1) reference_Y_P1 = stopping_Y_P1;
    playMusic(5);
  } else {
    Serial.println("PLAYING LOSING SHORT SOUND (P1)");
    playMusic(0); // play losing music
    gameOver_P1 = true;
    pendingGameOver_P1 = true; // <-- delay game over until music ends
  }
}

void check_alignment_P2() {
  Serial.println("Checking Alignment for P2");
  int diff = abs(reference_Y_P2 - stopping_Y_P2);

  if (diff == 0) {
    playMusic(5); // success blip
  } else if (diff < size_P2) {
    size_P2 -= diff;
    if (reference_Y_P2 < stopping_Y_P2) reference_Y_P2 = stopping_Y_P2;
    playMusic(5);
  } else {
    Serial.println("PLAYING LOSING SHORT SOUND (P2)");
    playMusic(0); // play losing music
    gameOver_P2 = true;
    pendingGameOver_P2 = true; // <-- delay game over until music ends
  }
}

void checkBothGameOver() {
  if (gameOver_P1 && gameOver_P2) {
    Serial.println("Both players lost. Resetting...");
    lose_reset();
  }
}

void win_reset() {
  Serial.println("WIN");
  
  int winnerScore;
  String winnerName;
  
  if (score_P1 > score_P2) {
    winnerScore = score_P1;
    winnerName = "Player 1";
  } else if (score_P2 > score_P1) {
    winnerScore = score_P2;
    winnerName = "Player 2";
  } else {
    winnerScore = score_P1; // tie case, same score
    winnerName = "Tie!";
  }

  if (winnerScore > high_score) {
    high_score = winnerScore;
  }

  lcd.clear();
  lcd.setCursor(1, 0);
  if (winnerName == "Tie!") {
    lcd.print("It's a Tie!");
  } else {
    lcd.print(winnerName);
    lcd.print(" Wins!");
  }
  lcd.setCursor(4, 1);
  lcd.print("Score: ");
  lcd.print(winnerScore);

  playMusic(1); // Win music
  delay(1000);

  reset_var();

  playMusic(2); // song
  delay(1500);
  clearAll();

      // Show reset message on LCD
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Resetting Game");
  delay(1500);  // Short delay to let users read the message
  lcd.clear();
}

bool isResetting = false;
bool musicStarted = false;
unsigned long resetStartTime = 0;
bool isMusicPlaying = false;

void lose_reset() {
  if (isResetting) return;  // prevent repeated calls

  Serial.println("Lose");

  int winnerScore;
  String winnerName;

  if (score_P1 > score_P2) {
    winnerScore = score_P1;
    winnerName = "Player 1 won";
  } else if (score_P2 > score_P1) {
    winnerScore = score_P2;
    winnerName = "Player 2 won";
  } else {
    winnerScore = score_P1; // tie case
    winnerName = "Tie Game";
  }

  if (winnerScore > high_score) {
    high_score = winnerScore;
  }

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Game Over!");
  lcd.setCursor(2, 1);
  lcd.print(winnerName);

  playMusic(3);  // start music
  isResetting = true;
  musicStarted = true;
}

void handleReset() {
  if (!isResetting) return;

  // Wait for music to finish playing
  if (musicStarted && !isMusicPlaying) {
    musicStarted = false;
    clearAll();

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Resetting Game");
    resetStartTime = millis();
  }

  // Wait 1.5 seconds after music finished before resetting
  if (!musicStarted && (millis() - resetStartTime >= 1500)) {
    lcd.clear();
    reset_var();
    isResetting = false;  // Reset complete
  }
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P1:");
  lcd.print(score_P1);
  lcd.setCursor(11, 0);
  lcd.print("P2:");
  lcd.print(score_P2);
  
  lcd.setCursor(5,1);
  lcd.print("Lvl:");
  lcd.print(game_level);
}

void clearAll() {
  for (int i = 0; i < NUM_DEVICES; i++)
    lc1.clearDisplay(i);
  for (int i = 0; i < NUM_DEVICES; i++)
    lc2.clearDisplay(i);
}

int *currentMelody = nullptr;
int *currentDurations = nullptr;
int currentLength = 0;
int currentNoteIndex = 0;
unsigned long noteStartTime = 0;

void playMusic(int type) {
  
  if (type == 0) {
    currentMelody = loseNotes;
    currentDurations = loseDurations;
    currentLength = sizeof(loseNotes) / sizeof(int);
  } else if (type == 1) {
    currentMelody = winNotes;
    currentDurations = winDurations;
    currentLength = sizeof(winNotes) / sizeof(int);
  } else if (type == 2) {
    currentMelody = winStarwarsNotes;
    currentDurations = winStarwarsDurations;
    currentLength = sizeof(winStarwarsNotes) / sizeof(int);
  } else if (type == 3) {
    currentMelody = loseStarwarsNotes;
    currentDurations = loseStarwarsDurations;
    currentLength = sizeof(loseStarwarsNotes) / sizeof(int);
  } else if (type == 4) {
    currentMelody = levelSelectNotes;
    currentDurations = levelSelectDurations;
    currentLength = sizeof(levelSelectNotes) / sizeof(int);
  } else if (type == 5) {
    tone(BUZZER_PIN, 450, 100);
    // No delay — short beep only
    return;
  } else {
    return;
  }

  currentNoteIndex = 0;
  noteStartTime = millis();
  isMusicPlaying = true;

  if (currentMelody[0] != 0)
    tone(BUZZER_PIN, currentMelody[0], currentDurations[0]);
  else
    noTone(BUZZER_PIN);
}

const unsigned long gapBetweenNotes = 30;  // 30 ms gap between notes

void updateMusic() {
  if (!isMusicPlaying || currentNoteIndex >= currentLength) return;

  unsigned long elapsed = millis() - noteStartTime;

  // Wait for the current note duration plus a short gap before moving to next note
  if (elapsed >= currentDurations[currentNoteIndex] + gapBetweenNotes) {
    currentNoteIndex++;

    if (currentNoteIndex >= currentLength) {
      noTone(BUZZER_PIN);
      isMusicPlaying = false;
    } else {
      noTone(BUZZER_PIN); // stop previous tone before starting new one

      if (currentMelody[currentNoteIndex] != 0) {
        tone(BUZZER_PIN, currentMelody[currentNoteIndex], currentDurations[currentNoteIndex]);
      } else {
        noTone(BUZZER_PIN);
      }

      noteStartTime = millis();
    }
  }
}

void reset_var() {
  // Reset all game-related variables for Player 1
  x_pos_P1 = 0;
  first_time_P1 = 0;
  reference_Y_P1 = 9;
  stopping_Y_P1 = 9;
  size_P1 = 4;
  state_P1 = 0;

  // Reset all game-related variables for Player 2
  x_pos_P2 = 0;
  first_time_P2 = 0;
  reference_Y_P2 = 9;
  stopping_Y_P2 = 9;
  size_P2 = 4;
  state_P2 = 0;

  // Reset common variables
  sign = true;
  game_on = false;
  score_P1 = 0;
  score_P2 = 0;
  gameOver_P1 = false;
  gameOver_P2 = false;
}

void forceReset(){
    if (digitalRead(SPECIAL_BUTTON) == LOW) {
    Serial.println("Reset button pressed. Resetting...");
    delay(100);  // debounce delay
    wdt_enable(WDTO_15MS);  // Enable watchdog timer for 15ms reset
    while(1) {}  // Wait for watchdog to reset Arduino
  }
}

void handleGameOverMusic() {
  if (!isMusicPlaying) {
    if (pendingGameOver_P1) {
      pendingGameOver_P1 = false;
      Serial.println("Player 1 Lost!");
      checkBothGameOver();
    }

    if (pendingGameOver_P2) {
      pendingGameOver_P2 = false;
      Serial.println("Player 2 Lost!");
      checkBothGameOver();
    }
  }
}