int main() {
    int x = 0;
    int y = 1;
    if (x-- == 0 && --y == 0) {
        return x + y;
    }
    return 0;
}