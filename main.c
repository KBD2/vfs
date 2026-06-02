#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NUM_INODES 256
// 8 byte overhead for the other inode stuff (type, size, next inode index)
#define INODE_DATA_BYTES 56
#define INODE_SUPER_IDX 0
#define INODE_END_IDX 0

typedef enum {
    VFS_SUCCESS,
    VFS_ERR_INODE_EXISTS,
    VFS_ERR_INODE_DOESNT_EXIST,
    VFS_ERR_INODE_INVALID,
    VFS_ERR_PARENT_INVALID,
    VFS_ERR_NO_FREE_INODES
} vfs_return;

const char *getMessage(int error) {
    switch (error) {
        case VFS_SUCCESS:
            return "Success";
        case VFS_ERR_INODE_EXISTS:
            return "inode is not empty";
        case VFS_ERR_INODE_DOESNT_EXIST:
            return "inode couldn't be found";
        case VFS_ERR_INODE_INVALID:
            return "Invalid inode index";
        case VFS_ERR_PARENT_INVALID:
            return "Invalid parent inode";
        case VFS_ERR_NO_FREE_INODES:
            return "No free inodes";
        default:
            return "Unknown error";
    }
};

int handleReturn(vfs_return ret) {
    if (ret != VFS_SUCCESS) {
        printf("Error occurred: %s\n", getMessage(ret));
    }
    return ret;
}

struct vfs_inode *filesystem;

enum vfs_inode_type {
    INODE_EMPTY,
    INODE_BRANCH,
    INODE_FILE
};

struct vfs_branch_descriptor {
    enum vfs_inode_type type;
    char name[32];
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

uint16_t findFreeInode() {
    for (int i = 0; i < NUM_INODES; i++) {
        if (filesystem[i].type == INODE_EMPTY) return i;
    }
    return 0;
}

vfs_return initialiseBranch(uint16_t idx) {
    if (idx >= NUM_INODES) return VFS_ERR_INODE_INVALID;
    struct vfs_inode *branch = &filesystem[idx];
    if (branch->type != INODE_EMPTY) {
        return VFS_ERR_INODE_EXISTS;
    }
    branch->type = INODE_BRANCH;
    branch->next = INODE_END_IDX;
    branch->size = 0;
    memset(branch->data.descData, 0, INODE_DATA_BYTES);
    return VFS_SUCCESS;
}

vfs_return initialiseSuper() {
    return initialiseBranch(0);
}

vfs_return createFolder(int parentIdx, char *name, uint16_t *idx) {
    if (parentIdx >= NUM_INODES) return VFS_ERR_INODE_INVALID;
    struct vfs_inode *parent = &filesystem[parentIdx];

    if (parent->type != INODE_BRANCH) return VFS_ERR_PARENT_INVALID;

    uint16_t freeInode = findFreeInode();

    if (freeInode == 0) return VFS_ERR_NO_FREE_INODES;

    // TODO: handle expanding branch
    int assignedDescriptor = parent->size++;

    struct vfs_branch_descriptor *descriptor = &parent->data.descData[assignedDescriptor];
    descriptor->idx = findFreeInode(),
    descriptor->type = INODE_BRANCH;
    strncpy(descriptor->name, name, 32);
    descriptor->name[31] = '\0';

    initialiseBranch(freeInode);

    memset(filesystem[freeInode].data.descData, 0, INODE_DATA_BYTES);

    *idx = freeInode;

    return VFS_SUCCESS;
}

void writeToFile() {
    FILE *file = fopen("./filesystem.bin", "wb");
    fwrite(filesystem, sizeof(struct vfs_inode), NUM_INODES, file);
    fclose(file);
}

int main() {
    printf("%lu bytes per inode\n", sizeof(struct vfs_inode));
    printf("%ld branch descriptors per branch inode\n", BRANCH_NUM_DESCRIPTORS);
    filesystem = calloc(NUM_INODES, sizeof(struct vfs_inode));
    if (filesystem == NULL) {
        printf("Failed to allocate filesystem\n");
        return 0;
    }

    if (handleReturn(initialiseSuper())) {
        return 0;
    }

    uint16_t idx;
    if (handleReturn(createFolder(0, "Chicken Jockey", &idx))) {
        return 0;
    }

    if (handleReturn(createFolder(idx, "Northernlion", &idx))) {
        return 0;
    }

    printf("%u\n", idx);

    writeToFile();
    free(filesystem);
    return 1;
}