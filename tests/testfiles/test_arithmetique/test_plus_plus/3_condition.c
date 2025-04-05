int main() {
    int x = 0;
    int y = 0;
    if (x++ == 0 && ++y == 1) {
        return x + y;
    }
    return 0;
}