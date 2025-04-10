int f(int x) {
    return (x & 240) >> 4;
}
int main() {
    return f(165);
}