#include <stdio.h>


void printArray(int array[], int n)
{
    int j;
    printf("array[%d] = {", n);
    for(j = 0; j < n; j++)
    {
        if(j > 0) printf(", ");
        printf("%d", array[j]);
    }

    printf("}\n");
}

void main()
{
    int a[10] = {1,2,3,4,5,6,7,8,9,10};
    printArray(a, 10);
}
