int factorielle(int n) {
    if (n < 2)
        return 1;

    return factorielle(n-1)*n;
}


int main() {
    int a = 0;
    int b;
    while (a < 10) {
        b = factorielle(a);
        a = a+1;
    }
        
    return b;
}