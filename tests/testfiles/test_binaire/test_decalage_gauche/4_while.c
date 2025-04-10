int main() {
    int a = 1;
    int b = 0;
    while (b < 4) {
        a = a << 1;
        b = b + 1;
    }
    return a;
}