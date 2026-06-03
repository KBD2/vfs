#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "defs.h"
#include "inode.h"
#include "filesystem.h"

const char *getMessage(vfs_ret_t error) {
    switch (error) {
        case VFS_SUCCESS:
            return "Success";
        case VFS_ERR_INODE_EXISTS:
            return "inode is not empty";
        case VFS_ERR_INODE_DOESNT_EXIST:
            return "inode couldn't be found";
        case VFS_ERR_INODE_INVALID:
            return "Invalid inode index";
        case VFS_ERR_PARENT_INVALID:
            return "Invalid parent inode";
        case VFS_ERR_NO_FREE_INODES:
            return "No free inodes";
        default:
            return "Unknown error";
    }
};

int handleReturn(vfs_ret_t ret) {
    if (ret != VFS_SUCCESS) {
        printf("Error occurred: %s\n", getMessage(ret));
    }
    return ret;
}

void writeToFile() {
    FILE *file = fopen("./filesystem.bin", "wb");
    fwrite(filesystem, sizeof(struct vfs_inode), NUM_INODES, file);
    fclose(file);
}

void render(SDL_Renderer* ctx) {
    SDL_SetRenderDrawColor(ctx, 255, 255, 255, 255);
    SDL_RenderClear(ctx);

    SDL_SetRenderDrawColor(ctx, 0, 0, 0, 255);
    for (int i = 0; i < 8; i++) {
        SDL_RenderDrawLine(ctx, 0, 60 * i, 640, 60 * i);
        SDL_RenderDrawLine(ctx, 80 * i, 0, 80 * i, 480);
    }

    for (int idx = 0; idx < NUM_INODES; idx++) {
        struct vfs_inode *inode = &filesystem[idx];
        SDL_Rect rect = {
            .x = 80 * (idx % 8) + 1,
            .y = 60 * (idx / 8) + 1,
            .w = 79,
            .h = 59
        };
        if  (inode->type == INODE_BRANCH) {
            SDL_SetRenderDrawColor(ctx, 0xe1, 0xb8, 0x42, 255);
            SDL_RenderFillRect(ctx, &rect);
        }
    }

    SDL_RenderPresent(ctx);
}

int main() {
    printf("%lu bytes per inode\n", sizeof(struct vfs_inode));
    printf("%ld branch descriptors per branch inode\n", BRANCH_NUM_DESCRIPTORS);
    filesystem = calloc(NUM_INODES, sizeof(struct vfs_inode));
    if (filesystem == NULL) {
        printf("Failed to allocate filesystem\n");
        return 0;
    }

    if (handleReturn(initialiseSuper())) {
        return 0;
    }

    uint16_t idx;
    if (handleReturn(createFolder(0, "Chicken Jockey", &idx))) {
        return 0;
    }

    if (handleReturn(createFolder(idx, "Northernlion", &idx))) {
        return 0;
    }

    printf("%u\n", idx);

    writeToFile();

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Virtual Filesystem", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        0
    );
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    // We need a software renderer because hardware acceleration segfaults on any draw
    // calls in WSL (that was fun to debug).
    SDL_Renderer* ctx = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (ctx == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    while (true) {
        render(ctx);

        SDL_Event e;
        if (SDL_WaitEvent(&e)) {
            if (e.type == SDL_QUIT) break;
        }
    }

    SDL_DestroyRenderer(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    free(filesystem);

    return 1;
}