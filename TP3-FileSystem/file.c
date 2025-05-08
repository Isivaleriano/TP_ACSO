#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

#define SECTOR_SIZE 512

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    if (inumber < 1 || blockNum < 0 || buf == NULL) {
        return -1;
    }

    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }
    int block_num = inode_indexlookup(fs, &in, blockNum);
    if (block_num <= 0) {
        return -1;
    }
    int bytes_read = diskimg_readsector(fs->dfd, block_num, buf);
    if (bytes_read == -1) {
        return -1;
    }

    int filesize = inode_getsize(&in);
    int start_byte = blockNum * SECTOR_SIZE;

    if (filesize <= start_byte) {
        return 0;  
    }

    int remaining = filesize - start_byte;
    return (remaining < SECTOR_SIZE) ? remaining : SECTOR_SIZE;
}
