/*
 *
 * ex8.c
 * Write a program that prints a sequence of numbers (given from the command argument), sort these numbers, and print the sorted number sequence.
 *
 */


#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("please input sequence of numbers\n");
        printf("example; %s 20 1 90 9222 42 13 \n", argv[0]);
        printf("\nInput:\n20 1 90 9222 42 13 \nSorted:\n1 13 20 42 90 9222\n");
        return 1;
    }

    int temp;
    int x, y, sum=0;
    int in_array[argc-1];
    int length = argc-1;

    printf("Input:\n");
    for (int i = 0; i<length; i++)
    {
        in_array[i] = atoi(argv[i+1]);
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

    printf("\n");
    return 0;
}
