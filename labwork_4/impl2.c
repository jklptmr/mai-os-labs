#include "contracts.h"

int gcd(int a, int b) {
    int min = a;
    if (b < a) {
        min = b;
    }
    int res = 1;
    for (int i = 1; i <= min; i++) {
        if (a % i == 0 && b % i == 0) {
            res = i;
        }
    }
    return res;
}

float area(float a, float b) {
    return a * b / 2;
}
