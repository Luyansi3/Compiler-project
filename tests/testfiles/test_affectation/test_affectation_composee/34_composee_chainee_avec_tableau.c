int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d[] = {4,1,5};
    a += b *= d[2] -= d[1] /= d[0];
    return d[2]; 
}