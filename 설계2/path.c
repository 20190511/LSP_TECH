int ConvertPath(char* origin, char* resolved) {
  int idx = 0;
  int i;
  char *path = (char *)malloc(sizeof(char *) * PATH_MAX);
  char *tmppath = (char *)malloc(sizeof(char *) * PATH_MAX);
  char **pathlist;
  int pathcnt;

  if(origin == NULL) {
    return -1;
  }

  if(origin[0] == '~') {
    sprintf(path, "%s%s", homePATH, origin+1);
  } else if(origin[0] != '/') {
    sprintf(path, "%s/%s", exePATH, origin);
  } else {
    sprintf(path, "%s", origin);
  }

  if(!strcmp(path, "/")) {
    resolved = "/";
    return 0;
  }

  if((pathlist = GetSubstring(path, &pathcnt, "/")) == NULL) {
    return -1;
  }

  pathList *headpath = (pathList *)malloc(sizeof(pathList));
  pathList *currpath = headpath;

  for(i = 0; i < pathcnt; i++) {
    if(!strcmp(pathlist[i], ".")) {
      continue;
    } else if(!strcmp(pathlist[i], "..")) {
      currpath = currpath->prev;
      currpath->next = NULL;
      continue;
    }

    pathList *newpath = (pathList *)malloc(sizeof(pathList));
    strcpy(newpath->path, pathlist[i]);
    currpath->next = newpath;
    newpath->prev = currpath;

    currpath = currpath->next;
  }

  currpath = headpath->next;

  strcpy(tmppath, "/");
  while(currpath != NULL) {
    strcat(tmppath, currpath->path);
    if(currpath->next != NULL) {
      strcat(tmppath, "/");
    }
    currpath = currpath->next;
  }

  strcpy(resolved, tmppath);

  return 0;
}
