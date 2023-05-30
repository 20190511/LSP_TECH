#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#define MAXPATHLEN 1024

FILE *old_fp;
FILE *new_fp;

void search_files (char* dir_path, time_t compare_time) {
	DIR* dir;
	struct dirent *dt;
	struct stat sb;
	int i,j = 0;
	char tmp[MAXPATHLEN];
	strcpy(tmp ,dir_path);
	
	if (lstat(tmp ,&sb) < 0) {
		fprintf(stderr, "lstat error for %s\n", tmp);
		exit(1);
	}
	
	if (!S_ISDIR (sb.st_mode)) {
		fprintf(stderr, "%s is not diretory file\n", tmp);
		exit(1);
	}
	
	char *ptr = tmp + strlen(dir_path);
	*ptr++ = '/';
	*ptr = '\0';

	if ((dir = opendir(tmp)) == NULL) {
		fprintf(stderr, "opendir error for %s\n", tmp);
		exit(1);
	}

	while ((dt = readdir(dir)) != NULL) {
		if (strcmp(dt->d_name, ".") && strcmp(dt->d_name, "..")) {

			strcpy(ptr, dt->d_name);
			//free(dt);
			if (lstat(tmp ,&sb) < 0) {
				fprintf(stderr, "lstat error for %s\n", tmp);
				exit(1);
			}

			if (S_ISDIR(sb.st_mode)) 
				search_files (tmp, compare_time);
			else {
				time_t mtime = sb.st_mtime;
				time_t ctime = sb.st_ctime;
				time_t atime = sb.st_atime;
				int f_size = sb.st_size;
				
				if (ctime <= compare_time)
					fprintf(old_fp, "%20s\t %ld\t %ld\t %ld\t %d\n", tmp, mtime, ctime, atime, f_size);
				else
					fprintf(new_fp, "%20s\t %ld\t %ld\t %ld\t %d\n", tmp, mtime, ctime, atime, f_size);

			}
		}
	}

	closedir(dir);
}

int main(int argc, char* argv[])
{

	if (argc < 3) {
		fprintf(stderr, "usage: %s <file> <time>\n", argv[0]);
		exit(1);
	}

	if ((old_fp = fopen("old.file", "w")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", "old.file");
		exit(1);
	}

	if ((new_fp = fopen("new.file", "w")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", "new.file");
		exit(1);
	}
	
	fprintf(old_fp, "%20s\t %s\t %s\t %s\t %s\n", "Path", "Modified Time", "Changed Time", "Accessed Time", "File Size");
	fprintf(new_fp, "%20s\t %s\t %s\t %s\t %s\n", "Path", "Modified Time", "Changed Time", "Accessed Time", "File Size");

	search_files(argv[1], atoi(argv[2]));
	fclose(old_fp);
	fclose(new_fp);
	exit(0);
}
