#include "defs.h"

const char *lastError;

vfs_ret_t error(vfs_error_t error) {
    switch (error) {
        case VFS_ERR_INODE_EXISTS:
            lastError = "inode is not empty";
            break;
        case VFS_ERR_INODE_DOESNT_EXIST:
            lastError = "inode couldn't be found";
            break;
        case VFS_ERR_INODE_INVALID:
            lastError = "Invalid inode index";
            break;
        case VFS_ERR_PARENT_INVALID:
            lastError = "Invalid parent inode";
            break;
        case VFS_ERR_NO_FREE_INODES:
            lastError = "No free inodes";
            break;
        case VFS_ERR_CANT_ALLOCATE:
            lastError = "Can't allocate memory";
            break;
        case VFS_ERR_INVALID_PATH_CHAR:
            lastError = "Invalid character in path";
            break;
        case VFS_ERR_INVALID_NAME:
            lastError = "Invalid name in path";
            break;
        case VFS_ERR_NOT_FOUND:
            lastError = "Path not found";
            break;
        case VFS_ERR_NAME_ALREADY_EXISTS:
            lastError = "Path already exists";
            break;
        default:
            lastError = "Unknown error";
    }
    return VFS_ERROR;
}

const char *getLastError() {
    return lastError;
}