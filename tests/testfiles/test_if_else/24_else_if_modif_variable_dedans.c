int main() {
    int x = 5;
    int a = 0;
    
    if (x > 3) {
        a = 1;
        x = 13;
    } else if (x > 10) {
        a = 2;
    } else if (x > 11) {
        a = 3;
    }
    
    return a;
}