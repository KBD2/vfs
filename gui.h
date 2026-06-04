#pragma once

typedef enum {
    GUI_CONTINUE,
    GUI_QUIT
} vfs_gui_ret_t;

int initGui();
void destroyGui();

vfs_gui_ret_t updateGui();