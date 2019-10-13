#include <stdio.h>
#include <string.h>


void main()
{
    char text[12];
    int i;
    strcpy(text, "hello world");
    printf("text = %s\n", text);

    for (i = 0; i < 12; i++)
    {
        printf("text[%2d] = '%c' (val = %d)\n", i, text[i], text[i]);
    }

    strcpy(text, "world!");
    printf("text = %s\n", text);
    for(i = 0; i < 12; i++)
    {
        printf("text[%2d] = '%c' (val = %d)\n", i, text[i], text[i]);
    }
}
