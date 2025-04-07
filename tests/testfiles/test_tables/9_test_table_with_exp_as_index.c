int main()
{
    int a[8];
    a[0]=1;
    a[1]=2;
    a[a[0]]=a[1]+a[0];
    return a[a[0]];
}