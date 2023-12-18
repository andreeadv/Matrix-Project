#include <LiquidCrystal.h>

#include "LedControl.h" 

#include <EEPROM.h> 

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int buzzerPin = 3;

int lcdRows = 2;
int lcdCols = 16;
int xVal = 0;
int yVal = 0;

// declare all the pins for joystick
const int pinSW = 2; // digital pin connected to switch output
const int pinX = A0; // A0 - analog pin connected to X output
const int pinY = A1; // A1 - analog pin connected to Y output
const int resetAndBombPin = 13;
byte SwButtonState = HIGH;
//debounce for sw button
byte readingSw = HIGH;
byte lastReadingSw = HIGH;
unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 50;

byte buttonState = HIGH;
//debounce for separate button
byte reading = HIGH;
byte lastReading = HIGH;
unsigned int lastDebounceTimeB = 0;
unsigned int debounceDelayB = 50;

const int minThreshold = 300; // Adjust this threshold based on your joystick
const int maxThreshold = 700; // Adjust this threshold based on your joystick
int joyMoved = false;

int navigateMenuOn = 0;
int displayIntroTime = 2000;
int displayOnLcdInterval = 70; //message was flickerring witout it 

byte star[8] = {
  B00000,
  B10101,
  B01110,
  B11011,
  B01110,
  B10101,
  B00000,
  B00000
};
byte arrowDownUp[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};
byte optionArrow[8] = {
  B01000,
  B00100,
  B00010,
  B00001,
  B00010,
  B00100,
  B01000,
  B00000
};
byte lostLife[8] = {
  B00000,
  B11011,
  B10101,
  B10001,
  B01010,
  B00100,
  B00000,
  B00000
};
byte life[8] = {
  B00000,
  B11011,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

byte bigBomb[8] = {
  B00001,
  B00110,
  B01110,
  B11111,
  B11111,
  B11001,
  B11011,
  B01110
};
byte rightArrow[8] = {
  B00000,
  B00000,
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100
};
byte leftArrow[8] = {
  B00000,
  B00000,
  B00000,
  B00100,
  B01000,
  B11111,
  B01000,
  B00100
};
unsigned long startTime = 0;
unsigned long previousMillisDisplayonLCD = 0;
unsigned long previousMillisIntro = 0;

int crtMainOption = 0, crtMainOptionDisplayed = 1;
int crtSecondOption = 0, crtSecondOptionDisplayed = 1;
int scrollPrevMillis = 0;
int onOffPrevMillis = 0;
int subMenuOn = 0;
int lcdBrightness;
int inAbout = 0;
int gameON = 0;
//------------------GAME VARIABLES----------------
const byte dinPin = 12; // pin 12 is connected to the MAX7219 pin 1
const byte clockPin = 11; // pin 11 is connected to the CLK pin 13
const byte loadPin = 10; // pin 10 is connected to LOAD pin 12
const byte mapSize = 16; // change to mapSize
const byte realMatrixSize = 8; // 8 for MAX7219

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); //DIN, CLK, LOAD, No. DRIVER

byte matrixBrightness = 4;

int crtPlayerRow; //row that has associated 1 in isBlinking matrix
int crtPlayerCol; //col that has associated 1 in isBlinking matrix

int xValue = 0;
int yValue = 0;

//variable used for bliming and animations
unsigned int playerBlinkingRate = 250;
unsigned int bombBlinkingRate = 125;
unsigned int bigBombBlinkingRate = 300;
unsigned int playerMoveInterval = 250;
unsigned int unbrWallsBlinkRate = 10;
unsigned long previousMillisB = 0;
unsigned long previousMillisBb = 0;
unsigned long previousMillisP = 0;
unsigned long previousMillis = 0;
unsigned long previousMillisMenuLayout = 0;
unsigned long previousMillisLives = 0;
unsigned long previousMillisScore = 0;
unsigned long previousMillisUnbrWalls = 0;

bool playerLedState = 0;
bool bombLedState = 0;
bool bigBombLedState = 0;
bool unbrWallsState = 0;
//variables used as direction indicators that are set according to the movement of the joystick
unsigned int right = 0;
unsigned int left = 0;
unsigned int up = 0;
unsigned int down = 0;

unsigned int placedBomb = 0;

//variables used for saving the position in which a bombed was placed 
unsigned int currentRow;
unsigned int currentCol;
unsigned int bombRow;
unsigned int bombCol;
unsigned int bigBombRow;
unsigned int bigBombCol;

//when a bomb explodes it destroys walls of length 3 in each direction
unsigned int nrToExplode = 3;
//when bigbombs explodes, it destroys walls in square two block in each direction from the player position at the moment it was placed
unsigned int bigBombNrToExplode = 2;
//number of score needed to be able to explode big bomb
unsigned int bigBombScore = 10;

