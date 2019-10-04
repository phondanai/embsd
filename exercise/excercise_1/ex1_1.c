#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{

    int up_to, count, prime = 1;

    up_to = atoi(*(argv+1));

    for (int i = 2; i <= up_to; i++){
        prime = 1;
        for(int denom = 2; denom < i; denom++)
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
