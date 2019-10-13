/*
 * ex2.c
 * Write a program that prints all prime numbers up to N 
 * (value N should be given from the command argument)
 */

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("please input a number\n");
        return 1;
    }

    int up_to, prime, i, denom;

    up_to = atoi(argv[1]);

    for (i = 2; i <= up_to; i++){
        prime = 1;
        for(denom = 2; denom < i; denom++)
        {
            if((i % denom) == 0) {
                prime = 0;
                break;
            }
        }
        if (prime == 1)
        {
            printf("%d\n", i);
        }
    }



    return 0;
}
