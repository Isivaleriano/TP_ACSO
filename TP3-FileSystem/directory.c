#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>

#define DIR_NAME_SIZE 14
#define SECTOR_SIZE 512

int directory_findname(struct unixfilesystem *fs, const char *name, int inumber, struct direntv6 *dirent) {
    if (!fs || !name || !dirent || inumber < 1) {
        return -1;
    }

    struct inode dirin;
    if (inode_iget(fs, inumber, &dirin) < 0) {
        return -1;
    }

    if ((dirin.i_mode & IFMT) != IFDIR) {
        return -1;
    }

    int filesize = inode_getsize(&dirin);
    int num_blocks = (filesize + SECTOR_SIZE - 1) / SECTOR_SIZE;

    char block[SECTOR_SIZE];

    for (int block_num = 0; block_num < num_blocks; block_num++) {
        int bytes = file_getblock(fs, inumber, block_num, block);
        if (bytes < 0) {
            return -1;
        }

        int num_entries = bytes / sizeof(struct direntv6);
        struct direntv6 *entry = (struct direntv6 *)block;

        for (int i = 0; i < num_entries; i++) {
            if (strncmp(entry[i].d_name, name, DIR_NAME_SIZE) == 0) {
                *dirent = entry[i];
                return 0;
            }
        }
    }

    return -1;
}
