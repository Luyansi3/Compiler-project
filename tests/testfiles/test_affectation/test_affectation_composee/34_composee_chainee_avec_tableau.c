int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d[] = {4,4,5};
    a += b -= d[2] *= d[1];
    return d[2]; 
}