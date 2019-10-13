/*
 *
 * ex3.c
 * Write a program that prints the "prime factored form" of N.
 *
 */

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){

    if (argc < 2){
        printf("Print prime factor from given number\n");
        printf("Please input one number\n");
        return 1;
    }

    int number, div=2;

    number = atoi(argv[1]);

    printf("%d = ", number);
    while(number!=1) {
        if(number % div != 0)
        {
            div = div + 1;
        }
        else
        {
            number = number / div;
            if(number != 1){
                printf("%d * ", div);
            }
            else
            {
                printf("%d", div);
                break;
            }
        }
    }
    printf("\n");

    return 0;
}
