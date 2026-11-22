#include <stdio.h>
#include <stdlib.h>

int global_init = 42;         
int global_uninit;             
static int local_static = 20; 

int main() {
    int local_var = 10;                         
    int *heap_var = (int *)malloc(sizeof(int)); 
    *heap_var = 500;

    printf(".......Variable Addresses.......\n\n");
    printf("global_var (Data/BSS)  : %p\n", (void *)&global_init);
    printf("bss_var    (Data/BSS)  : %p\n", (void *)&global_uninit);
    printf("static_var (Data/BSS)  : %p\n", (void *)&local_static);
    printf("local_var  (Stack)     : %p\n", (void *)&local_var);
    printf("heap_var ptr (Stack)   : %p   <- pointer variable itself\n", (void *)&heap_var);
    printf("heap_var     (Heap)    : %p   <- allocated memory\n", (void *)heap_var);

    unsigned long stack_addr = (unsigned long)&local_var;
    unsigned long heap_addr  = (unsigned long)heap_var;
    printf("\nStack - Heap difference: %lu bytes\n", stack_addr - heap_addr);

    free(heap_var);
    return 0;
}
