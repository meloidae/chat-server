#ifndef CHATUI_H
#define CHATUI_H
#include <ncurses.h>
#include <stddef.h>

#define NORMAL_STR "NORMAL"
#define COMMAND_STR "CHAT"
#define PAD_ROWS 2048

enum ui_mode { MODE_NORMAL, MODE_CHAT };

enum handle_flag { FLAG_PASS, FLAG_QUIT, FLAG_SEND };

typedef struct chat_ui {
    int mode;
    WINDOW *main;
    WINDOW *status;
    WINDOW *command;
    char *buffer;
    size_t buf_size;
    char *buf_pos;
    int main_top;
} chat_ui;

void init_ncurses();
void end_ncurses();
WINDOW *create_newwin(int height, int width, int starty, int startx);
WINDOW *create_newpad(int height, int width, int visible_height, int visible_width, int starty, int startx);
void destroy_win(WINDOW *win);
chat_ui *init_ui();
void destroy_ui(chat_ui *ui);
void resize_ui(chat_ui *ui);
int handle_key(chat_ui *ui, int key);
void print_ui_status(chat_ui *ui);
void print_ui_main(chat_ui *ui, const char *time_str, const char *message);
void reset_ui_buffer(chat_ui *ui);
#endif
