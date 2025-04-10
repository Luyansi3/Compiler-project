int main()
{
    int a[1]={1};
    int b[2]={a[0],a[0]+a[0]};
    return b[0];
}