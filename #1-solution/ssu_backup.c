#include "ssu_header.h"

int RecoverFile(char *originPath, char *backupPath, char *newPath) {
  fileNode *head = (fileNode *)malloc(sizeof(fileNode));
  fileNode *curr = head;
  struct dirent **namelist;
  struct stat tmpbuf;
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *filepath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *filename = (char *)malloc(sizeof(char *) * NAMEMAX);
  char *input = (char *)malloc(sizeof(char *) * STRMAX);
  char *date = (char *)malloc(sizeof(char *) * STRMAX);
  int fd1, fd2;
  char *buf = (char *)malloc(sizeof(char *) * STRMAX);
  int i;
  int idx;
  int cnt;
  int num;
  int len;
  
  strcpy(filepath, backupPath);
  for(idx = strlen(filepath)-1; filepath[idx] != '/'; idx--);

  strcpy(filename, filepath+idx+1);
  filepath[idx] = '\0';
  
  if((cnt = scandir(filepath, &namelist, NULL, alphasort)) == -1) {
		fprintf(stderr, "ERROR: scandir error for %s\n", filepath);
		return 1;
  }

  for(i = 0; i < cnt; i++) {
    if(!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")) continue;

    sprintf(tmpPath, "%s/%s", filepath, namelist[i]->d_name);

    if (lstat(tmpPath, &tmpbuf) < 0) {
      fprintf(stderr, "ERROR: lstat error for %s\n", tmpPath);
      return 1;
    }

    if(S_ISREG(tmpbuf.st_mode)) {
      strcpy(tmpPath, namelist[i]->d_name);
      tmpPath[strlen(tmpPath) - 13] = '\0';
      if(strcmp(tmpPath, filename)) continue;

      fileNode *new = (fileNode *)malloc(sizeof(fileNode));
      sprintf(new->path, "%s/%s", filepath, namelist[i]->d_name);
      new->statbuf = tmpbuf;
      curr->next = new;
      curr = curr->next;
    }
  }

  if(head->next == NULL) {
    printf("no backup file(s) of \"%s\"\n", originPath);
    return 1;
  } else if(head->next->next == NULL) {
    if(access(newPath, F_OK) != -1 && !cmpHash(newPath, head->next->path)) {
      printf("%s is not changed with %s\n", head->next->path, newPath);
      return 1;
    }

    if((fd1 = open(head->next->path, O_RDONLY)) < 0) {
      fprintf(stderr,"ERROR: open error for %s\n",head->next->path);	
      return 1;
    }

    if((fd2 = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 777)) < 0) {
      fprintf(stderr, "ERROR: open error for %s\n", newPath);
      return 1;
    }

    while((len = read(fd1, buf, head->next->statbuf.st_size)) > 0) {
      write(fd2, buf, len);
    }

    if(remove(head->next->path)) {
      fprintf(stderr, "ERROR: remove error for %s", head->next->path);
    }

    printf("\"%s\" backup recover to \"%s\"\n", head->next->path, newPath);
    free(head);
  } else {
    printf("backup files of \"%s\"\n", originPath);
    printf("0. exit\n");
    curr = head->next;
    for(i = 1; curr != NULL; curr = curr->next) {
      strcpy(date, curr->path + (strlen(curr->path) - 12));
      printf("%d. %s\t%sbytes\n", i, date, cvtNumComma(curr->statbuf.st_size));
      i++;
    }
    printf("Choose file to recover\n");
    
    while(true) {
      printf(">> ");
      fgets(input, sizeof(input), stdin);
      input[strlen(input) - 1] = '\0';

      num = atoi(input);
      if(num < 0 || num >= i) {
        printf("wrong input!\n");
        continue;
      }

      if(num == 0) return 1;

      curr = head->next;
      for(i = 1; curr != NULL; curr = curr->next) {
        if(i == num) {
          if(access(newPath, F_OK) != -1 && !cmpHash(newPath, curr->path)) {
            printf("%s is not changed with %s\n", curr->path, newPath);
            return 1;
          }

          if((fd1 = open(curr->path, O_RDONLY)) < 0) {
            fprintf(stderr,"ERROR: open error for %s\n", curr->path);	
            return 1;
          }

          if((fd2 = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 777)) < 0) {
            fprintf(stderr, "ERROR: open error for %s\n", newPath);
            return 1;
          }

          while((len = read(fd1, buf, curr->statbuf.st_size)) > 0) {
            write(fd2, buf, len);
          }

          if(remove(curr->path)) {
            fprintf(stderr, "ERROR: remove error for %s", curr->path);
          }

          printf("\"%s\" backup recover to \"%s\"\n", curr->path, newPath);
          free(curr);
          break;
        }
        i++;
      }

      break;
    }
  }
  return 0;
}

