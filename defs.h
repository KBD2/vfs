#pragma once

#define NUM_INODES 64
#define MAX_NAME_LENGTH 31
#define PATH_NODE_BUF_SIZE 100

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

typedef enum {
    VFS_SUCCESS,
    VFS_ERROR
} vfs_ret_t;

typedef enum {
    VFS_ERR_INODE_EXISTS,
    VFS_ERR_INODE_DOESNT_EXIST,
    VFS_ERR_INODE_INVALID,
    VFS_ERR_PARENT_INVALID,
    VFS_ERR_NO_FREE_INODES,
    VFS_ERR_CANT_ALLOCATE,
    VFS_ERR_INVALID_PATH_CHAR,
    VFS_ERR_INVALID_NAME,
    VFS_ERR_NOT_FOUND,
    VFS_ERR_NAME_ALREADY_EXISTS,
    VFS_ERR_INODE_TYPE_INVALID
} vfs_error_t;

vfs_ret_t error(vfs_error_t error);

const char *getLastError();