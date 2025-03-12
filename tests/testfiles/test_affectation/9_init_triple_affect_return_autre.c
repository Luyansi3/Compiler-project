#include <inttypes.h>

int main() {
    int b=23;
    int a = b;
    int c;
    c = a = b;
    return a;
}