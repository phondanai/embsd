#include <stdio.h>

void print_float(float f)
{
    int i;
    int v = *(int *) (&f);

    for(i = 31; i >=0; i--)
    {
        if(v & (1 << i)) printf("1");
        else printf("0");

        if(i == 31 || i == 23) printf(" ");
    }

    printf("\n");
}

void main()
{
    int a;
    float f = 1.5;
    printf("%f = ",f);
    print_float(f);
    for(a = 0;a < 10; a++)
    {
        f = f * f;
        printf("%e = ", f);
        print_float(f);
    }
}
