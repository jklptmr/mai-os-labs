#include "contracts.h"

int gcd(int a, int b) {
    while (b != 0) {
        int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

float area(float a, float b) {
    return a * b;
}
