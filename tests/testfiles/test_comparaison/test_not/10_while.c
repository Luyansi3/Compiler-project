int main() {
    int a = 0;
    int b = 0;

    while (!a) {
        if (b > 3) {
            a = 5;
        }

        b = b + 1;
    }

    return a;
}