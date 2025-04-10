int main() {
    int a = 7;
    while ((a & 1) == 1) {
        a = a >> 1;
    }
    return a; 
}