#include <string.h>

#include "filesystem.h"
#include "defs.h"
#include "inode.h"

struct vfs_inode *filesystem = NULL;

uint16_t findFreeInode() {
    for (int i = 0; i < NUM_INODES; i++) {
        if (filesystem[i].type == INODE_EMPTY) return i;
    }
    return 0;
}

vfs_ret_t initialiseBranch(uint16_t idx) {
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

vfs_ret_t initialiseSuper() {
    return initialiseBranch(0);
}

vfs_ret_t createFolder(int parentIdx, const char *name, uint16_t *idx) {
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