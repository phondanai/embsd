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

void swap(int * a0, int * a1)
{
    int t = *a0;
    *a0 = *a1;
    *a1 = t;
}

void sort(int a[], int n)
{
    int i, j;
    for(i = 0; i < n; i = i + 1)
    {
        for(j = 1; j < n; j = j + 1)
        {
            if(a[j-1] > a[j]) swap(&a[j - 1], &a[j]);
        }
    }
}

void main()
{
    int a[10] = {10,5,2,1,7,6,3,4,9,8};
    printArray(a, 10);
    sort(a, 10);
    printArray(a, 10);
}
