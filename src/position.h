#include <Arduino.h>

struct Position {
    byte x;
    byte y;
    byte level; 
    Position(byte l, byte x, byte y): x(x), y(y),  level(l) {};
    Position(): x(-1), y(-1), level(-1) {};
    int column() {
        return x + y * 4;
    }
};


