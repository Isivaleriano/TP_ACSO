#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "inode.h"
#include "diskimg.h"

#define INODE_SIZE 32 
#define SECTOR_SIZE 512 
#define INODES_PER_BLOCK (SECTOR_SIZE / INODE_SIZE) 
#define INODE_START_SECTOR 2
#define ADDRS_PER_BLOCK (SECTOR_SIZE / sizeof(uint16_t)) 

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) return -1;

    int block_num = INODE_START_SECTOR + (inumber - 1) / INODES_PER_BLOCK;
    int index = (inumber - 1) % INODES_PER_BLOCK;

    if (index >= INODES_PER_BLOCK) return -1;

    char buffer[SECTOR_SIZE];
    int bytes = diskimg_readsector(fs->dfd, block_num, buffer);
    if (bytes == -1) return -1;

    struct inode *inodes = (struct inode *)buffer;
    *inp = inodes[index];

    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int fileBlockNum) {
    if (fileBlockNum < 0) return -1;

    if (!(inp->i_mode & ILARG)) {
        if (fileBlockNum >= 8) return -1;
        return inp->i_addr[fileBlockNum];
    }

    if (fileBlockNum < 7 * (int)ADDRS_PER_BLOCK) {
        int indirect_index = fileBlockNum / ADDRS_PER_BLOCK;
        int offset = fileBlockNum % ADDRS_PER_BLOCK;

        uint16_t indirect_block[ADDRS_PER_BLOCK];
        int block_num = inp->i_addr[indirect_index];
        if (block_num == 0) return -1;

        if (diskimg_readsector(fs->dfd, block_num, &indirect_block) == -1) {
            return -1;
        }

        return indirect_block[offset];
    }

    int relativeBlock = fileBlockNum - 7 * ADDRS_PER_BLOCK;
    int outer_index = relativeBlock / ADDRS_PER_BLOCK;
    int inner_index = relativeBlock % ADDRS_PER_BLOCK;

    if (outer_index >= (int)ADDRS_PER_BLOCK) return -1;

    uint16_t doubly_indirect_block[ADDRS_PER_BLOCK];
    int doubly_block_num = inp->i_addr[7];
    if (doubly_block_num == 0) return -1;

    if (diskimg_readsector(fs->dfd, doubly_block_num, &doubly_indirect_block) == -1) {
        return -1;
    }

    int indirect_block_num = doubly_indirect_block[outer_index];
    if (indirect_block_num == 0) return -1;

    uint16_t indirect_block[ADDRS_PER_BLOCK];
    if (diskimg_readsector(fs->dfd, indirect_block_num, &indirect_block) == -1) {
        return -1;
    }

    return indirect_block[inner_index];
}

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1);
}
