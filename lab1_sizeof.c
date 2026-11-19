#include <stdio.h>

int main() {
    printf("Data Type Sizes on this system:\n");
    printf("-----------------------------------\n");
    printf("sizeof(char)          = %zu bytes\n", sizeof(char));
    printf("sizeof(int)           = %zu bytes\n", sizeof(int));
    printf("sizeof(float)         = %zu bytes\n", sizeof(float));
    printf("sizeof(double)        = %zu bytes\n", sizeof(double));
    printf("sizeof(long)          = %zu bytes\n", sizeof(long));
    printf("sizeof(unsigned int)  = %zu bytes\n", sizeof(unsigned int));
    printf("sizeof(long long)     = %zu bytes\n", sizeof(long long));
    return 0;
}
