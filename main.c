#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "defs.h"
#include "inode.h"
#include "filesystem.h"

const char *getMessage(vfs_ret_t error) {
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

int handleReturn(vfs_ret_t ret) {
    if (ret != VFS_SUCCESS) {
        printf("Error occurred: %s\n", getMessage(ret));
    }
    return ret;
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