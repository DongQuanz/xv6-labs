#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void
runCommand(char *path, char **command, int commandCount)
{
  char *args[MAXARG];
  int i;

  for (i = 0; i < commandCount; i++)
    args[i] = command[i];

  args[commandCount] = path;
  args[commandCount + 1] = 0;

  int pid = fork();

  if (pid < 0) {
    fprintf(2, "find: fork failed\n");
    return;
  }

  if (pid == 0) {
    exec(args[0], args);
    fprintf(2, "find: exec %s failed\n", args[0]);
    exit(1);
  }

  wait(0);
}

void
lookInside(char *path, char *target, char **command, int commandCount)
{

  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_DEVICE:
    break;
  case T_FILE:
    p = path;
    for (char *s = path; *s; s++) {
      if (*s == '/')
        p = s + 1;
    }
    if (strcmp(p, target) == 0) {
      if (commandCount > 0)
        runCommand(path, command, commandCount);
      else
        printf("%s\n", path);
    }
    break;
  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      printf("find: path too long\n");
      break;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
        continue;

      if (strcmp(p, target) == 0) {
        if (commandCount > 0)
          runCommand(buf, command, commandCount);
        else
          printf("%s\n", buf);
      }

      if (stat(buf, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", buf);
        continue;
      }

      if (st.type == T_DIR)
        lookInside(buf, target, command, commandCount);
    }
    break;
  }
  close(fd);
  return;
}

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "find: too few arguments\n");
    return 1;
  }

  if (argc == 3) {
    lookInside(argv[1], argv[2], 0, 0);
    return 0;
  }

  if (strcmp(argv[3], "-exec") != 0 || argc < 5) {
    fprintf(2, "find: invalid arguments\n");
    return 1;
  }

  lookInside(argv[1], argv[2], &argv[4], argc - 4);
  return 0;
}
