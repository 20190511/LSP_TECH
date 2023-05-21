#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>

void ssu_func(int loc_val, int loc_volatile, int loc_register);

int count;
static jmp_buf glob_buffer;

int main(void)
{
    register int loc_register;
    volatile int loc_volatile;
    int loc_val;
    int ret_val;

    loc_val = 10 ; loc_volatile = 11; loc_register = 13;
    if (setjmp(glob_buffer) != 0)
    {
        printf("after longjmp, loc_var = %d, loc_volatile = %d, loc_register = %d\n",
                loc_val, loc_volatile, loc_register);
        exit(0);
    }

    loc_val = 80; loc_volatile = 81 ; loc_register = 82;
    ssu_func (loc_val, loc_volatile, loc_register);
    exit(0);
}


void ssu_func(int loc_val, int loc_volatile, int loc_register)
{
    if (count == 3)
        longjmp(glob_buffer, 1);
    count++;
        printf("ssu_func, loc_var = %d, loc_volatile = %d, loc_register = %d\n",
                loc_val, loc_volatile, loc_register);
        ssu_func(loc_val+1, loc_volatile+1, loc_register+1);
        printf("ssu_func, loc_var = %d, loc_volatile = %d, loc_register = %d\n",
            loc_val, loc_volatile, loc_register);
}