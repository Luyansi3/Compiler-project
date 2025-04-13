int main() {
    int a = 0;
    while (a < 7) {
        a = a | 1;
        a = a + 2;
    }
    return a;
}