#include <stdio.h>
#include <dlfcn.h>

typedef int (*gcd_func)(int, int);
typedef float (*area_func)(float, float);

void *lib = NULL;
gcd_func gcd = NULL;
area_func area = NULL;

int load_lib(const char *path) {
    if (lib != NULL) {
        dlclose(lib);
    }
    lib = dlopen(path, RTLD_LOCAL | RTLD_LAZY);
    if (lib == NULL) {
        printf("error: %s\n", dlerror());
        return 0;
    }
    gcd = (gcd_func)dlsym(lib, "gcd");
    area = (area_func)dlsym(lib, "area");
    if (gcd == NULL || area == NULL) {
        printf("error: %s\n", dlerror());
        dlclose(lib);
        return 0;
    }
    return 1;
}

int main() {
    int current = 1;
    if (!load_lib("./libimpl1.dylib")) {
        return 1;
    }

    int cmd;
    while (scanf("%d", &cmd) == 1) {
        if (cmd == 0) {
            if (current == 1) {
                current = 2;
                if (!load_lib("./libimpl2.dylib")) {
                    return 1;
                }
            } else {
                current = 1;
                if (!load_lib("./libimpl1.dylib")) {
                    return 1;
                }
            }
            printf("impl %d\n", current);
        } else if (cmd == 1) {
            int a, b;
            scanf("%d %d", &a, &b);
            printf("%d\n", gcd(a, b));
        } else if (cmd == 2) {
            float a, b;
            scanf("%f %f", &a, &b);
            printf("%f\n", area(a, b));
        }
    }

    dlclose(lib);
    return 0;
}
