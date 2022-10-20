#include <stdio.h>

int main(const int argc, char * const argv[])
{
    for (int i = 1; i < argc; ++i)
        printf("%s\n", argv[i]);
        
    return 0;
} 
