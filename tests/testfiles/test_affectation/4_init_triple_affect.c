#include <inttypes.h>

int main() {
    int b=23;
    int a = b;
    int c;
    int d;
    d= c = a = b;
    return c;
}