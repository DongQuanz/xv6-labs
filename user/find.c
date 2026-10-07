#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void lookInside(char* path, char* target) {

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

                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;

                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;

                if (strcmp(de.name, target) == 0)
                    printf("%s\n", buf);

                if (stat(buf, &st) < 0) {
                    fprintf(2, "find: cannot stat %s\n", buf);
                    continue;
                }
                
                if (st.type == T_DIR)
                    lookInside(buf, target);
            }
            break;
    }
    close(fd);
    return;
}

int
main (int argc, char* argv[]) {

    if (argc > 3) {
        fprintf(2, "find: too many arguments\n");
        return 1;
    }
    else if (argc < 3) {
        fprintf(2, "find: too few arguments\n");
        return 1;
    }

    lookInside(argv[1], argv[2]);
    return 0;
}