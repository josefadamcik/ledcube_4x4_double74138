#include <Arduino.h>


#define PIN_A 8
#define PIN_B 9
#define PIN_C 10
#define PIN_S 11

#define PIN_L0 4
#define PIN_L1 5
#define PIN_L2 6
#define PIN_L3 7


int lastLevel = -1;



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

}

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

void lightLevel(int l) {
    for (byte i = 0; i < 16; i++) {
        writeLed(i, l);
        delayMicroseconds(1200);
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

void loop() {
    // writeLed(0,0);
    // delay(2000);
    animateLevels();
    // turnOff();
    // delay(2000);
}
