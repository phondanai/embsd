#include <stdio.h>

int sum_up(int n) /* sum up to n */
{
   int i, c = 0; /* declares "int" data i, c (c is initialized to 0) */ 

   for(i = 1; i <= n; i = i + 1) /* i = 1, 2, ..., n */
       c = c +i;

   return c;
}

void main()
{
    int c = sum_up(100);
    printf("c = %d\n", c);
}
