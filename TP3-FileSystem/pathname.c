#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (!fs || !pathname || pathname[0] != '/') {
        return -1;
    }

    char path_copy[1024];
    strncpy(path_copy, pathname, sizeof(path_copy));
    path_copy[sizeof(path_copy) - 1] = '\0';

    int inumber = 1; 
    struct direntv6 result;

    char *token = strtok(path_copy, "/");
    while (token != NULL) {
        if (directory_findname(fs, token, inumber, &result) < 0) {
            return -1;
        }
        inumber = result.d_inumber;
        token = strtok(NULL, "/");
    }

    return inumber;
}
