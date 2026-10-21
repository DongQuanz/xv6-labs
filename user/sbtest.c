#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "user/user.h"

static void
report(char *what, int fd)
{
  if (fd < 0) {
    printf("sbtest: forkpath child %s denied\n", what);
  } else {
    printf("sbtest: forkpath child %s ok\n", what);
    close(fd);
  }
}

int
main(int argc, char *argv[])
{
  int r;

  if (argc != 2) {
    fprintf(2, "Usage: sbtest getpid|fork|escape|forkpath|interpose\n");
    exit(1);
  }

  // getpid and fork take no argument, so a kernel that blocks a call by
  // corrupting its arguments cannot make them fail.
  if (strcmp(argv[1], "getpid") == 0) {
    printf("sbtest: getpid ret %d\n", getpid());
  } else if (strcmp(argv[1], "fork") == 0) {
    r = fork();
    if (r == 0) {
      printf("sbtest: CHILD RAN\n");
      exit(0);
    }
    if (r > 0)
      wait(0);
    printf("sbtest: fork ret %d\n", r);
  } else if (strcmp(argv[1], "escape") == 0) {
    interpose(0, 0);
    interpose(0, "README");
    r = open("README", O_RDONLY);
    if (r < 0) {
      printf("sbtest: escape confined\n");
    } else {
      printf("sbtest: escape ESCAPED\n");
      close(r);
    }
  } else if (strcmp(argv[1], "interpose") == 0) {
    char longpath[MAXPATH + 1];
    char *badpaths[] = {0, "", (char *)-1, longpath};
    char *paths[] = {0, "", longpath, "-", "README", "exec.sh"};
    int mask = 1 << SYS_open;

    memset(longpath, 'x', MAXPATH);
    longpath[MAXPATH] = 0;
    for (int i = 0; i < sizeof(badpaths) / sizeof(badpaths[0]); i++) {
      if (interpose(mask, badpaths[i]) != -1)
        exit(1);
      r = open("README", O_RDONLY);
      if (r < 0)
        exit(1);
      close(r);
    }

    if (interpose(mask, "README") != 0)
      exit(1);
    for (int i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
      if (interpose(0, paths[i]) != -1 ||
          interpose(mask, paths[i]) != -1)
        exit(1);
      r = open("README", O_RDONLY);
      if (r < 0)
        exit(1);
      close(r);
      r = open("exec.sh", O_RDONLY);
      if (r != -1)
        exit(1);
    }
    printf("sbtest: interpose state preserved\n");
  } else if (strcmp(argv[1], "forkpath") == 0) {
    r = fork();
    if (r == 0) {
      report("README", open("README", O_RDONLY));
      report("exec.sh", open("exec.sh", O_RDONLY));
      exit(0);
    }
    if (r > 0)
      wait(0);
  } else {
    fprintf(2, "sbtest: unknown test %s\n", argv[1]);
    exit(1);
  }

  printf("sbtest: alive\n");
  exit(0);
}
