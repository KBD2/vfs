#pragma once

#include <stdint.h>

#include "defs.h"
#include "inode.h"

vfs_ret_t initFilesystem();
void destroyFilesystem();

vfs_ret_t createFolder(const char *path);