#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>


#define OLD "old.file"
#define NEW "new.file"
FILE *old_fp;
FILE *new_fp;

void search_files (char* dir_path, time_t compare_time) {

	DIR *dir;
	struct dirent *dent;
	char *ptr;
	char cur_path [4096];
	strcpy(cur_path, dir_path);
	struct stat statbuf;

	if (lstat(dir_path, &statbuf) < 0) {
		fprintf(stderr, "lstat error\n");
		exit(1);
	}

	if (!S_ISDIR(statbuf.st_mode)) {
		fprintf(stderr, "%s is not direcotry\n", dir_path);
		exit(1);
	}

	if ((dir = opendir(dir_path)) == NULL) {
		fprintf(stderr, "opendir error for %s\n", dir_path);
		exit(1);
	}

	ptr = cur_path + strlen(cur_path);
	*ptr++ = '/';
	*ptr = '\0';
	while ((dent = readdir(dir)) != NULL) {
		if (strcmp(dent->d_name, ".") && strcmp(dent->d_name, "..")) {
			strcpy(ptr, dent->d_name);

			if (lstat(cur_path, &statbuf) < 0) {
				fprintf(stderr, "lstat error\n");
				exit(1);
			}
			
			if (S_ISDIR(statbuf.st_mode))
				search_files(cur_path, compare_time);
			else {
				char print_time [6000];
				sprintf(print_time, "%20s\t %ld\t %ld\t %ld\t %ld\n", cur_path, statbuf.st_mtime,
						statbuf.st_ctime, statbuf.st_atime, statbuf.st_size);
				
				if (statbuf.st_ctime <= compare_time)
					fputs(print_time, old_fp);
				else 
					fputs(print_time, new_fp);
			}
		}
	}


	
}


int main(int argc, char* argv[])
{
	
	if (argc < 3) {
		fprintf(stderr, "usage: %s <PATH> <TIME>\n", argv[0]);
		exit(1);
	}

	if((old_fp = fopen(OLD, "w")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}
	if((new_fp = fopen(NEW, "w")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}

	fprintf(old_fp, "%20s\t %s\t %s\t %s\t %s\n", "Path", "Modified Time", "Changed Time", "Accessed Time", "File Size");
	fprintf(new_fp, "%20s\t %s\t %s\t %s\t %s\n", "Path", "Modified Time", "Changed Time", "Accessed Time", "File Size");
	
	search_files(argv[1], atoi(argv[2]));
	fclose(old_fp);
	fclose(new_fp);
	exit(0);
}
