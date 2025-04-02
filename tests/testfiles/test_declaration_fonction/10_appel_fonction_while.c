int main() {
    int a = 0;
    while (smaller_than(a, 10)) {
        a = add_one(a);
    }

    return a;
}


int smaller_than(int a, int b) {
    return a < b;
}


int add_one(int a) {
    return a+1;
}