//used for setting a number of blinks until a bomb explodes
unsigned int currentBombTicks = 0, bombTicks = 15, currentBigBombTicks = 0, bigBombTicks = 3;
byte moveFlag = 1;

int mapMatrix[mapSize][mapSize];

//starting coordinates for displying on 8x8 matrix
byte startRow, startCol, startRowPos, startColPos;
byte defaultValue = 0, wallsValue = 1, playerValue = 2, bombValue = 3, bigBombValue = 4, unbreakableWalls = 5, bombAndPlayer = 10;
bool wallsFlag = 0, bombFlag = 0, bombAnimationFlag = 0, playerIsDead = 0, mapIsEmpty = 0, emptyCheckFlag = 0;
byte wallsProb, unbrWallsProb;
unsigned int score = 0;
unsigned int scoreForBigBomb = 0;
unsigned int totalNumberOfLives = 3;
unsigned int deadCounter = 0; //increments every time the player is killed by the bomb 
unsigned int remainingLives = 3; //initially the player has all the lives 
int heartIndex = 0;
int highestScore = -1;
String bigBombScoreNeeded = String(scoreForBigBomb);
String currentMessage = bigBombScoreNeeded + "/10 for";
byte eppromAddr[3] = {
  1,
  2,
  3
};
byte eepromValue;
bool subSubMenuOn = 0;
//on/off sound
int settingSound = 0, soundON = 1;//by default sound is ON
void setup() {
  // put your setup code here, to run once:
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(resetAndBombPin, INPUT_PULLUP);
  lcd.begin(lcdCols, lcdRows);
  lcd.createChar(0, star); // Create the custom character after lcd.begin()
  lcd.createChar(1, arrowDownUp);
  lcd.createChar(2, optionArrow);
  lcd.createChar(3, lostLife);
  lcd.createChar(4, life);
  lcd.createChar(5, bigBomb);
  lcd.createChar(6, rightArrow);
  lcd.createChar(7, leftArrow);
  startTime = millis();
  Serial.begin(9600);
  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, matrixBrightness); // sets brightness (0~15 possible values)
  lc.clearDisplay(0); // clear screen

  //probability for spawning walls, the bigger is value the smaller is chance (EX. wallsProb = 3 => chance = 1/3 = 33%)
  wallsProb = 3;
  unbrWallsProb = 5;

  //number of score needed to be able to explode big bomb
  unsigned int bigBombScore = 10;

  randomSeed(analogRead(pinX)); //seed random 
}

void loop() {
  unsigned long currentMillis = millis();
  if (gameON) {
    if (currentMillis - previousMillisDisplayonLCD >= displayOnLcdInterval) {
      previousMillisDisplayonLCD = currentMillis;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ClearTheMAP!");
    }
    playGame();
  } else {
    
    if (currentMillis - startTime <= displayIntroTime) {
      DisplayIntroMsg();
    } else {
      if (currentMillis - previousMillisIntro >= displayOnLcdInterval && navigateMenuOn == 0 && crtMainOption == 0) {
        previousMillisIntro = currentMillis;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Press for menu");
      }
    }
    displayMenuLayout();

    readingSw = digitalRead(pinSW);

    if (readingSw != lastReadingSw) {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (readingSw != SwButtonState) {
        SwButtonState = readingSw;
        if (SwButtonState == HIGH) {
          if (!navigateMenuOn) {
            navigateMenuOn = 1;
            Serial.println("in menu");
          } else {
            if (!subMenuOn) {
              crtMainOption = crtMainOptionDisplayed;
              Serial.print("crtMainOption: ");
              Serial.println(crtMainOption);
              takeMainMenuAction();

            } else if (!subSubMenuOn) {
              crtSecondOption = crtSecondOptionDisplayed;
              Serial.print("crtSecondOption: ");
              Serial.println(crtSecondOption);
              takeSubMenuAction();
            } else {
              Serial.print("subsubmenu exit ");
              settingSound = 0;
              subMenuOn = 0;
              subSubMenuOn = 0;
              crtSecondOption = 0; // no scnd option currently displayed or selected
              crtSecondOptionDisplayed = 0;
            }
          }
        }
      }
    }
    lastReadingSw = readingSw;

    joystickMenuMovement();
    
  }
}

