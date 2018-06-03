#include <Arduino.h>
#include "position.h"



/****
 * Consants and definitions
 */


// === UNO
 
// #define PIN_A 8
// #define PIN_B 9
// #define PIN_C 10
// #define PIN_S 11

// #define PIN_L0 4
// #define PIN_L1 5
// #define PIN_L2 6
// #define PIN_L3 7

// #define PIN_BTN 2
// #define PIN_SPEED A0

// ===  ATMEL ATTINY84 / ARDUINO
//
//                           +-\/-+
//                     VCC  1|    |14  GND
//             (D 10)  PB0  2|    |13  AREF (D  0)
//             (D  9)  PB1  3|    |12  PA1  (D  1) 
//                     PB3  4|    |11  PA2  (D  2) 
//  PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3) 
//  PWM        (D  7)  PA7  6|    |9   PA4  (D  4) 
//  PWM        (D  6)  PA6  7|    |8   PA5  (D  5)        PWM

#define PIN_A 1
#define PIN_B 2
#define PIN_C 3
#define PIN_S 4

#define PIN_L0 5
#define PIN_L1 6
#define PIN_L2 7
#define PIN_L3 10


#define PIN_BTN 8 //int0s
#define PIN_SPEED A0 //D0

// EOF PINS


enum Mode { Random, Snake, AnimateLevels };
const byte levelCount = 4;
const byte columnCount = 16;
const byte sideLength = 4;

const int showDelay = 1200;
const int defaultFrameDuration = 250;
const int minFrameDuration = 20;
const int maxFrameDuration = 600;

/**
 * State
 **/

int lastLevel = -1;
Mode currentMode = Snake;
int frameDuration = defaultFrameDuration;


boolean state[levelCount][columnCount] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

Mode nextMode(Mode mode) {
    switch(mode) {
        case Random: return Snake;
        case Snake: return AnimateLevels;
        case AnimateLevels: return Random;
    }
}
uint8_t level2pin(uint8_t l) {
    switch (l) {
        case 0: return PIN_L0;
        case 1: return PIN_L1;
        case 2: return PIN_L2;
        case 3: return PIN_L3;
    }
}

/****** 
 * LED control
 */ 

/**
 * @param column -  0-15
 * @param level - 0-3
 */ 
void writeLed(int column, int level) {
        if (lastLevel != level) {
            if (lastLevel >= 0) {
                digitalWrite(level2pin(lastLevel), HIGH);
            }
            digitalWrite(level2pin(level), LOW);
            lastLevel = level;
        }
        //select first or second 74138 ic
        if (column >= 8) {
            column -= 8;
            digitalWrite(PIN_S, LOW);
        } else {
            digitalWrite(PIN_S, HIGH);
        }
        digitalWrite(PIN_A, bitRead(column, 0));
        digitalWrite(PIN_B, bitRead(column, 1));
        digitalWrite(PIN_C, bitRead(column, 2));
}

void writeState(unsigned long forMillis) {
    unsigned long started = millis();
    do {
        for (int l = 0; l < levelCount; l++) {
            for (int c = 0; c < columnCount; c++) {
                if (state[l][c]) {
                    writeLed(c, l);
                    delayMicroseconds(showDelay);
                }
            }
        }
    } while (started + forMillis > millis());
} 

void clearState() {
    for (int c = 0; c < columnCount; c++) {
        for (int l = 0; l < levelCount; l++) {
            state[l][c] = false;
        }
    }
}

/**
 * Button interrupt and debouncing.
 */ 

const unsigned long debounceTime = 150;
volatile boolean debouncing = false;
volatile int lastButtonState = HIGH;
volatile unsigned long changeDebounceStarted;
volatile boolean triggerObserved = false;

void btn_interrupt() {
    int buttonState = digitalRead(PIN_BTN);
    if (lastButtonState == HIGH && buttonState == LOW) { // button press, start deboucne
        //start debouncing every time
        changeDebounceStarted = millis();
        debouncing = true;
    } else if (lastButtonState == LOW && buttonState == HIGH) { 
        if (debouncing && changeDebounceStarted + debounceTime < millis()) {
            triggerObserved = true;
            debouncing =  false;
        }
    }

    lastButtonState = buttonState;
}


/******
 * Animate levels
 */

void lightLevel(int l) {
    for (byte i = 0; i < columnCount; i++) {
        writeLed(i, l);
        delayMicroseconds(showDelay);
    }
}

void animateLevels() {
    for (byte l = 0; l < levelCount; l++) {
        for (int i = 0; i < 10; i++) {
            lightLevel(l);
        }
    }
}

