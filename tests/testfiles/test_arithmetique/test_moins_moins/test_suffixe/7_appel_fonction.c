int f(int x) {
    return x * 2;
}
int main() {
    int a = 3;
    return f(a--);
}