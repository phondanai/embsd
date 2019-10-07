/*
 *
 * ex4.c
 * Write a program that multiplies two signed integers WITHOUT using "*" operator
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    int x, y, sum=0;

    if (argc < 3)
    {
        printf("please input two numbers\n");
        return 1;
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);

    for (int i =1;i<=y; i++)
        sum += x;

    printf("%d\n",sum);


    return 0;
}
