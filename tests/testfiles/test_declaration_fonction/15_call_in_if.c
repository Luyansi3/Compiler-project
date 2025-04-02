int greater_than(int a, int b) {
    return a>b;
}


int double_it(int a) {
    return a*2;
}


int main() {
    int a = 12;
    if (greater_than(a, 80))
        return 1;
    else if (greater_than(a, 12 ))
        return 2;
    else 
        return 0;
}