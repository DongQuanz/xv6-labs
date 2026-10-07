#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(2, "sleep: too few arguments\n");
        return 1;
    } else if (argc > 2) {
        fprintf(2, "sleep: too many arguments\n");
        return 1;
    }

    int time = atoi(argv[1]);
    if (time <= 0) {
        fprintf(2, "sleep: invalid time value\n");
        return 1;
    }
    
    printf("sleep: pausing for %d ticks...\n", time);

    pause(time);

    return 0;
}