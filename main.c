#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "defs.h"
#include "inode.h"
#include "filesystem.h"
#include "gui.h"

int handleReturn(vfs_ret_t ret) {
    if (ret != VFS_SUCCESS) {
        printf("Error occurred: %s\n", getLastError());
    }
    return ret;
}

int main() {

    if (handleReturn(initFilesystem())) return 0;

    if (handleReturn(createFolder("One"))) return 0;

    if (handleReturn(createFolder("Two")))  return 0;
    if (handleReturn(createFolder("Two/Three")))  return 0;

    if (handleReturn(createFile("Two/Three/test.txt"))) return 0;

    if (handleReturn(createFolder("One/Four")))  return 0;

    if (initGui()) return 1;

    while (updateGui() == GUI_CONTINUE) {}
    
    destroyGui();
    destroyFilesystem();

    return 1;
}