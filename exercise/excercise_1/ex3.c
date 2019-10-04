#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    int x, y, sum=0;

    if (argc < 2)
    {
        printf("please input two numbers\n");
        return 1;
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);
    sum = x;
    int count = 0;

    while(sum > 0)
    {
        sum -= y;
        count++;
    }

    printf("%d\n",count);


    return 0;
}
