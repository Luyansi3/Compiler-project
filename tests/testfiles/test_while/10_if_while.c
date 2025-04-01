int main() {
    int a = 1;
    
    if (a > 3)
        a = a + 1;
    else
        {
            while (a < 5){
                if (a > 3)
                    a = a + 1;
                else
                    a = a + 2;
            }            
        }
    return a;
}