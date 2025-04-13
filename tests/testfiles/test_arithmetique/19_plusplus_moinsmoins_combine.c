int main() {
    int a = 5;
    int b = 2;
    int c = 10;
    int r = (++a * 2 - b--) / 3 + c / 4;
    return r;
}