#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("please input numbers\n");
        return 1;
    }

    int temp;
    int x, y, sum=0;
    int in_array[argc-1];
    int length = argc-1;

    //printf("argc = %d\n", argc);
    printf("Input:\n");
    for (int i = 0; i<length; i++)
    {
        in_array[i] = atoi(argv[i+1]);
        //printf("index %d: %d\n", i, in_array[i]);
        printf("%d ", in_array[i]);
    }
    printf("\nSorted:\n");

    for (int i =0; i<length; i++)
    {
        for(int j = i+1; j<length; j++)
        {
            if (in_array[i] > in_array[j])
            {
                temp = in_array[i];
                in_array[i] = in_array[j];
                in_array[j] = temp;
            }
        }
    }

    for(int i = 0; i < length; i++)
        printf("%d ", in_array[i]);
        //printf("in_array[%d]: %d\n", i, in_array[i]);

    printf("\n");
    return 0;
}
