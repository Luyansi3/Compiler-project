int alphabet(int n)
{
    int a;
    a='A';
    while (a<'A'+n)
    {
        putchar(a);
        a=a+1;
    }
    return a;
}
int main()
{
    int c;
    c = alphabet(15);
    return c;
}