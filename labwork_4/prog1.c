#include <stdio.h>
#include "contracts.h"

int main() {
    int cmd;
    while (scanf("%d", &cmd) == 1) {
        if (cmd == 1) {
            int a, b;
            scanf("%d %d", &a, &b);
            printf("%d\n", gcd(a, b));
        } else if (cmd == 2) {
            float a, b;
            scanf("%f %f", &a, &b);
            printf("%f\n", area(a, b));
        }
    }
    return 0;
}