void playGame() {

  if (!wallsFlag) {
    generateMapWalls();
    wallsFlag = 1;
  }

  displayElements();

  checkForEmpty();
  //reset deadcouter when reaches 3 
  if (deadCounter >= 3 || mapIsEmpty) {
    gameOver();
  }

}
void gameOver() {
  lcd.clear();
  if (!mapIsEmpty) {
    lcd.setCursor(3, 0);
    lcd.print("YOU LOST");
    lcd.setCursor(0, 1);
    lcd.print("YOUR SCR IS");
    lcd.setCursor(13, 1);
    lcd.print(score);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("MAP IS CLEAR!");
    lcd.setCursor(0, 1);
    lcd.print("YOU WON,SCR=");
    lcd.setCursor(13, 1);
    lcd.print(score);
  }
  delay(5000);
  byte eepromScore;
  EEPROM.get(0, eepromScore);
  if (score > eepromScore) {
    byte aux1, aux2;
    EEPROM.get(0, aux1);
    EEPROM.get(1, aux2);
    EEPROM.update(0, score);
    EEPROM.update(1, aux1);
    EEPROM.update(2, aux2);
  } else {
    EEPROM.get(1, eepromScore);
    if (score > eepromScore) {
      byte aux1;
      EEPROM.get(1, aux1);
      EEPROM.update(1, score);
      EEPROM.update(2, aux1);
    } else {
      EEPROM.get(2, eepromScore);
      if (score > eepromScore) {
        byte aux1;
        EEPROM.update(2, score);
      }
    }
  }
  resetMatrix();
  gameON = 0;
  crtMainOption = 0;
  crtMainOptionDisplayed = 1;
  score = 0;
  scoreForBigBomb = 0;
  deadCounter = 0;
  mapIsEmpty = 0;
  wallsFlag = 0;
}

void resetEEPROM() {
  for (int i = 0; i < 3; i++) {
    EEPROM.update(i, 0);
  }
}

void resetMatrix() {
  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      mapMatrix[row][col] = defaultValue;
      lc.setLed(0, row, col, false);
    }
  }
}

void setInitialPlayerPosition() {
  crtPlayerRow = 1 + random(mapSize - 10);
  crtPlayerCol = 1 + random(mapSize - 10);
  startRow = 0;
  startCol = 0;
  mapMatrix[crtPlayerRow][crtPlayerCol] = playerValue;
}

void generateMapWalls() {

  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      mapMatrix[row][col] = defaultValue;
    }
  }

  setInitialPlayerPosition();

  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      //no walls in close proximity of the player
      //formula source(chat gpt)
      int distanceToPlayer = abs(row - crtPlayerRow) + abs(col - crtPlayerCol);
      // Randomly decide whether to set the wall (50% chance)
      if (distanceToPlayer >= 3) { // Adjust the distance as needed
        byte walls, unbrWalls;
        walls = random(wallsProb);
        unbrWalls = random(unbrWallsProb);
        if (walls == 0 && mapMatrix[row][col] != unbreakableWalls) {
          mapMatrix[row][col] = wallsValue;
        }
        if (unbrWalls == 0 && mapMatrix[row][col] != wallsValue) {
          mapMatrix[row][col] = unbreakableWalls;
        }

      }

    }
  }
}

void displayElements() {

  startRowPos = startRow;
  startColPos = startCol;

  placeBomb();
  //place big bomb at every 15 score
  placeBigBomb();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisUnbrWalls >= unbrWallsBlinkRate) {
    previousMillisUnbrWalls = currentMillis;
    unbrWallsState = !unbrWallsState;
  }

  for (int row = startRowPos; row < startRowPos + realMatrixSize; row++) {
    for (int col = startColPos; col < startColPos + realMatrixSize; col++) {
      //clear empty
      if (mapMatrix[row][col] == 0) {
        lc.setLed(0, row - startRowPos, col - startColPos, false);
      }
      //display walls
      if (mapMatrix[row][col] == wallsValue) {
        lc.setLed(0, row - startRowPos, col - startColPos, true);
      }
      if (mapMatrix[row][col] == unbreakableWalls) {
        lc.setLed(0, row - startRowPos, col - startColPos, unbrWallsState);
      }
      //blink player
      if (mapMatrix[row][col] == playerValue || mapMatrix[row][col] == bombAndPlayer) {
        unsigned long currentMillis = millis();
        movePlayer(row, col);
        if (currentMillis - previousMillisP >= playerBlinkingRate) {
          previousMillisP = currentMillis;
          playerLedState = !playerLedState;
        }
        lc.setLed(0, row - startRowPos, col - startColPos, playerLedState);
      }
      //blink small bomb
      if (mapMatrix[row][col] == bombValue || mapMatrix[row][col] == bombAndPlayer) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillisB >= bombBlinkingRate) {
          previousMillisB = currentMillis;
          bombLedState = !bombLedState;
          currentBombTicks++;
          //buzz at every bomb tick
          if(soundON)
            tone(buzzerPin, 3000, 100);
        }
        lc.setLed(0, row - startRowPos, col - startColPos, bombLedState);
      }
      //blink big bomb
      if (mapMatrix[row][col] == bigBombValue) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillisBb >= bigBombBlinkingRate) {
          previousMillisBb = currentMillis;
          bigBombLedState = !bigBombLedState;
          //buzz at every bomb tick
          if(soundON)
            tone(buzzerPin, 150, 250);
          currentBigBombTicks++;
        }
        lc.setLed(0, row - startRowPos, col - startColPos, bigBombLedState);
      }
    }
  }
}

