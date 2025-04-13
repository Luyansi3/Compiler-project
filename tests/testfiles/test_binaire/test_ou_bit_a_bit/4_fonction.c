int set_bit(int val, int bit) {
    return val | (1 << bit);
}
int main() {
    return set_bit(4, 1); 
}
