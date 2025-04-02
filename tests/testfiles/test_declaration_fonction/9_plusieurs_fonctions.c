int add(int a, int b) {
    return a+b;
}


int sub(int a, int b) {
    return a-b;
}

int multiplier(int a, int b) {
    return a*b;
}

int main() {
    int a = 4;
    int b = 12-a;
    int c = 1;

    a = a * sub(a,b);
    b = mult(a, a);

    return add(a, c);
}