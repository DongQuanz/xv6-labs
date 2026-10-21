#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  int size = 32 * PGSIZE;
  char *data = sbrk(size);
  char marker[] = "Here it is: ";

  if (data == SBRK_ERROR)
    exit(1);

  for (int page = 0; page < size; page += PGSIZE) {
    for (int offset = 32 + '0'; offset <= 32 + 'z'; offset++) {
      char *p = data + page + offset;
      if (memcmp(p, marker, sizeof(marker) - 1) != 0)
        continue;

      char *secret = p + sizeof(marker) - 1;
      if (offset != 32 + (unsigned char)secret[0])
        continue;

      int len = 0;
      while (offset + sizeof(marker) - 1 + len < PGSIZE) {
        char c = secret[len];
        if (c == 0)
          break;
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z')))
          break;
        len++;
      }
      if (len > 0 && offset + sizeof(marker) - 1 + len < PGSIZE &&
          secret[len] == 0) {
        printf("%s\n", secret);
        exit(0);
      }
    }
  }
  exit(1);
}
