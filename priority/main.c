#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
    int ret = getpriority(PRIO_PGRP, 0);
    ret = setpriority(PRIO_PGRP, 0, -20);
    ret = getpriority(PRIO_PGRP, 0);
    printf("ret : %d\n", ret);
    usleep(10000000);
    return 0;
}
