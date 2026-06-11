#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "gui.h"
#include "defs.h"
#include "filesystem.h"
#include "inode.h"

SDL_Window *window;
SDL_Renderer *ctx;

SDL_Surface *textAtlas;

int initGui() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (!IMG_Init(IMG_INIT_PNG)) {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
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
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    ctx = SDL_CreateSoftwareRenderer(surface);
    if (ctx == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    textAtlas = IMG_Load("resource/cp437.png");

    if (textAtlas == NULL) {
        printf("Error loading text atlas: %s\n", IMG_GetError());
        return 1;
    }

    return 0;
}

void destroyGui() {
    SDL_FreeSurface(textAtlas);
    SDL_DestroyRenderer(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#define drawText(surface, x, y, text) drawTextWrap(surface, x, y, -1, text);

void drawTextWrap(SDL_Surface *surface, uint16_t x, uint16_t y, int wrapChars, const char *text) {
    SDL_Rect srcRect = {
        .x = 0,
        .y = 0,
        .w = 10,
        .h = 10
    };
    SDL_Rect destRect = {
        .x = x,
        .y = y,
        .w = 10,
        .h = 10
    };
    for (int i = 0; i < strlen(text); i++) {
        char c = text[i];
        srcRect.x = 10 * (c % 16);
        srcRect.y = 10 * (c / 16);
        SDL_BlitSurface(textAtlas, &srcRect, surface, &destRect);
        destRect.x += 7; // Only works for letters and numbers, others have different width
        if (wrapChars > -1 && (i + 1) % wrapChars == 0) {
            destRect.x = x;
            destRect.y += 11;
        }
    }
}

void render() {
    static char buf[32];

    SDL_SetRenderDrawColor(ctx, 255, 255, 255, 255);
    SDL_RenderClear(ctx);

    SDL_SetRenderDrawColor(ctx, 0, 0, 0, 255);
    for (int i = 0; i < 8; i++) {
        SDL_RenderDrawLine(ctx, 0, 60 * i, 640, 60 * i);
        SDL_RenderDrawLine(ctx, 80 * i, 0, 80 * i, 480);
    }

    SDL_Surface *descriptorText = SDL_CreateRGBSurface(0, 640, 480, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

    for (int idx = 0; idx < NUM_INODES; idx++) {
        sprintf(buf, "%u", idx);
        drawText(descriptorText, 80 * (idx % 8) + 1, 60 * (idx / 8) + 1, buf);
        struct vfs_inode *inode;
        getInode(idx, &inode);
        SDL_Rect rect = {
            .x = 80 * (idx % 8) + 1,
            .y = 60 * (idx / 8) + 1,
            .w = 79,
            .h = 59
        };
        if  (inode->type == INODE_BRANCH) {
            sprintf(buf, "Parent: %d", idx);
            SDL_SetRenderDrawColor(ctx, 0xe1, 0xb8, 0x42, 0xff);
            SDL_RenderFillRect(ctx, &rect);
            for (int i = 0; i < inode->size; i++) {
                struct vfs_branch_descriptor *desc = &inode->data.descData[i];
                drawTextWrap(descriptorText, 80 * (desc->idx % 8) + 1, 60 * (desc->idx / 8) + 11, 11, desc->name);
                drawText(descriptorText, 80 * (desc->idx % 8) + 1, 60 * (desc->idx / 8) + 31, buf);
            }
        } else if (inode->type == INODE_FILE) {
            SDL_SetRenderDrawColor(ctx, 0xae, 0xd5, 0xf5, 0xff);
            SDL_RenderFillRect(ctx, &rect);
        }
    }

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    SDL_BlitSurface(descriptorText, NULL, surface, NULL);

    SDL_FreeSurface(descriptorText);

    SDL_RenderPresent(ctx);
    SDL_UpdateWindowSurface(window);
}

vfs_gui_ret_t updateGui() {
    render();
    SDL_Event e;
    if (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) return GUI_QUIT;
    }
    return GUI_CONTINUE;
}