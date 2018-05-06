#include <Arduino.h>
#include "position.h"


/****
 * Consants and definitions
 */

#define PIN_A 8
#define PIN_B 9
#define PIN_C 10
#define PIN_S 11

#define PIN_L0 4
#define PIN_L1 5
#define PIN_L2 6
#define PIN_L3 7


enum Mode { Random, Snake, AnimateLevels };
const byte levelCount = 4;
const byte columnCount = 16;
const byte sideLength = 4;

const int showDelay = 1200;
const int defaultFrameDuration = 250;

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
                digitalWrite(PIN_L0 + lastLevel, HIGH);
            }
            digitalWrite(PIN_L0 + level, LOW);
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

/******
 * Animate levels
 */

void lightLevel(int l) {
    for (byte i = 0; i < 16; i++) {
        writeLed(i, l);
        delayMicroseconds(showDelay);
    }
}

void animateLevels() {
    for (byte l = 0; l < 4; l++) {
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

    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_C, LOW);
    digitalWrite(PIN_S, HIGH);

    digitalWrite(PIN_L0, HIGH);
    digitalWrite(PIN_L1, HIGH);
    digitalWrite(PIN_L2, HIGH);
    digitalWrite(PIN_L3, HIGH);
    //Serial.begin(9600);

    clearState();
}

void loop() {
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