/******
 * RANDOM
 */

int  randomOnCount = 0;
const int randomMaxOn = 5;

void changeStateRandomWithLimit() {
    boolean changed = false;
    do {
        int l = random(0,levelCount);
        int c = random(0,columnCount);
        if (randomOnCount < randomMaxOn  && !state[l][c])  {
            state[l][c] = true;
            randomOnCount++;
            changed = true;
        } else if (randomOnCount >= randomMaxOn  && state[l][c]) {
            state[l][c] = false;
            randomOnCount--;
            changed = true;
        }
    } while (!changed);
}


/******
 * SNAKE
 */

const byte snakeMaxLen = 5;
byte snakeLen = 0;
Position snake[snakeMaxLen];

void randomSnakeNextMove() {
    //choose random direction
    boolean validDerection = false;
    Position newPosition;
    while (!validDerection) {
        newPosition = snake[0];
        byte direction = random(0,6);
        switch(direction) {
            case 0: //level up
                newPosition.level++;
                break;
            case 1: //level down
                newPosition.level--;
                break;
            case 2: //x+
                newPosition.x++;
                break;
            case 3: //x-
                newPosition.x--;
                break;
            case 4:
                newPosition.y++;
                break;
            case 5:
                newPosition.y--;
                break;
        }
        validDerection = true;
        //checkvalidity - cube bounds
        if (newPosition.level < 0 || newPosition.level >= levelCount) {
            validDerection = false;
        } else if (newPosition.x < 0 || newPosition.x >= sideLength || newPosition.y < 0 || newPosition.y >= sideLength ) {
            validDerection =  false;
        } else {
            //checkvalidity - collision with snake's body
            for (int i = 1; i < snakeMaxLen; i++) {
                if (newPosition.level == snake[i].level && newPosition.y == snake[i].y && newPosition.x == snake[i].x) {
                    validDerection = false;
                    break;
                }
            }
        }
    }     

    snake[0] = newPosition;
}

void changeStateRandomSnake() {
    if (snakeLen == snakeMaxLen) {
        //turn off the last segment of body
        state[snake[snakeMaxLen - 1].level][snake[snakeMaxLen - 1].column()] = false;
        snakeLen--;
    }
    //move body segments in array
    for (int i = snakeMaxLen - 1; i > 0; i--) {
        snake[i] = snake[i-1];
    }
    //determine new segment's position
    if (snakeLen == 0) {
        snake[0].level = random(0,levelCount);
        snake[0].x = random(0,4);
        snake[0].y = random(0,4);
    } else {
        randomSnakeNextMove();
    }

    snakeLen++;
    state[snake[0].level][snake[0].column()] = true;
    
}



/******
 * setup & loop
 */



void setup() {
    // put your setup code here, to run once:
    pinMode(PIN_A, OUTPUT);
    pinMode(PIN_B, OUTPUT);
    pinMode(PIN_C, OUTPUT);
    pinMode(PIN_S, OUTPUT);
    pinMode(PIN_L0, OUTPUT);
    pinMode(PIN_L1, OUTPUT);
    pinMode(PIN_L2, OUTPUT);
    pinMode(PIN_L3, OUTPUT);

    pinMode(PIN_BTN, INPUT_PULLUP);

    //todo: return btn functionality
    //attachInterrupt(digitalPinToInterrupt(PIN_BTN), btn_interrupt, CHANGE);
    attachInterrupt(0, btn_interrupt, CHANGE);

    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_C, LOW);
    digitalWrite(PIN_S, HIGH);

    digitalWrite(PIN_L0, HIGH);
    digitalWrite(PIN_L1, HIGH);
    digitalWrite(PIN_L2, HIGH);
    digitalWrite(PIN_L3, HIGH);
    // Serial.begin(9600);

    clearState();
}

void loop() {
    int speedInput = analogRead(PIN_SPEED);
    frameDuration = map(speedInput, 0, 1023, minFrameDuration, maxFrameDuration);


    if (triggerObserved) {
        triggerObserved = false;
        currentMode = nextMode(currentMode);
        clearState();
        writeState(frameDuration);
    }

    boolean shouldWriteState = true;
    switch(currentMode) {
        case AnimateLevels:
            animateLevels();
            shouldWriteState = false;
            break;
        case Random:
            changeStateRandomWithLimit();
            break;
        case Snake:
            changeStateRandomSnake();
            break;
    }
    
    if (shouldWriteState) {
        writeState(frameDuration);
    }
}

