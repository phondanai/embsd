#include <stdio.h>
#include <stdlib.h>


int atoi2(char *str) {
    int result = 0;
    int i = 0;

    while(str[i] != '\0') {
        result = (result*10)+ str[i] - '0';
//        printf("%d, ", result);
        i++;
    }

    return result;
}

int main(int argc, char *argv[])
{
    //res = res * 10 + c - '0';

    printf("%d\n", atoi2(argv[1]));

    return 0;
}
