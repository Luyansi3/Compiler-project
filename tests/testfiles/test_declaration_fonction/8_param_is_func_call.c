int add(int a, int b) {
    return a+b;
}


int main() {
    int b = 2;
    int c = 0;
    int a = add(add(b,c), b);


    return a;
}