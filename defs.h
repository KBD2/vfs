#pragma once

#define NUM_INODES 64
#define MAX_NAME_LENGTH 32

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

typedef enum {
    VFS_SUCCESS,
    VFS_ERROR
} vfs_ret_t;

vfs_ret_t error(const char *message);

const char *getLastError();