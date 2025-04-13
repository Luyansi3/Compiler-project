int main() {
    int i = 0;
    while (i < 2) {
        int j = helperFunction(i);
        while (j < 10) {
            int k = j + 1;
            j = j + 5;
        }
        i = i + 1;
    }
    return j;
}