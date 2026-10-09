#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data, int len);

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else if (argc == 2) {
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while (n < sizeof(data)) {
      int nn = read(0, data + n, sizeof(data) - n);
      if (nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data, n);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

void
memdump(char *fmt, char *data, int len)
{
  for (; *fmt; fmt++) {
    if (*fmt == 'i') {
      if (len < 4) {
        printf("memdump: not enough data for '%c'\n", *fmt);
        return;
      }
      printf("%d\n", *(int *)data);
      data += 4;
      len -= 4;
    } else if (*fmt == 'p') {
      if (len < 8) {
        printf("memdump: not enough data for '%c'\n", *fmt);
        return;
      }
      printf("%lx\n", *(uint64 *)data);
      data += 8;
      len -= 8;
    } else if (*fmt == 'h') {
      if (len < 2) {
        printf("memdump: not enough data for '%c'\n", *fmt);
        return;
      }
      printf("%d\n", *(short *)data);
      data += 2;
      len -= 2;
    } else if (*fmt == 'c') {
      if (len < 1) {
        printf("memdump: not enough data for '%c'\n", *fmt);
        return;
      }
      printf("%c\n", *data);
      data++;
      len--;
    } else if (*fmt == 's') {
      if (len < 8) {
        printf("memdump: not enough data for '%c'\n", *fmt);
        return;
      }
      printf("%s\n", *(char **)data);
      data += 8;
      len -= 8;
    } else if (*fmt == 'S') {
      for (int i = 0; i < len && data[i] != '\0'; i++)
        printf("%c", data[i]);
      printf("\n");
      return;
    }
  }
}
