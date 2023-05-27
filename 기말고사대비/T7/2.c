#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>

void ssu_func (int loc_val, int loc_volatile, int loc_register);

static jmp_buf glob_buf;

int main(void)
{
	int loc_val = 10;
	volatile int loc_volatile =11;
	register int loc_register = 13;

	if (setjmp(glob_buf) != 0) {
		printf("after longjmp, loc_val = %d, loc_volatile = %d, loc_register = %d\n",
				loc_val, loc_volatile, loc_register);
		exit(0);
	}

	loc_val = 80;
	loc_volatile = 81;
	loc_register = 82;
	ssu_func(loc_val, loc_volatile, loc_register);
	exit(0);
	
}

void ssu_func (int loc_val, int loc_volatile, int loc_register)
{
	printf("before longjmp, loc_val = %d, loc_volatile = %d, loc_register = %d\n",
			loc_val, loc_volatile, loc_register);
	longjmp(glob_buf, 1);
}
