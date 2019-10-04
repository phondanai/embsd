#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{

    int up_to, count;

    up_to = atoi(*(argv+1));

    for(int i = 0; i <= up_to; i++)
    {
        count = 0;

        for(int j = 1; j <= i; j++)
        {
            if( i % j ==0)
                count++;
        }
        if (count == 2)
            printf("%d\n", i);
    }



    return 0;
}
