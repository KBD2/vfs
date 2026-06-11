#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "filesystem.h"
#include "defs.h"
#include "inode.h"

struct vfs_inode *filesystem = NULL;

struct path_part {
    char name[MAX_NAME_LENGTH + 1];
    struct path_part *next;
};

struct path_part *pathParts = NULL;

void freePathParts() {
    struct path_part *part = pathParts;
    while (part != NULL) {
        struct path_part *next = part->next;
        free(part);
        part = next;
    }
    pathParts = NULL;
}

vfs_ret_t addPathPart(const char *name) {
    struct path_part *part = malloc(sizeof(struct path_part));
    if (part == NULL) return error(VFS_ERR_CANT_ALLOCATE);
    strncpy(part->name, name, MAX_NAME_LENGTH);
    part->name[MAX_NAME_LENGTH] = '\0';
    part->next = NULL;
    if (pathParts == NULL) {
        pathParts = part;
    } else {
        struct path_part *end = pathParts;
        while (end->next != NULL) end = end->next;
        end->next = part;
    }
    return VFS_SUCCESS;
}

void debugPathParts() {
    printf("=====\nPATH PART DEBUG\n");
    struct path_part *part = pathParts;
    while (part != NULL) {
        printf("%s\n", part->name);
        part = part->next;
    }
    printf("=====\n");
}
    
void writeToFile() {
    FILE *file = fopen("./filesystem.bin", "wb");
    fwrite(filesystem, sizeof(struct vfs_inode), NUM_INODES, file);
    fclose(file);
}

void destroyFilesystem() {
    writeToFile();
    free(filesystem);
    freePathParts();
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

vfs_ret_t initialiseSuper() {
    struct vfs_inode *super = filesystem; // First inode in the filesystem, variable for clarity
    super->type = INODE_BRANCH;
    super->next = INODE_END_IDX;
    super->size = 0;
    memset(super->data.byteData, 0, INODE_DATA_BYTES);
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

vfs_ret_t splitPath(const char *path, int *length) {
    static char nameBuf[MAX_NAME_LENGTH + 1];
    freePathParts();
    int numParts = 0;
    int nameBufIdx = 0;
    for (int i = 0; i < strlen(path); i++) {
        char c = path[i];
        if (c == '/') {
            nameBuf[nameBufIdx] = '\0';
            if (addPathPart(nameBuf) == VFS_ERROR) return VFS_ERROR;
            numParts++;
            nameBufIdx = 0;
        } else {
            if (nameBufIdx == (MAX_NAME_LENGTH - 1)) return error(VFS_ERR_INVALID_NAME);
            nameBuf[nameBufIdx++] = c;
        }
    }
    if (nameBufIdx != 0) {
        nameBuf[nameBufIdx] = '\0';
        if (addPathPart(nameBuf) == VFS_ERROR) return VFS_ERROR;
        numParts++;
    }
    if (length != NULL) *length = numParts;
    return VFS_SUCCESS;
}

vfs_ret_t allocateNode(struct vfs_inode *parent, enum vfs_inode_type type, const char *name, uint16_t *idx) {
    if (parent->type != INODE_BRANCH) return error(VFS_ERR_PARENT_INVALID);

    uint16_t freeInode = findFreeInode();

    if (freeInode == 0) return error(VFS_ERR_NO_FREE_INODES);

    // TODO: handle expanding branch
    int assignedDescriptor = parent->size++;

    struct vfs_branch_descriptor *descriptor = &parent->data.descData[assignedDescriptor];
    descriptor->idx = freeInode,
    descriptor->type = type;
    strncpy(descriptor->name, name, 32);
    descriptor->name[31] = '\0';

    struct vfs_inode *inode = &filesystem[freeInode];
    inode->type = type;
    inode->next = INODE_END_IDX;
    inode->size = 0;
    memset(inode->data.byteData, 0, INODE_DATA_BYTES);

    if (idx != NULL) *idx = freeInode;

    return VFS_SUCCESS;
}

vfs_ret_t createNodeFromPath(const char *path, enum vfs_inode_type type) {
    if (!validatePath(path)) return error(VFS_ERR_INVALID_PATH_CHAR);

    if (type != INODE_BRANCH && type != INODE_FILE) return error(VFS_ERR_INODE_TYPE_INVALID);

    int pathLength;
    if (splitPath(path, &pathLength) == VFS_ERROR) return VFS_ERROR;

    struct vfs_inode *currentBranch = &filesystem[INODE_SUPER_IDX];

    struct path_part *part = pathParts;
    while (part->next != NULL) {
        struct vfs_branch_descriptor *result = findDescriptor(part->name, currentBranch);
        if (result == NULL || result->type != INODE_BRANCH) {
            return error(VFS_ERR_NOT_FOUND);
        }
        currentBranch = &filesystem[result->idx];
        part = part->next;
    }

    if (findDescriptor(part->name, currentBranch) != NULL) return error(VFS_ERR_NAME_ALREADY_EXISTS);

    return allocateNode(currentBranch, type, part->name, NULL);
}

inline vfs_ret_t createFolder(const char *path) {
    return createNodeFromPath(path, INODE_BRANCH);
}

inline vfs_ret_t createFile(const char *path) {
    return createNodeFromPath(path, INODE_FILE);
}

vfs_ret_t initFilesystem() {
    filesystem = calloc(NUM_INODES, sizeof(struct vfs_inode));
    if (filesystem == NULL) return error(VFS_ERR_CANT_ALLOCATE);
    initialiseSuper();
    return VFS_SUCCESS;
}