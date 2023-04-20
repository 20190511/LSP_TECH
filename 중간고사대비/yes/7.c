#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void ssu_setbuf(FILE *fp, char* buf);

int main(void)
{
	char buf[BUFFER_SIZE];
	char *fname = "/dev/pts/1";
	FILE *fp;

	if ((fp = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname);
		exit(1);
	}

	ssu_setbuf(fp, buf);
	fprintf(fp,"Hello ");
	sleep(1);
	fprintf(fp,"UNIX!! ");
	sleep(1);
	fprintf(fp,"\n");
	sleep(1);

	ssu_setbuf(fp, NULL);
	fprintf(fp,"How ");
	sleep(1);
	fprintf(fp,"are ");
	sleep(1);
	fprintf(fp,"you? ");
	sleep(1);
	fprintf(fp,"\n");
	sleep(1);
	fclose(fp);
	exit(1);
}

void ssu_setbuf(FILE *fp, char* buf)
{
	int fd;
	size_t size;
	int mode;

	fd = fileno(fp);

	if(isatty(fd))
		mode = _IOLBF;
	else
		mode = _IOFBF;

	if (buf == NULL)
	{
		mode =_IONBF;
		size = 0;
	}
	else
		size = BUFFER_SIZE;

	setvbuf(fp, buf, mode, size);
}