void checkForEmpty() {
  emptyCheckFlag = 0;
  for (int row = 0; row < mapSize; row++) {
    for (int col = 0; col < mapSize; col++) {
      if (mapMatrix[row][col] == wallsValue || mapMatrix[row][col] == unbreakableWalls) {
        emptyCheckFlag = 1;
      }
    }
  }
  if (!emptyCheckFlag) {

    mapIsEmpty = 1;
    deadCounter = 5;
    playerIsDead = 1;
  }
}

void placeBigBomb() {
  reading = digitalRead(resetAndBombPin);

  if (readingSw != lastReadingSw) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTimeB) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH && scoreForBigBomb >= bigBombScore) {
        bigBombRow = crtPlayerRow;
        bigBombCol = crtPlayerCol;
        mapMatrix[bigBombRow][bigBombCol] = bigBombValue;
      }
    }
  }
  lastReadingSw = readingSw;

  if (currentBigBombTicks >= bigBombTicks) {
    //set with the number of leds that can be turned off according to the position in the matrix
    int top, right, bottom, left;

    //for top
    if (bigBombRow < bigBombNrToExplode) {
      top = bigBombRow;
    } else {
      top = bigBombNrToExplode;
    }
    //for right
    if (mapSize - bigBombCol < bigBombNrToExplode) {
      right = mapSize - bigBombCol;
    } else {
      right = bigBombNrToExplode;
    }
    //for bottom
    if (mapSize - bigBombRow < bigBombNrToExplode) {
      bottom = mapSize - bigBombRow;
    } else {
      bottom = bigBombNrToExplode;
    }
    //for left
    if (bigBombCol < bigBombNrToExplode) {
      left = bigBombCol;
    } else {
      left = bigBombNrToExplode;
    }

    //first part of exploding phase of big bomb
    for (int row = bigBombRow - top; row <= bigBombRow + bottom; row++) {
      for (int col = bigBombCol - left; col <= bigBombCol + right; col++) {
        lc.setLed(0, row, col, true);
      }
    }
    if(soundON)
      tone(buzzerPin, 1000, 75);
    delay(200);
    //final part of exploding phase of big bomb
    for (int row = bigBombRow - top; row <= bigBombRow + bottom; row++) {
      for (int col = bigBombCol - left; col <= bigBombCol + right; col++) {
        if (mapMatrix[row][col] == wallsValue || mapMatrix[row][col] == unbreakableWalls) {
          score++;
          scoreForBigBomb++;
          mapMatrix[row][col] = defaultValue;
        }
      }
    }
    if(soundON)
      tone(buzzerPin, 100, 500);
    mapMatrix[bigBombRow][bigBombCol] = playerValue;
    currentBigBombTicks = 0;
    scoreForBigBomb = scoreForBigBomb - bigBombScore;
  }
}

