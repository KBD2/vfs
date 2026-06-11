#pragma once

#include <stdint.h>

#include "defs.h"
#include "inode.h"

vfs_ret_t initFilesystem();
void destroyFilesystem();

vfs_ret_t getInode(uint16_t idx, struct vfs_inode **inode);

vfs_ret_t createFolder(const char *path);
vfs_ret_t createFile(const char *path);