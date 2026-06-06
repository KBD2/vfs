#pragma once

#include <stdint.h>

#include "defs.h"

// 8 byte overhead for the other inode stuff (type, size, next inode index)
#define INODE_DATA_BYTES 4088
#define INODE_SUPER_IDX 0
#define INODE_END_IDX 0

enum vfs_inode_type {
    INODE_EMPTY,
    INODE_BRANCH,
    INODE_FILE
};

struct vfs_branch_descriptor {
    enum vfs_inode_type type;
    char name[MAX_NAME_LENGTH];
    uint16_t idx;
};

#define BRANCH_NUM_DESCRIPTORS (INODE_DATA_BYTES / sizeof(struct vfs_branch_descriptor))

union vfs_branch_data {
    struct vfs_branch_descriptor descData[BRANCH_NUM_DESCRIPTORS];
    char byteData[INODE_DATA_BYTES];
};

struct vfs_inode {
    enum vfs_inode_type type;
    uint16_t next;
    uint16_t size;
    union vfs_branch_data data;
};