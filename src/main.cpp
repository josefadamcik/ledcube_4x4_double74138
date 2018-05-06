#include <Arduino.h>


#define PIN_A 8
#define PIN_B 9
#define PIN_C 10
#define PIN_S 11

#define PIN_L0 4
#define PIN_L1 5
#define PIN_L2 6
#define PIN_L3 7

#define SHOW_DELAY 1000

struct Position {
    byte column;
    byte level;
    Position(byte c, byte l) : column(c), level(l) {}
};

enum Mode { Random, Snake };

int lastLevel = -1;
Mode currentMode = Random;
int frameDuration = 200;


boolean state[4][16] = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};





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
        // Serial.print(column);
        
        //select first or second 74138 ic
        if (column >= 8) {
            column -= 8;
            digitalWrite(PIN_S, LOW);
        } else {
            digitalWrite(PIN_S, HIGH);
        }
        // Serial.print('\t');
        // Serial.print(bitRead(column, 0));
        // Serial.print(bitRead(column, 1));
        // Serial.print(bitRead(column, 2));
        digitalWrite(PIN_A, bitRead(column, 0));
        digitalWrite(PIN_B, bitRead(column, 1));
        digitalWrite(PIN_C, bitRead(column, 2));
        // Serial.println();


}

void writeState(long forMillis) {
    long started = millis();
    do {
        for (int l = 0; l < 4; l++) {
            for (int c = 0; c < 16; c++) {
                if (state[l][c]) {
                    writeLed(c, l);
                    delayMicroseconds(SHOW_DELAY);
                }
            }
        }
    } while (started + forMillis > millis());

} 

void lightLevel(int l) {
    for (byte i = 0; i < 16; i++) {
        writeLed(i, l);
        delayMicroseconds(SHOW_DELAY);
    }
}

void animateLevels() {
    for (byte l = 0; l < 4; l++) {
        for (int i = 0; i < 10; i++) {
            lightLevel(l);
        }
    }
}

void turnOff() {
    if (lastLevel >= 0) {
        digitalWrite(PIN_L0 + lastLevel, HIGH);
        lastLevel = -1;
    }
}


int  randomOnCount = 0;
const int randomMaxOn = 5;

void changeStateRandomWithLimit() {
    boolean changed = false;
    do {
        int l = random(0,4);
        int c = random(0,16);
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



byte snakeMaxLen = 3;
byte snakeLen = 0;
Position snake[] = {Position(-1,-1), Position(-1,-1), Position(-1,-1)};


void changeStateRandomSnake() {
    Position next = Position(-1,-1);
    if (snakeLen == 0) {
        next.column = random(0,16);
        next.level = random(0,4);
        snake[snakeLen] = next;
    } 
    
}

void clearState() {
    for (int c = 0; c < 16; c++) {
        for (int l = 0; l < 4; l++) {
            state[l][c] = false;
        }
    }
}

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
    switch(currentMode) {
        case Random:
            changeStateRandomWithLimit();
            break;
        case Snake:
            changeStateRandomSnake();
            break;
    }
    
    writeState(frameDuration);
}

