int main() {
    int a = 0;
    int b = 1;
    if (a++) {
        b = 2;
    }
    else {
        b = 3;
    }
    return b;
}