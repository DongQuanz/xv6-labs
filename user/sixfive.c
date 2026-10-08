#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void
processStr(char *str)
{
  int n = atoi(str);
  if (n % 5 == 0 || n % 6 == 0) {
    printf("%d\n", n);
  }
}

int
readFromSrc(int fd)
{
  int n;
  char buf[500];

  int cnt = 0;
  char num[32];
  int flag = 1;

  char sep[] = " -\r\t\n./,";

  while ((n = read(fd, buf, sizeof(buf))) > 0) {

    for (int i = 0; i < n; i++) {

      if (flag && buf[i] >= '0' && buf[i] <= '9') {
        num[cnt++] = buf[i];
      } else if (strchr(sep, buf[i]) != 0) {

        if (flag && cnt > 0) {
          num[cnt] = '\0';
          processStr(num);
        }

        cnt = 0;
        flag = 1;
      } else {
        flag = 0;
      }
    }
  }

  if (n < 0) {
    return 1;
  } else {
    if (flag && cnt > 0) {
      num[cnt] = '\0';
      processStr(num);
    }
  }

  close(fd);
  return 0;
}

int
main(int argc, char *argv[])
{

  if (argc == 1) {
    return readFromSrc(0);
  }

  for (int x = 1; x < argc; x++) {
    int fd = open(argv[x], O_RDONLY);
    if (fd < 0) {
      fprintf(2, "sixfive: cannot open %s\n", argv[x]);
      continue;
    }

    int fail = readFromSrc(fd);
    if (fail > 0) {
      fprintf(2, "sixfive: reading from %s failed\n", argv[x]);
      continue;
    }
  }

  return 0;
}