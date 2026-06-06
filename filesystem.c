#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "filesystem.h"
#include "defs.h"
#include "errors.h"
#include "inode.h"

const char *lastError = "";

vfs_ret_t error(const char *message) {
    lastError = message;
    return VFS_ERROR;
}

const char *getLastError() {
    return lastError;
}

struct vfs_inode *filesystem = NULL;
char *nameBuf = NULL;

vfs_ret_t initFilesystem() {
    filesystem = calloc(NUM_INODES, sizeof(struct vfs_inode));
    if (filesystem == NULL) return error(VFS_ERR_CANT_ALLOCATE);
    nameBuf = malloc(32 * sizeof(char));
    if (nameBuf == NULL) return error(VFS_ERR_CANT_ALLOCATE);
    return VFS_SUCCESS;
}
    
void writeToFile() {
    FILE *file = fopen("./filesystem.bin", "wb");
    fwrite(filesystem, sizeof(struct vfs_inode), NUM_INODES, file);
    fclose(file);
}

void destroyFilesystem() {
    writeToFile();
    free(filesystem);
    free(nameBuf);
}

vfs_ret_t getInode(uint16_t idx, struct vfs_inode **inode) {
    if (idx >= NUM_INODES) return error(VFS_ERR_INODE_INVALID);
    *inode = &filesystem[idx];
    return VFS_SUCCESS;
}

uint16_t findFreeInode() {
    for (int i = 0; i < NUM_INODES; i++) {
        if (filesystem[i].type == INODE_EMPTY) return i;
    }
    return 0;
}

vfs_ret_t initialiseBranch(uint16_t idx) {
    if (idx >= NUM_INODES) return error(VFS_ERR_INODE_INVALID);
    struct vfs_inode *branch = &filesystem[idx];
    if (branch->type != INODE_EMPTY) {
        return error(VFS_ERR_INODE_EXISTS);
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

bool validatePath(const char *path) {
    for (int i = 0; i < strlen(path); i++) {
        char c = path[i];
        if (!(
            c >= 'a' && c <= 'z' 
            || c >= 'A' && c <= 'Z' 
            || c >= '1' && c <= '0' 
            || c == '.' || c == '(' || c == ')' || c == '/' || c == ' '
        )) {
            return false;
        }
    }
    return true;
}

// TODO: Handle multi-part branch
struct vfs_branch_descriptor *findDescriptor(char *name, struct vfs_inode *inode) {
    if (inode->type != INODE_BRANCH) return NULL;
    for (int i = 0; i < inode->size; i++) {
        struct vfs_branch_descriptor *test = &inode->data.descData[i];
        if (!strcmp(name, test->name)) return test;
    }
    return NULL;
}

vfs_ret_t createBranch(struct vfs_inode *parent, const char *name, uint16_t *idx) {
    if (parent->type != INODE_BRANCH) return error(VFS_ERR_PARENT_INVALID);

    uint16_t freeInode = findFreeInode();

    if (freeInode == 0) return error(VFS_ERR_NO_FREE_INODES);

    // TODO: handle expanding branch
    int assignedDescriptor = parent->size++;

    struct vfs_branch_descriptor *descriptor = &parent->data.descData[assignedDescriptor];
    descriptor->idx = findFreeInode(),
    descriptor->type = INODE_BRANCH;
    strncpy(descriptor->name, name, 32);
    descriptor->name[31] = '\0';

    initialiseBranch(freeInode);

    memset(filesystem[freeInode].data.descData, 0, INODE_DATA_BYTES);

    if (idx != NULL) *idx = freeInode;

    return VFS_SUCCESS;
}

vfs_ret_t createFolder(const char *path) {
    if (!validatePath(path)) return error(VFS_ERR_INVALID_PATH_CHAR);
    struct vfs_inode *currentBranch = &filesystem[INODE_SUPER_IDX];
    int nameBufIdx = 0;
    for (int i = 0; i < strlen(path); i++) {
        char c = path[i];
        if (c == '/') {
            nameBuf[nameBufIdx] = '\0';
            struct vfs_branch_descriptor *result = findDescriptor(nameBuf, currentBranch);
            if (result == NULL || result->type != INODE_BRANCH) {
                return error(VFS_ERR_NOT_FOUND);
            }
            currentBranch = &filesystem[result->idx];
            nameBufIdx = 0;
        } else {
            if (nameBufIdx == (MAX_NAME_LENGTH - 2)) return error(VFS_ERR_INVALID_NAME);
            nameBuf[nameBufIdx++] = c;
        }
    }
    if (nameBufIdx == 0) return error(VFS_ERR_NOT_FOUND);
    nameBuf[nameBufIdx] = '\0';
    if (findDescriptor(nameBuf, currentBranch) != NULL) return error(VFS_ERR_NAME_ALREADY_EXISTS);

    return createBranch(currentBranch, nameBuf, NULL);
}