void placeBomb() {
  readingSw = digitalRead(pinSW);

  if (readingSw != lastReadingSw) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (readingSw != SwButtonState) {
      SwButtonState = readingSw;
      if (SwButtonState == HIGH && !bombFlag && !playerIsDead) {
        mapMatrix[crtPlayerRow][crtPlayerCol] = bombAndPlayer;
        bombRow = crtPlayerRow;
        bombCol = crtPlayerCol;
        bombFlag = 1;
      } else if (SwButtonState == HIGH && playerIsDead) {
        //generateMapWalls();
        mapMatrix[crtPlayerRow][crtPlayerCol] = playerValue;
        playerIsDead = 0;
        // score = 0;
      }
    }
  }
  lastReadingSw = readingSw;
  if (bombFlag) {
    //set with the number of leds that can be turned off according to the position in the matrix
    int top, right, bottom, left;

    //for top
    if (bombRow < nrToExplode) {
      top = bombRow;
    } else {
      top = nrToExplode;
    }
    //for right
    if (mapSize - bombCol < nrToExplode) {
      right = mapSize - bombCol;
    } else {
      right = nrToExplode;
    }
    //for bottom
    if (mapSize - bombRow < nrToExplode) {
      bottom = mapSize - bombRow;
    } else {
      bottom = nrToExplode;
    }
    //for left
    if (bombCol < nrToExplode) {
      left = bombCol;
    } else {
      left = nrToExplode;
    }
    if (currentBombTicks == bombTicks) {
      if(soundON)
        tone(buzzerPin, 250, 500);
      if (!bombAnimationFlag) {
        // //first part of exploding animation
        //top
        for (int i = 0; i <= top; i++) {
          if (mapMatrix[bombRow - i][bombCol] == playerValue || mapMatrix[bombRow - i][bombCol] == bombAndPlayer) {
            playerIsDead = 1;
            //death sound
            if(soundON)
              tone(buzzerPin, 100, 500);
            deadCounter++;
            // Serial.print("deadCounter ");
            // Serial.println(deadCounter);  
          }
          if (mapMatrix[bombRow - i][bombCol] == wallsValue) {
            score++;
            scoreForBigBomb++;
            lc.setLed(0, bombRow - i, bombCol, true);
          }
        }

        //right 
        for (int i = 0; i <= right; i++) {
          if (mapMatrix[bombRow][bombCol + i] == playerValue || mapMatrix[bombRow][bombCol + i] == bombAndPlayer) {
            playerIsDead = 1;
            //death sound
            if(soundON)
              tone(buzzerPin, 100, 500);
            currentMessage = "pressToContinue";
            deadCounter++;
            // Serial.print("deadCounter ");
            // Serial.println(deadCounter);  
          }
          if (mapMatrix[bombRow][bombCol + i] == wallsValue) {
            score++;
            scoreForBigBomb++;
            lc.setLed(0, bombRow, bombCol + i, true);
          }
        }

        //bottom
        for (int i = 0; i <= bottom; i++) {
          if (mapMatrix[bombRow + i][bombCol] == playerValue || mapMatrix[bombRow + i][bombCol] == bombAndPlayer) {
            playerIsDead = 1;
            //death sound
            if(soundON)
               tone(buzzerPin, 100, 500);
            currentMessage = "pressToContinue";
            deadCounter++;
            //  Serial.print("deadCounter ");
            //  Serial.println(deadCounter); 

          }
          if (mapMatrix[bombRow + i][bombCol] == wallsValue) {
            score++;
            scoreForBigBomb++;
            lc.setLed(0, bombRow + i, bombCol, true);
          }
        }

        //left
        for (int i = 0; i <= left; i++) {
          //to check that playes is on the bomb when it explodes 
          if (mapMatrix[bombRow][bombCol - i] == playerValue || mapMatrix[bombRow][bombCol - i] == bombAndPlayer) {
            playerIsDead = 1;
            //death sound
            if(soundON)
              tone(buzzerPin, 100, 500);
            currentMessage = "pressToContinue";
            deadCounter++;
            // Serial.print("deadCounter ");
            // Serial.println(deadCounter);  
          }

          if (mapMatrix[bombRow][bombCol - i] == wallsValue) {
            score++;
            scoreForBigBomb++;
            lc.setLed(0, bombRow, bombCol - i, true);
          }
        }
        bombAnimationFlag = 1;
      }
      if (bombAnimationFlag) {
        for (int i = 0; i <= top; i++) {
          if (mapMatrix[bombRow - i][bombCol] != unbreakableWalls)
            mapMatrix[bombRow - i][bombCol] = 0;
        }
        for (int i = 0; i <= right; i++) {
          if (mapMatrix[bombRow][bombCol + i] != unbreakableWalls)
            mapMatrix[bombRow][bombCol + i] = 0;
        }
        for (int i = 0; i <= bottom; i++) {
          if (mapMatrix[bombRow + i][bombCol] != unbreakableWalls)
            mapMatrix[bombRow + i][bombCol] = 0;
        }
        for (int i = 0; i <= left; i++) {
          if (mapMatrix[bombRow][bombCol - i] != unbreakableWalls)
            mapMatrix[bombRow][bombCol - i] = 0;
        }
      }

      //bomb exploded reset flags and remove it from map
      bombFlag = 0;
      bombAnimationFlag = 0;
      currentBombTicks = 0;
      lc.setLed(0, bombRow, bombCol, false);
      //case when domb exploded and player didnt move at all
      if (mapMatrix[bombRow][bombCol] == bombAndPlayer || mapMatrix[bombRow][bombCol] == playerValue) {
        playerIsDead = 1;
        deadCounter++;
        //death sound
        if(soundON)
          tone(buzzerPin, 100, 500);
        currentMessage = "pressToContinue";
      }
      mapMatrix[bombRow][bombCol] = 0;
      //print on lcd display instead
      // Serial.println(score);

    }
  }
  displayGameState();
  displayScore();
}

