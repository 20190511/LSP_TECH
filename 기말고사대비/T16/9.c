#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define NAME_SIZE 256
FILE* old_fp;
FILE* new_fp;

void search_files (char* dir_path, time_t compare_time) {
	char tmp[NAME_SIZE];	
	strcpy(tmp, dir_path);
	DIR* dir;
	struct dirent* dt;
	struct stat sb;
	char* ptr;

	if (lstat(tmp, &sb) < 0){ 
		fprintf(stderr, "lstat error for %s\n", tmp);
		exit(1);
	}

	if (!S_ISDIR(sb.st_mode)) {
		fprintf(stderr, "%s is not directory file\n", tmp);
		exit(1);
	}

	ptr = tmp + strlen(tmp);
	*ptr++ = '/';
	*ptr = 0;
	
	if ((dir = opendir(tmp)) == NULL) {
		fprintf(stderr, "opendir error for %s\n", tmp);
		exit(1);
	}

	while ((dt = readdir(dir)) != NULL ){
		if (strcmp(dt->d_name, ".") && strcmp(dt->d_name, "..")) {
			strcpy(ptr, dt->d_name);
			
			if (lstat(tmp, &sb) < 0) {
				fprintf(stderr, "lstat error for %s\n", tmp);
				exit(1);
			}

			if (S_ISDIR(sb.st_mode))
				search_files(tmp, compare_time);
			else {
				time_t mtime = sb.st_mtime;
				time_t ctime = sb.st_ctime;
				time_t atime = sb.st_atime;

				if (ctime <= compare_time) 
					fprintf(old_fp, "%20s\t %ld\t %ld\t %ld\t %ld\n", tmp, mtime, ctime, atime, sb.st_size);
				else
					fprintf(new_fp, "%20s\t %ld\t %ld\t %ld\t %ld\n", tmp, mtime, ctime, atime, sb.st_size);

			}
		}
	}
	closedir(dir);

}

int main (int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <directory file> <compare time>\n", argv[0]);
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
