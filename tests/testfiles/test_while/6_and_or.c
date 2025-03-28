int main() {
    int a = 3;
    int b = 1;
    while (a > 5 && a < 7 || b < 3) {
        b = b + 1;
        a = a + 1;
    }
    return a;
}