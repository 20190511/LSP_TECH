#include <stdio.h>

void help(void){
    printf("Usage:\n");
    printf("  > add <FILENAME> [OPTION]\n");
    printf("    -d : add directory recursive\n");
    printf("  > remove <FILENAME> [OPTION]\n");
    printf("    -a : remove all file(recureive)\n");
    printf("    -c : remove all backups\n");
    printf("  > recover <FILENAME> [OPTION]\n");
    printf("    -d : recover directory recursive\n");
    printf("    -n <NEWNAME> : recover file with new name\n");
    printf("  > ls\n");
    printf("  > vi\n");
    printf("  > vim\n");
    printf("  > help\n");
    printf("  > exit\n\n");
}