#pragma once

#include <stdint.h>

#include "defs.h"

extern struct vfs_inode *filesystem;

uint16_t findFreeInode();

vfs_ret_t initialiseBranch(uint16_t idx);

vfs_ret_t createFolder(int parentIdx, const char *name, uint16_t *idx);

vfs_ret_t initialiseSuper();