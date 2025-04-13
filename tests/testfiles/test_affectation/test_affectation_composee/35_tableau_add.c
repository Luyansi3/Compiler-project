int main() {
    int a = 1;
    int b = 2;

    int d[] = {4,4,5};
    d[b] += a+9;
    a=2;
    b=5;
    return d[a]; 
}