void displayGameState() {
  if (millis() - previousMillisLives >= displayOnLcdInterval) {
    previousMillisLives = millis();
    if (playerIsDead) {
      lcd.setCursor(0, 1);
      lcd.print("PressToContinue");
    } else if (!playerIsDead) {
      if (scoreForBigBomb < bigBombScore) {
        lcd.setCursor(0, 1);
        String bigBombScoreCurrent = String(scoreForBigBomb);
        String bigBombScoreNeed = String(bigBombScore);
        String currentMessage = bigBombScoreCurrent + "/" + bigBombScoreNeed + " for";
        lcd.print(currentMessage);
        lcd.setCursor((currentMessage.length() + 1), 1);
        lcd.write(byte(5));
      } else {
        lcd.setCursor(0, 1);
        String currentMessage = "PrBtn for";
        lcd.print(currentMessage);
        lcd.setCursor(10, 1);
        lcd.write(byte(5));
      }
    }
    lcd.setCursor(13, 0);
    for (int i = 0; i < totalNumberOfLives - deadCounter; i++) {
      lcd.write(byte(4)); // Full life
    }
    for (int i = totalNumberOfLives - deadCounter; i < totalNumberOfLives; i++) {
      lcd.write(byte(3));
    }
  }
}

void displayScore() {
  if (millis() - previousMillisScore >= displayOnLcdInterval) {
    previousMillisScore = millis();
    if (!playerIsDead) {
      lcd.setCursor(13, 1);
      lcd.print(score);
    }

  }
}
void movePlayer(unsigned int row, unsigned int col) {
  int movePlayerBuzzerTone = 2500;
  int movePlayerBuzzerDuration = 100;
  unsigned long currentMillis = millis();
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  if (moveFlag) {
    //move player down
    if (xValue < minThreshold && joyMoved == false) {
      //move player down 
      if (row + 1 < mapSize && mapMatrix[row + 1][col] == 0) {
        //play sound
        if(soundON)
          tone(buzzerPin, movePlayerBuzzerTone, movePlayerBuzzerDuration);

        if (mapMatrix[crtPlayerRow][crtPlayerCol] == bombAndPlayer) {
          mapMatrix[crtPlayerRow][crtPlayerCol] = bombValue;
        } else {
          mapMatrix[crtPlayerRow][crtPlayerCol] = 0;
        }
        crtPlayerRow = row + 1;
        if (crtPlayerRow - startRow == realMatrixSize - 1 && (startRow + realMatrixSize) != mapSize) {
          startRow++;
          //Serial.println(startRowPos);
        }
        crtPlayerCol = col;
        mapMatrix[crtPlayerRow][crtPlayerCol] = playerValue;
      }
      //Serial.println("to down");
      joyMoved = true;
      xValue = 0;
      moveFlag = 0;
    }
    //move player up
    if (xValue > maxThreshold && joyMoved == false) {
      if (row - 1 >= startRow && mapMatrix[row - 1][col] == 0 && crtPlayerRow != 0) {
        //play sound
        if(soundON)
          tone(buzzerPin, movePlayerBuzzerTone, movePlayerBuzzerDuration);
        if (mapMatrix[crtPlayerRow][crtPlayerCol] == bombAndPlayer) {
          mapMatrix[crtPlayerRow][crtPlayerCol] = bombValue;
        } else {
          mapMatrix[crtPlayerRow][crtPlayerCol] = 0;
        }
        crtPlayerRow = row - 1;
        if (crtPlayerRow - startRow == 0 && startRow != 0) {
          startRow--;
          //Serial.println(startRowPos);
        }
        crtPlayerCol = col;
        mapMatrix[crtPlayerRow][crtPlayerCol] = playerValue;
      }
      // Serial.println("to up");
      joyMoved = true;
      xValue = 0;
      moveFlag = 0;
    }
    //move player right
    if (yValue > maxThreshold && joyMoved == false) {
      if (col + 1 < mapSize && mapMatrix[row][col + 1] == 0) {
        //play sound
        if(soundON)
          tone(buzzerPin, movePlayerBuzzerTone, movePlayerBuzzerDuration);
        if (mapMatrix[crtPlayerRow][crtPlayerCol] == bombAndPlayer) {
          mapMatrix[crtPlayerRow][crtPlayerCol] = bombValue;
        } else {
          mapMatrix[crtPlayerRow][crtPlayerCol] = 0;
        }
        crtPlayerRow = row;
        crtPlayerCol = col + 1;
        if (crtPlayerCol - startCol == (realMatrixSize - 1) && (startCol + realMatrixSize) != mapSize) {
          startCol++;
          //Serial.println(startColPos);
        }
        mapMatrix[crtPlayerRow][crtPlayerCol] = playerValue;
      }
      // Serial.println("to right");
      joyMoved = true;
      yValue = 0;
      moveFlag = 0;
    }
    //move player left
    if (yValue < minThreshold && joyMoved == false) {
      if (col - 1 >= startCol && mapMatrix[row][col - 1] == 0 && crtPlayerCol != 0) {
        //play sound
        if(soundON)
          tone(buzzerPin, movePlayerBuzzerTone, movePlayerBuzzerDuration);
        if (mapMatrix[crtPlayerRow][crtPlayerCol] == bombAndPlayer) {
          mapMatrix[crtPlayerRow][crtPlayerCol] = bombValue;
        } else {
          mapMatrix[crtPlayerRow][crtPlayerCol] = 0;
        }
        crtPlayerRow = row;
        crtPlayerCol = col - 1;
        if (crtPlayerCol - startCol == 0 && startCol != 0) {
          startCol--;
          //Serial.println(startRowPos);
        }
        mapMatrix[crtPlayerRow][crtPlayerCol] = playerValue;
      }
      // Serial.println("to left");
      joyMoved = true;
      yValue = 0;
      moveFlag = 0;
    }

    if (xValue >= minThreshold && xValue <= maxThreshold) {
      joyMoved = false;
      previousMillis = currentMillis;
    }
    if (yValue >= minThreshold && yValue <= maxThreshold) {
      joyMoved = false;
      previousMillis = currentMillis;
    }
  } else {
    if (currentMillis - previousMillis >= playerMoveInterval) {
      previousMillis = currentMillis;
      moveFlag = 1;
    }
  }
}
void joystickMenuMovement() {
  unsigned long currentMillis = millis();
  if (currentMillis - scrollPrevMillis >= 300) {
    scrollPrevMillis = currentMillis;
    // Read joystick values
    xVal = analogRead(pinX);
    // Check joystick movements
    if (xVal < minThreshold && joyMoved == false) {
      navigateDown();
      //Serial.println("down");
      joyMoved = true;
    }

    if (xVal > maxThreshold && joyMoved == false) {
      navigateUp();
      //Serial.println("up");
      joyMoved = true;
      //for correctly selecting the first main/second option
    }
  }
    // Reset joyMoved flag when joystick is back to the center
  if (xVal >= minThreshold && xVal <= maxThreshold) {
    joyMoved = false;
  }

  if (currentMillis - onOffPrevMillis >= 300) {
    onOffPrevMillis = currentMillis;
    yVal = analogRead(pinY);
    //eft movement -> sound on
    if (yVal < minThreshold && joyMoved == false && settingSound){
      soundON = 1;
      Serial.println("sound on");
      joyMoved = true;
    }
    //right movement -> sound off
    if (yVal > maxThreshold && joyMoved == false && settingSound){
      soundON = 0;
      Serial.println("sound off");
      joyMoved = true;
    }
  }
 
  if (yVal >= minThreshold && yVal <= maxThreshold) {
    joyMoved = false;
  }
}
void displayMenuLayout() {
  unsigned long currentMillis = millis();
  if (navigateMenuOn && !subSubMenuOn) {
    if (currentMillis - previousMillisMenuLayout >= displayOnLcdInterval) {
      previousMillisMenuLayout = currentMillis;
      lcd.clear();
      displayJoystickDirections();
      displayArrowOption();
      lcd.setCursor(3, 0);
      lcd.print("Main Menu: ");
      displayOption();
    }
  }
}
void takeMainMenuAction() {
  switch (crtMainOption) {

  case 1:
    Serial.println("Starting Game");
    gameON = 1;
    break;
  case 2:
    subMenuOn = 1;
    Serial.println("activated submenu");
    break;
  case 3:
    printAbout();
    crtMainOption = 0;
    crtMainOptionDisplayed = 1;
    Serial.print("out of about"); //without pressing the button again
    break;
  }
}
void takeSubMenuAction() {
  switch (crtSecondOption) {
  case 1:
    Serial.println("Saving matrix br...");
    subSubMenuOn = 1;
    break;
  case 2:
    Serial.println("Saving LCD br...");
    subSubMenuOn = 1;
    break;
  case 3:
    byte eepromScore;
    Serial.println("Best Score:");
    subSubMenuOn = 1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("bScr:");
    for (int i = 0; i < 3; i++) {
      EEPROM.get(i, eepromScore);
      lcd.setCursor(5 + (i * 4), 0);
      lcd.print(eepromScore);
    }
    lcd.setCursor(0, 1);
    lcd.print("press to exit!");

    break;
  case 4:
    Serial.println("ON / OFF");
    subSubMenuOn = 1;
    settingSound = 1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ON");
    lcd.setCursor(2, 0);
    lcd.print("/");
    lcd.setCursor(3, 0);
    lcd.print("OFF");
    lcd.setCursor(0, 1);
    lcd.print("L");
    lcd.setCursor(1, 1);
    lcd.write(byte(7));
    lcd.setCursor(2, 1);
    lcd.write(byte(6));
    lcd.setCursor(3, 1);
    lcd.print("R");
    // lcd.setCursor(7, 0);
    // lcd.print("Sound");
    // lcd.setCursor(13, 0);
    // if(soundON){
    //   lcd.print("ON");
    // }else{
    //   lcd.print("OFF");
    // }
    lcd.setCursor(5, 1);
    lcd.print("PressToExit");
    break;
  case 5:
    Serial.println("ResetScores");
    subSubMenuOn = 1;
    resetEEPROM();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scores reseted!");
    lcd.setCursor(0, 1);
    lcd.print("Press for menu");
    break;
  case 6:
    subMenuOn = 0;
    crtSecondOption = 0; // no scnd option currently displayed or selected
    crtSecondOptionDisplayed = 0;
    Serial.println("submenu dezactivated");
    break;
  }

}
void navigateDown() {

  // Implement logic for moving down in the menu
  if (!subMenuOn) {
    if (crtMainOptionDisplayed < 3) {
      crtMainOptionDisplayed++;
    } else crtMainOptionDisplayed = 1;

  } else {
    if (crtSecondOptionDisplayed < 6) {
      crtSecondOptionDisplayed++;
    } else crtSecondOptionDisplayed = 1;
  }
}

