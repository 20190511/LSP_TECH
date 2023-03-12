#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	char character;
	int fd;
	int line_count = 0;



	if ((fd = open("ssu_test.txt", O_RDONLY)) < 0)
	{
		fprintf(stderr, "open error for %s\n", "ssu_test.txt");
		exit(1);
	}

	while (1)
	{
		if (read(fd, &character, 1) > 0)
		{
			if (character == '\n')
				line_count++;
		}
		else
			break;
	}

	printf("total line : %d \n", line_count);
	exit(0);
}
		
