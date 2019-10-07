/*
 * 
 * ex6.c
 * Write your own "atoi" function.
 *
 */

#include <stdio.h>


int atoi2(char *str) {
    int result = 0;
    int i = 0;

    while(str[i] != '\0') {
        result = (result*10)+ str[i] - '0';
        i++;
    }

    return result;
}

int main(int argc, char *argv[])
{
    if (argc < 2){
        printf("please input one number as argument\n");
        return 1;
    }
    printf("%d\n", atoi2(argv[1]));

    return 0;
}
