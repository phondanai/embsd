#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>


int main(int argc, char *argv[])
{

    int i, j, up_to, count;

    up_to = atoi(*(argv+1));

    int primes[up_to+1];
    for(i = 2; i<=up_to; i++)
        primes[i] = i;

    i = 2;
    while ((i*i) <= up_to)
    {
        if (primes[i] != 0)
        {
            for(j=2; j<up_to; j++)
            {
                if (primes[i]*j > up_to)
                    break;
                else
                    primes[primes[i]*j]=0;
            }
        }
        i++;
    }

    for (i=2; i<=up_to; i++)
    {
        if (primes[i]!=0)
            printf("%d\n", primes[i]);
    }



    return 0;
}
