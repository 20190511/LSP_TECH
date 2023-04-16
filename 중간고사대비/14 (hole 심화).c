#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int ssu_write(int fd, char* buf);

int main(void)
{
	char buf[128];
	char pattern[4] = "bcd";
	char *pos1 = buf, *pos2 = buf;
	char *fname_in = "in.txt";
	char *fname_out= "out.txt";
	int size;
	int fd1, fd2;
	int j = 0;

	if ((fd1=open(fname_in, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s", fname_in);
		exit(1);
	}

	if((size = lseek(fd1, 0, SEEK_END)) < 0) {
		fprintf(stderr, "lseek error\n");
		exit(1);
	}
	
	lseek(fd1, 0, SEEK_SET);
	if (read(fd1, buf, size) < 0) {
		fprintf(stderr, "read error\n");
		exit(1);
	}

	close(fd1);

	if ((fd2=open(fname_out, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
		fprintf(stderr, "open error for %s", fname_out);
		exit(1);
	}
	
	while ((pos2 = strstr(pos1, pattern)) != NULL) {
		*pos2 = '\0';
		ssu_write(fd2, pos1);
		lseek(fd2, strlen(pattern), SEEK_CUR);
		pos1 = pos2 + strlen(pattern);
	}
	ssu_write(fd2, pos1);
	close(fd2);
	exit(0);
}

int ssu_write(int fd, char*buf) {
	return write(fd, buf, strlen(buf));
}
