#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define TABLE_SIZE	128
#define BUFFER_SIZE	 1024

int main (int argc, char* argv[])
{
	static struct {
		long offset;
		int length;
	} table [TABLE_SIZE];
	char buf [BUFFER_SIZE];
	long offset;
	int entry;
	int i;
	int length;
	int fd;

	if (argc < 2)
	{
		fprintf(stderr, "usage : %s <file>\n", argv[0]);
		exit(1);
	}

	entry = 0;
	offset = 0;
	while ((length = read(fd, buf, BUFFER_SIZE)) > 0)
	{
		for (i = 0 ; i < length ; i++)
		{
			table[entry].length++;
			offset++;

			if (buf[i] == '\n')
				table[++entry].offset = offset;
		}
	}

#ifdef DEBUG
	for (i = 0 ; i < entry ; i++)
		printf("%d:%d, %ld", i+1, table[i].offset, table[i].length);
#endif

	while (1)
	{
		printf("Enter line number : ");
		scanf("%d", &length);

		if (--length < 0)
			break;

		lseek(fd, table[length].offset, 0);

		if ( read(fd, buf, table[length].length) <= 0)
			continue;
		buf[table[length].length] = '\0';
		printf("%s", buf);
	}
	close(fd);
	exit(0);
}