int RecoverDir(char *originPath, char *backupPath, char *newPath) {
  struct stat statbuf;
  struct dirent **namelist;
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpOriginPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *buf = (char *)malloc(sizeof(char *) * STRMAX);
  char *date = (char *)malloc(sizeof(char *) * STRMAX);
  char *input = (char *)malloc(sizeof(char *) * STRMAX);
  int cnt;
  int len;
  int fd1, fd2;
  int i;
  int num;
  dirNode *list = (dirNode *)malloc(sizeof(dirNode));
  list->head = (fileNode *)malloc(sizeof(fileNode));
  fileNode *curr = list->head;

  if (lstat(backupPath, &statbuf) < 0) {
    fprintf(stderr, "Usage : recover <FILENAME> [OPTION]\n");
    return 1;
  }

  if(access(newPath, F_OK))
    mkdir(newPath, 0777);
  
  if((cnt = scandir(backupPath, &namelist, NULL, alphasort)) == -1) {
		fprintf(stderr, "ERROR: scandir error for %s\n", backupPath);
		return 1;
  }

  for(int i = 0; i < cnt; i++) {
    if(!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")) continue;

    sprintf(tmpPath, "%s/%s", backupPath, namelist[i]->d_name);
    if (lstat(tmpPath, &statbuf) < 0) {
      fprintf(stderr, "ERROR: lstat error for %s\n", tmpPath);
      return 1;
    }

    if(S_ISDIR(statbuf.st_mode)) {
      dirNode *new = (dirNode *)malloc(sizeof(dirNode));
      sprintf(tmpPath, "%s/%s", originPath, namelist[i]->d_name);
      strcpy(new->path, tmpPath);
      sprintf(tmpPath, "%s/%s", backupPath, namelist[i]->d_name);
      strcpy(new->backupPath, tmpPath);
      sprintf(tmpPath, "%s/%s", newPath, namelist[i]->d_name);
      strcpy(new->newPath, tmpPath);
      mainDirList->tail->next = new;
      mainDirList->tail = mainDirList->tail->next;
    } else if(S_ISREG(statbuf.st_mode)) {
      sprintf(tmpOriginPath, "%s%s", homePATH, tmpPath + strlen(backupPATH));
      tmpOriginPath[strlen(tmpOriginPath) - 13] = '\0';

      backupNode *new = (backupNode *)malloc(sizeof(backupNode));
      sprintf(new->backupPath, "%s/%s", backupPath, namelist[i]->d_name);
      sprintf(new->newPath, "%s/%s", newPath, namelist[i]->d_name);
      new->newPath[strlen(new->newPath)-13] = '\0';
      if (lstat(new->backupPath, &(new->statbuf)) < 0) {
        fprintf(stderr, "ERROR: lstat error for %s\n", new->backupPath);
        return 1;
      }

      if(curr->next == NULL) {
        curr->next = (fileNode *)malloc(sizeof(fileNode));
        strcpy(curr->next->path, tmpOriginPath);
        curr->next->head = (backupNode *)malloc(sizeof(backupNode));
      } else if(strcmp(curr->next->path, tmpOriginPath)) {
        curr = curr->next;
        curr->next = (fileNode *)malloc(sizeof(fileNode));
        strcpy(curr->next->path, tmpOriginPath);
        curr->next->head = (backupNode *)malloc(sizeof(backupNode));
      }

      backupNode *currBackup = curr->next->head;
      while(currBackup->next != NULL) {
        currBackup = currBackup->next;
      }
      currBackup->next = new;
    }
  }

  curr = list->head->next;
  while(curr != NULL) {
    backupNode *currBackup = curr->head->next;
    if(currBackup->next == NULL) {
      if(access(currBackup->newPath, F_OK) != -1 && !cmpHash(currBackup->newPath, currBackup->backupPath)) {
        printf("%s is not changed with %s\n", currBackup->backupPath, currBackup->newPath);
        curr = curr->next;
        continue;
      }

      if((fd1 = open(currBackup->backupPath, O_RDONLY)) < 0) {
        fprintf(stderr,"ERROR: open error for %s\n",currBackup->backupPath);	
        return 1;
      }

      if((fd2 = open(currBackup->newPath, O_CREAT | O_TRUNC | O_WRONLY, 777)) < 0) {
        fprintf(stderr, "ERROR: open error for %s\n", currBackup->newPath);
        return 1;
      }

      while((len = read(fd1, buf, currBackup->statbuf.st_size)) > 0) {
        write(fd2, buf, len);
      }

      if(remove(currBackup->backupPath)) {
        fprintf(stderr, "ERROR: remove error for %s\n", currBackup->backupPath);
      }

      printf("\"%s\" backup recover to \"%s\"\n", currBackup->backupPath, currBackup->newPath);
    } else {
      printf("backup files of \"%s\"\n", curr->path);
      printf("0. exit\n");
      for(i = 1; currBackup != NULL; currBackup = currBackup->next) {
        strcpy(date, currBackup->backupPath + (strlen(currBackup->backupPath) - 12));
        printf("%d. %s\t%sbytes\n", i, date, cvtNumComma(currBackup->statbuf.st_size));
        i++;
      }
      printf("Choose file to recover\n");
      
      while(true) {
        printf(">> ");
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';

        num = atoi(input);
        if(num < 0 || num >= i) {
          printf("wrong input!\n");
          continue;
        }

        if(num == 0) return 1;

        currBackup = curr->head->next;
        for(i = 1; currBackup != NULL; currBackup = currBackup->next) {
          if(i == num) {
            if(access(currBackup->newPath, F_OK) != -1 && !cmpHash(currBackup->newPath, currBackup->backupPath)) {
              printf("%s is not changed with %s\n", currBackup->backupPath, currBackup->newPath);
              curr = curr->next;
              continue;
            }

            if((fd1 = open(currBackup->backupPath, O_RDONLY)) < 0) {
              fprintf(stderr,"ERROR: open error for %s\n",currBackup->backupPath);	
              return 1;
            }

            if((fd2 = open(currBackup->newPath, O_CREAT | O_TRUNC | O_WRONLY, 777)) < 0) {
              fprintf(stderr, "ERROR: open error for %s\n", currBackup->newPath);
              return 1;
            }

            while((len = read(fd1, buf, currBackup->statbuf.st_size)) > 0) {
              write(fd2, buf, len);
            }

            if(remove(currBackup->backupPath)) {
              fprintf(stderr, "ERROR: remove error for %s\n", currBackup->backupPath);
            }

            printf("\"%s\" backup recover to \"%s\"\n", currBackup->backupPath, currBackup->newPath);
            
            break;
          }
          i++;
        }

        break;
      }
    }
    curr = curr->next;
  }
}

int RecoverCommand(command_parameter *parameter) {
	struct stat statbuf;
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *backupPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *newPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char **newPathList = NULL;
  int newPathDepth = 0;
  int i;

  strcpy(originPath, parameter->filename);
  sprintf(backupPath, "%s%s", backupPATH, originPath + strlen(homePATH));
  if(parameter->commandopt & OPT_N) {
    strcpy(newPath, parameter->tmpname);
  } else {
    strcpy(newPath, originPath);
  }

  strcpy(tmpPath, newPath);
  if((newPathList = GetSubstring(tmpPath, &newPathDepth, "/")) == NULL) {
    fprintf(stderr, "ERROR: %s can't be backuped\n", newPath);
    return -1;
  }

  strcpy(tmpPath, "");
  for(i = 0; i < newPathDepth-1; i++) {
    strcat(tmpPath, "/");
    strcat(tmpPath, newPathList[i]);

    if(access(tmpPath, F_OK))
      mkdir(tmpPath, 0777);
  }

  if(parameter->commandopt & OPT_D) {
    mainDirList = (dirList *)malloc(sizeof(dirList));
    dirNode *head = (dirNode *)malloc(sizeof(dirNode));
    mainDirList->head = head;
    dirNode *curr = head->next;
    dirNode *new = (dirNode *)malloc(sizeof(dirNode));
    strcpy(new->path, originPath);
    strcpy(new->backupPath, backupPath);
    strcpy(new->newPath, newPath);
    curr = new;
    mainDirList->tail = curr;

    while(curr != NULL) {
      RecoverDir(curr->path, curr->backupPath, curr->newPath);
      curr = curr->next;
    }
  } else {
    RecoverFile(originPath, backupPath, newPath);
  }
}

int RemoveFile(char* path) {
  fileNode *head = (fileNode *)malloc(sizeof(fileNode));
  fileNode *curr = head;
	struct stat statbuf, tmpbuf;
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *filepath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *filename = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *date = (char *)malloc(sizeof(char *) * PATHMAX);
	struct dirent **namelist;
  int cnt;
  int idx;
  int i;
  char input[STRMAX];
  int num;

  sprintf(originPath, "%s%s", homePATH, path+strlen(backupPATH));

  strcpy(filepath, path);
  for(idx = strlen(filepath)-1; filepath[idx] != '/'; idx--);
  strcpy(filename, filepath+idx+1);
  filepath[idx] = '\0';

  if (lstat(filepath, &statbuf) < 0) {
    fprintf(stderr, "Usage : remove <FILENAME> [OPTION]\n");
    return 1;
  }

  if((cnt = scandir(filepath, &namelist, NULL, alphasort)) == -1) {
		fprintf(stderr, "ERROR: scandir error for %s\n", filepath);
		return 1;
  }

  for(i = 0; i < cnt; i++) {
    if(!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")) continue;

    sprintf(tmpPath, "%s/%s", filepath, namelist[i]->d_name);

    if (lstat(tmpPath, &tmpbuf) < 0) {
      fprintf(stderr, "ERROR: lstat error for %s\n", tmpPath);
      return 1;
    }

    if(S_ISREG(tmpbuf.st_mode)) {
      strcpy(tmpPath, namelist[i]->d_name);
      tmpPath[strlen(tmpPath) - 13] = '\0';
      if(strcmp(tmpPath, filename)) continue;

      fileNode *new = (fileNode *)malloc(sizeof(fileNode));
      sprintf(new->path, "%s/%s", filepath, namelist[i]->d_name);
      new->statbuf = tmpbuf;
      curr->next = new;
      curr = curr->next;
    }
  }

  if(head->next == NULL) {
    printf("no backup file(s) of \"%s\"\n", originPath);
    return 1;
  } else if(head->next->next == NULL) {
    if(remove(head->next->path)) {
      fprintf(stderr, "ERROR: remove error for %s", head->next->path);
    }

    printf("\"%s\" backup file removed\n", head->next->path);
    free(head);
  } else {
    printf("backup files of \"%s\"\n", originPath);
    printf("0. exit\n");
    curr = head->next;
    for(i = 1; curr != NULL; curr = curr->next) {
      strcpy(date, curr->path + (strlen(curr->path) - 12));
      printf("%d. %s\t%sbytes\n", i, date, cvtNumComma(curr->statbuf.st_size));
      i++;
    }
    printf("Choose file to recover\n");
    
    while(true) {
      printf(">> ");
      fgets(input, sizeof(input), stdin);
      input[strlen(input) - 1] = '\0';

      num = atoi(input);
      if(num < 0 || num >= i) {
        printf("wrong input!\n");
        continue;
      }

      if(num == 0) return 1;

      curr = head->next;
      for(i = 1; curr != NULL; curr = curr->next) {
        if(i == num) {
          if(remove(curr->path)) {
            fprintf(stderr, "ERROR: remove error for %s", curr->path);
          }

          printf("\"%s\" backup file removed\n", curr->path);
          free(curr);
          break;
        }
        i++;
      }

      break;
    }
  }
  return 0;
}

int RemoveAll(char* path, int flag, int *filecnt, int *dircnt) {
	struct stat statbuf;
  int cnt;
  struct dirent **namelist;
  char *tmpPath = (char *)malloc(sizeof(char) * PATHMAX);
  int i;

  if (lstat(path, &statbuf) < 0) {
    fprintf(stderr, "Usage : remove <FILENAME> [OPTION]\n");
    return 1;
  }

  if(S_ISDIR(statbuf.st_mode)) {
    if((cnt = scandir(path, &namelist, NULL, alphasort)) == -1) {
      fprintf(stderr, "ERROR: scandir error for %s\n", path);
      return 1;
    }

    for(i = 0; i < cnt; i++) {
      if(!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")) continue;

      sprintf(tmpPath, "%s/%s", path, namelist[i]->d_name);
      
      RemoveAll(tmpPath, flag, filecnt, dircnt);
    }
    if(strcmp(path, backupPATH)) {
      *dircnt += 1;
    }
  } else {
    if(flag == 1) {
      printf("\"%s\" backup file removed\n", path);
    }
    *filecnt += 1;
  }
  
  if(strcmp(path, backupPATH)) {
    remove(path);
  } else {
    if(*dircnt == 0 && *filecnt == 0) {
      printf("no file(s) in the backup\n");
    } else {
      printf("backup directory cleared(%d regular files and %d subdirectories totally).\n", *filecnt, *dircnt);
    }
  }

  return 0;
}

int RemoveCommand(command_parameter *parameter) {
	struct stat statbuf;
  int i;
  char *backuptime;
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *backupPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char **backupPathList = NULL;
  int backupPathDepth = 0;
  int flag = 0;
  int filecnt = 0;
  int dircnt = 0;

  strcpy(originPath, parameter->filename);
  sprintf(backupPath, "%s%s", backupPATH, originPath + strlen(homePATH));

  if(parameter->commandopt & OPT_C) {
    strcpy(backupPath, backupPATH);
    flag = 2;
  }

  if(parameter->commandopt & OPT_A) {
    flag = 1;
  }

  if(flag == 0) {
    RemoveFile(backupPath);
  } else {
    RemoveAll(backupPath, flag, &filecnt, &dircnt);
  }

  return 0;
}

int BackupFile(char *path, char *date) {
  int len;
  int fd1, fd2;
  char *buf = (char *)malloc(sizeof(char *) * STRMAX);
	struct stat statbuf, tmpbuf;
	struct dirent **namelist;
  int cnt;

  char *filename = (char *)malloc(sizeof(char *) * PATHMAX);
  char *filepath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpName = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpdir = (char *)malloc(sizeof(char *) * PATHMAX);
  char *newPath = (char *)malloc(sizeof(char *) * PATHMAX);
  int idx;
  int i;

  char *filehash = (char *)malloc(sizeof(char *) * hash);
  char *tmphash = (char *)malloc(sizeof(char *) * hash);

  strcpy(filepath, path);
  for(idx = strlen(filepath)-1; filepath[idx] != '/'; idx--);

  strcpy(filename, filepath+idx+1);
  filepath[idx] = '\0';

  if (lstat(path, &statbuf) < 0) {
    fprintf(stderr, "ERROR: lstat error for %s\n", path);
    return 1;
  }

  ConvertHash(path, filehash);

  sprintf(tmpdir, "%s%s", backupPATH, filepath+strlen(homePATH));

  if((cnt = scandir(tmpdir, &namelist, NULL, alphasort)) == -1) {
		fprintf(stderr, "ERROR: scandir error for %s\n", tmpdir);
		return 1;
  }

  for(i = 0; i < cnt; i++) {
    if(!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")) continue;

    sprintf(tmpPath, "%s/%s", tmpdir, namelist[i]->d_name);

    if (lstat(tmpPath, &tmpbuf) < 0) {
      fprintf(stderr, "ERROR: lstat error for %s\n", tmpPath);
      return 1;
    }

    if(S_ISREG(tmpbuf.st_mode)) {
      strcpy(tmpName, namelist[i]->d_name);
      tmpName[strlen(tmpName) - 13] = '\0';
      if(strcmp(tmpName, filename)) continue;
      if(statbuf.st_size != tmpbuf.st_size) continue;

      ConvertHash(tmpPath, tmphash);
      if(!strcmp(filehash, tmphash)) {
        printf("\"%s\" is already backuped\n", tmpPath);
        return 1;
      }
    }

    strcpy(tmpPath, namelist[i]->d_name);
  }

  sprintf(newPath, "%s%s_%s", backupPATH, path+strlen(homePATH), date);

  if((fd1 = open(path, O_RDONLY)) < 0) {
		fprintf(stderr,"ERROR: open error for %s\n",path);
		return 1;
  }

	if((fd2 = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 777)) < 0) {
		fprintf(stderr, "ERROR: open error for %s\n", newPath);
		return 1;
  }

	while((len = read(fd1, buf, statbuf.st_size)) > 0) {
		write(fd2, buf, len);
	}

  printf("\"%s\" backuped\n", newPath);
}

int BackupDir(char *path, char *date) {
	struct dirent **namelist;
	struct stat statbuf;
  char *tmppath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *tmpdir = (char *)malloc(sizeof(char *) * PATHMAX);
  int cnt;

  strcpy(tmpdir, backupPATH);
  strcat(tmpdir, path+strlen(homePATH));
  
  if(access(tmpdir, F_OK))
    mkdir(tmpdir, 0777);
  
  if((cnt = scandir(path, &namelist, NULL, alphasort)) == -1) {
		fprintf(stderr, "ERROR: scandir error for %s\n", path);
		return 1;
  }

  for(int i = 0; i < cnt; i++) {
    if(!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")) continue;

    strcpy(tmppath, path);
    strcat(tmppath, "/");
    strcat(tmppath, namelist[i]->d_name);
    if (lstat(tmppath, &statbuf) < 0) {
      fprintf(stderr, "ERROR: lstat error for %s\n", tmppath);
      return 1;
    }

    if(S_ISDIR(statbuf.st_mode)) {
      dirNode *new = (dirNode *)malloc(sizeof(dirNode));
      strcpy(new->path, tmppath);
      mainDirList->tail->next = new;
      mainDirList->tail = mainDirList->tail->next;
    } else if(S_ISREG(statbuf.st_mode)) {
      BackupFile(tmppath, date);
    }
  }
}

int AddCommand(command_parameter *parameter) {
	struct stat statbuf;
  char *backuptime;
  char *tmpPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *originPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char *newBackupPath = (char *)malloc(sizeof(char *) * PATHMAX);
  char **backupPathList = NULL;
  int backupPathDepth = 0;
  int i;

  strcpy(originPath, parameter->filename);

  if (lstat(originPath, &statbuf) < 0) {
    fprintf(stderr, "ERROR: lstat error for %s\n", originPath);
    return 1;
  }

  if(!S_ISREG(statbuf.st_mode) && !S_ISDIR(statbuf.st_mode)) {
    fprintf(stderr, "ERROR: %s is not directory or regular file\n", originPath);
    return -1;
  }

  if(S_ISDIR(statbuf.st_mode) && !(parameter->commandopt & OPT_D)) {
    fprintf(stderr, "ERROR: %s is a directory file\n", originPath);
    return -1;
  }


  sprintf(newBackupPath, "%s%s", backupPATH, originPath + strlen(homePATH));
  if((backupPathList = GetSubstring(newBackupPath, &backupPathDepth, "/")) == NULL) {
    fprintf(stderr, "ERROR: %s can't be backuped\n", originPath);
    return -1;
  }

  strcpy(tmpPath, "");
  for(i = 0; i < backupPathDepth-1; i++) {
    strcat(tmpPath, "/");
    strcat(tmpPath, backupPathList[i]);

    if(access(tmpPath, F_OK))
      mkdir(tmpPath, 0777);
  }
  
  if(S_ISREG(statbuf.st_mode)) {
    BackupFile(originPath, getDate());
  } else if(S_ISDIR(statbuf.st_mode)) {
    mainDirList = (dirList *)malloc(sizeof(dirList));
    dirNode *head = (dirNode *)malloc(sizeof(dirNode));
    mainDirList->head = head;
    dirNode *curr = head->next;
    dirNode *new = (dirNode *)malloc(sizeof(dirNode));
    strcpy(new->path, originPath);
    curr = new;
    mainDirList->tail = curr;

    while(curr != NULL) {
      BackupDir(curr->path, getDate());
      curr = curr->next;
    }
  }
  
  return 0;
}

void CommandFun(char **arglist) {
  int (*commandFun)(command_parameter * parameter);
	command_parameter parameter={
    arglist[0], arglist[1], arglist[2], atoi(arglist[3])
  };

  if(!strcmp(parameter.command, commanddata[0])) {
    commandFun = AddCommand;
  } else if(!strcmp(parameter.command, commanddata[1])) {
    commandFun = RemoveCommand;
  } else if(!strcmp(parameter.command, commanddata[2])) {
    commandFun = RecoverCommand;
  }

  if(commandFun(&parameter) != 0) {
    exit(1);
  }
}

void CommandExec(command_parameter parameter) {
  pid_t pid;

  parameter.argv[0] = "command";
  parameter.argv[1] = (char *)malloc(sizeof(char *) * 32);
  sprintf(parameter.argv[1], "%d", hash);

  parameter.argv[2] = parameter.command;
  parameter.argv[3] = parameter.filename;
  parameter.argv[4] = parameter.tmpname;
  parameter.argv[5] = (char *)malloc(sizeof(char *) * 32);
  sprintf(parameter.argv[5], "%d", parameter.commandopt);
  parameter.argv[6] = (char *)0;

  if((pid = fork()) < 0) {
    fprintf(stderr, "ERROR: fork error\n");
    exit(1);
  } else if(pid == 0) {
    execv(exeNAME, parameter.argv);
    exit(0);
  } else {
    pid = wait(NULL);
  }
}

void SystemExec(char **arglist) {
  pid_t pid;
  char whichPath[PATHMAX];

  sprintf(whichPath, "/usr/bin/%s", arglist[0]);

  if((pid = fork()) < 0) {
    fprintf(stderr, "ERROR: fork error\n");
    exit(1);
  } else if(pid == 0) {
    execv(whichPath, arglist);
    exit(0);
  } else {
    pid = wait(NULL);
  }
}

void HelpExec() {
  pid_t pid;

  if((pid = fork()) < 0) {
    fprintf(stderr, "ERROR: fork error\n");
    exit(1);
  } else if(pid == 0) {
    execl(exeNAME, "help", (char *)0);
    exit(0);
  } else {
    pid = wait(NULL);
  }
}

void ParameterInit(command_parameter *parameter) {
  parameter->command = (char *)malloc(sizeof(char *) * PATH_MAX);
  parameter->filename = (char *)malloc(sizeof(char *) * PATH_MAX);
  parameter->tmpname = (char *)malloc(sizeof(char *) * PATH_MAX);
	parameter->commandopt = 0;
}

int ParameterProcessing(int argcnt, char **arglist, int command, command_parameter *parameter) {
	struct stat buf;
  optind = 0;
  opterr = 0;
	int lastind;
	int option;
	int optcnt = 0;

	switch(command) {
		case CMD_ADD: {
			if (argcnt < 2) {
				fprintf(stderr, "Usage : %s <FILENAME> [OPTION]\n", arglist[0]);
				return -1;
			}

      if(ConvertPath(arglist[1], parameter->filename) != 0) {
        fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
        return -1;
      }

      if(strncmp(parameter->filename, homePATH, strlen(homePATH))
      || !strncmp(parameter->filename, backupPATH, strlen(backupPATH))
      || !strcmp(parameter->filename, homePATH)) {
        fprintf(stderr, "ERROR: %s can't be backuped\n", parameter->filename);
        return -1;
      }

      if (lstat(parameter->filename, &buf) < 0) {
        fprintf(stderr, "ERROR: lstat error for %s\n", parameter->filename);
        return -1;
      }

      if(!S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode)) {
        fprintf(stderr, "ERROR: %s is not regular file\n", parameter->filename);
        return -1;
      }

			lastind = 2;

			while((option = getopt(argcnt, arglist, "d")) != -1) {
        if(option != 'd') {
          fprintf(stderr, "ERROR: unknown option %c\n", optopt);
          return -1;
        }

        if(optind == lastind) {
          fprintf(stderr, "ERROR: wrong option input\n");
          return -1;
        }

        if(option == 'd')	{
          if(parameter->commandopt & OPT_D) {
            fprintf(stderr, "ERROR: duplicate option -%c\n", option);
            return -1;
          }
          parameter->commandopt |= OPT_D;
        }

        optcnt++;
				lastind = optind;
			}
			if(argcnt - optcnt != 2) {
				fprintf(stderr, "ERROR: argument error\n");
				return -1;
			}
			break;
    }
		case CMD_REM: {
			if (argcnt < 2) {
				fprintf(stderr, "Usage : %s <FILENAME> [OPTION]\n", arglist[0]);
				return -1;
			}
      
      parameter->filename = arglist[1];

			lastind = 1;

			while((option = getopt(argcnt, arglist, "ac")) != -1) {
        if(option != 'a' && option != 'c') {
          fprintf(stderr, "ERROR: unknown option %c\n", optopt);
          return -1;
        }

        if(optind == lastind) {
          fprintf(stderr, "ERROR: wrong option input\n");
          return -1;
        }
        
        if(option == 'a')	{
          if(parameter->commandopt & OPT_A) {
            fprintf(stderr, "ERROR: duplicate option -%c\n", option);
            return -1;
          }
          parameter->commandopt |= OPT_A;
        }
        
        if(option == 'c')	{
          if(parameter->commandopt & OPT_C) {
            fprintf(stderr, "ERROR: duplicate option -%c\n", option);
            return -1;
          }
          parameter->commandopt |= OPT_C;
        }

        optcnt++;
				lastind = optind;
			}

      if(parameter->commandopt & OPT_A && parameter->commandopt & OPT_C) {
				fprintf(stderr, "ERROR: option -a and -c can't use concurrency\n");
				return -1;
      }
      
			if(((parameter->commandopt & OPT_A) && argcnt - optcnt != 2)
      || ((parameter->commandopt & OPT_C) && argcnt - optcnt != 1)) {
				fprintf(stderr, "ERROR: argument error\n");
				return -1;
			}

      if(parameter->commandopt & OPT_C) {
        break;
      }

      if(ConvertPath(parameter->filename, parameter->filename) != 0) {
        fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
        return -1;
      }

      if(strncmp(parameter->filename, homePATH, strlen(homePATH))
      || !strncmp(parameter->filename, backupPATH, strlen(backupPATH))
      || !strcmp(parameter->filename, homePATH)) {
        fprintf(stderr, "ERROR: %s can't be backuped\n", parameter->filename);
        return -1;
      }
      
			break;
    }
		case CMD_REC: {
			if (argcnt < 2) {
				fprintf(stderr, "Usage : %s <FILENAME> [OPTION]\n", arglist[0]);
				return -1;
			}
      
      if(ConvertPath(arglist[1], parameter->filename) != 0) {
        fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->filename);
        return -1;
      }

      if(strncmp(parameter->filename, homePATH, strlen(homePATH))
      || !strncmp(parameter->filename, backupPATH, strlen(backupPATH))
      || !strcmp(parameter->filename, homePATH)) {
        fprintf(stderr, "ERROR: %s can't be backuped\n", parameter->filename);
        return -1;
      }

			lastind = 2;

			while((option = getopt(argcnt, arglist, "dn:")) != -1) {
        if(option != 'd' && option != 'n') {
          fprintf(stderr, "ERROR: unknown option %c\n", optopt);
          return -1;
        }

        if(optind == lastind) {
          fprintf(stderr, "ERROR: wrong option input\n");
          return -1;
        }

        if(option == 'd')	{
          if(parameter->commandopt & OPT_D) {
            fprintf(stderr, "ERROR: duplicate option -%c\n", option);
            return -1;
          }
          parameter->commandopt |= OPT_D;
        }

        if(option == 'n')	{
          if(parameter->commandopt & OPT_N) {
            fprintf(stderr, "ERROR: duplicate option -%c\n", option);
            return -1;
          }

          if(optarg == NULL) {
            fprintf(stderr, "ERROR: <NEWNAME> is null\n");
            return -1;
          }

          if(ConvertPath(optarg, parameter->tmpname) != 0) {
            fprintf(stderr, "ERROR: %s is invalid filepath\n", parameter->tmpname);
            return -1;
          }

          if(strncmp(parameter->tmpname, homePATH, strlen(homePATH))
          || !strncmp(parameter->tmpname, backupPATH, strlen(backupPATH))
          || !strcmp(parameter->tmpname, homePATH)) {
            fprintf(stderr, "ERROR: %s can't be backuped\n", parameter->tmpname);
            return -1;
          }

          parameter->commandopt |= OPT_N;
          optcnt++;
        }

        optcnt++;
				lastind = optind;
			}
			if(argcnt - optcnt != 2) {
				fprintf(stderr, "argument error\n");
				return -1;
			}
			break;
    }
  }
}

int Prompt() {
  char input[STRMAX];
	int argcnt = 0;
	char **arglist = NULL;
  int command;
	command_parameter parameter={(char *)0, (char *)0, (char *)0, 0};

  while(true) {
    printf("20230000> ");
    fgets(input, sizeof(input), stdin);
    input[strlen(input) - 1] = '\0';

		if((arglist = GetSubstring(input, &argcnt, " \t")) == NULL) {
      continue;
    }

    if(argcnt == 0) continue;

		if(!strcmp(arglist[0], commanddata[0])) {
			command = CMD_ADD;
		} else if(!strcmp(arglist[0], commanddata[1])) {
			command = CMD_REM;
		}	else if(!strcmp(arglist[0], commanddata[2])) {
			command = CMD_REC;
		}	else if(!strcmp(arglist[0], commanddata[3])) {
			command = CMD_SYS;
		}	else if(!strcmp(arglist[0], commanddata[4])) {
			command = CMD_SYS;
		}	else if(!strcmp(arglist[0], commanddata[5])) {
			command = CMD_SYS;
		}	else if(!strcmp(arglist[0], commanddata[6])) {
			command = CMD_HELP;
		}	else if(!strcmp(arglist[0], commanddata[7])) {
			command = CMD_EXIT;
      fprintf(stdout, "Program exit(0)\n");
      exit(0);
		}	else {
			command = NOT_CMD;
		}

    if(command & (CMD_ADD | CMD_REM | CMD_REC)) {
		  ParameterInit(&parameter);
      parameter.command = arglist[0];
      if(ParameterProcessing(argcnt, arglist, command, &parameter) == -1) {
        continue;
      }

      CommandExec(parameter);

    } else if(command & CMD_SYS) {
      SystemExec(arglist);
    } else if(command & CMD_HELP || command == NOT_CMD) {
      HelpExec();
    }
  }
}

void Init() {
  getcwd(exePATH, PATHMAX);
  sprintf(homePATH, "%s", getenv("HOME"));
  sprintf(backupPATH, "%s/backup", getenv("HOME"));
  
	if (access(backupPATH, F_OK))
		mkdir(backupPATH, 0777);
}

int main(int argc, char* argv[]) {
  Init();

  if(!strcmp(argv[0], "command")) {
    hash = atoi(argv[1]);

    CommandFun(argv+2);
  } else if(!strcmp(argv[0], "help")) {
    help();
  } else {
    if (argc < 2) {
      fprintf(stderr, "usage : %s <md5 | sha1>\n", argv[0]);
      return -1;
    }

    strcpy(exeNAME, argv[0]);

    if(strcmp(argv[1], "md5") && strcmp(argv[1], "sha1")) {
      fprintf(stderr, "input error: wrong hash <md5 | sha1>\n");
      return -1;
    }
    
    if(!strcmp(argv[1], "md5")) {
      hash = HASH_MD5;
    }
    if(!strcmp(argv[1], "sha1")) {
      hash = HASH_SHA1;
    }

    Prompt();
  }
	
	exit(0);
}