void navigateUp() {
  // Implement logic for moving up in the menu and cycling through options 
  if (!subMenuOn) {
    if (crtMainOptionDisplayed > 1) {
      crtMainOptionDisplayed--;
    } else crtMainOptionDisplayed = 3;
  } else {
    if (crtSecondOptionDisplayed > 1) {
      crtSecondOptionDisplayed--;
    } else crtSecondOptionDisplayed = 6;
  }

}

void DisplayIntroMsg() {
  lcd.setCursor(0, 0);
  lcd.print("Dive into matrix");
  lcd.setCursor(2, 1);
  lcd.write(byte(0)); // Display the custom character with index 0
  lcd.setCursor(3, 1);
  lcd.print("BOMBERMAN");
  lcd.setCursor(12, 1);
  lcd.write(byte(0)); // Display the custom character with index 0
}

void displayOption() {
  lcd.setCursor(4, 1);
  if (!subMenuOn) {
    switch (crtMainOptionDisplayed) {
    case 1:
      lcd.print("Start Game");
      break;
    case 2:
      lcd.print("Settings");
      break;
    case 3:
      lcd.print("About");
      break;
    default:
      lcd.print("Start Game"); //by default show the first option
      break;

    }
  } else if (!subSubMenuOn) {
    switch (crtSecondOptionDisplayed) {
    case 1:
      lcd.print("Matrix Br");
      break;
    case 2:
      lcd.print("LCD Br");
      break;
    case 3:
      lcd.print("BestScore");
      break;
    case 4:
      lcd.print("Sound");
      break;
    case 5:
      lcd.print("Reset Score");
      break;
    case 6:
      lcd.print("Back");
      break;
    default:
      lcd.print("Matrix Br"); //by default show the first option
      break;
    }
  }

}

void displayJoystickDirections() {
  lcd.setCursor(15, 1);
  lcd.write(byte(1));
}

void displayArrowOption() {
  lcd.setCursor(3, 1);
  lcd.write(byte(2));
}

void printAbout() {
  lcd.clear();
  inAbout = 1;
  // Add your "About" text here
  String aboutText = "place bombs to destroy walls, small bomb can destroy one type of wall, big bomb can destroy all types, you win when map is clear";

  // display scrolling
  // for (int i = 0; i < aboutText.length() - 16; ++i) {
  //     lcd.setCursor(0, 0);
  //     lcd.print(aboutText.substring(i,i+16));
  //     delay(500); // Adjust the delay time for scrolling speed
  // }

  //display in chunks
  for (int i = 0; i <= aboutText.length() / 32 + 2; i = i + 2) {
    lcd.setCursor(0, 0);
    lcd.print(aboutText.substring(((i + 0) * 16), ((i + 1) * 16)));
    lcd.setCursor(0, 1);
    lcd.print(aboutText.substring(((i + 1) * 16), ((i + 2) * 16)));
    delay(1000);
  }

  // Clear the screen after auto-scrolling
  lcd.clear();
}