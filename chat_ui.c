#include <stdlib.h>
#include <string.h>
#include "chat_ui.h"


void init_ncurses() {
    initscr();
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_WHITE);
    } // if
    //raw();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    noecho();
} // init_ncurses


void end_ncurses() {
    endwin();
} // terminate_ncurses

WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    refresh();
    wrefresh(local_win);

    return local_win;
} // create_newwin

WINDOW *create_newpad(int height, int width, int visible_height, int visible_width, int starty, int startx) {
    WINDOW *local_pad;
    local_pad = newpad(height, width);
    refresh();
    prefresh(local_pad, 0, 0, starty, startx, visible_height, visible_width);
    return local_pad;
} // create_newpad

void destroy_win(WINDOW *local_win) {
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
} // destroy_win

chat_ui *init_ui(size_t buffer_size) {
    chat_ui *ui;
    int max_rows, max_cols;
    
    ui = (chat_ui *)malloc(sizeof(chat_ui));
    ui->buffer = (char *)malloc(buffer_size);
    ui->buf_pos = ui->buffer;
    ui->buf_size = buffer_size;
    ui->main_top = 0;

    init_ncurses();
    getmaxyx(stdscr, max_rows, max_cols);
    
    ui->mode = MODE_NORMAL;
    ui->main = create_newpad(100, max_cols, max_rows - 2, max_cols, 0, 0);
    ui->status = create_newwin(1, max_cols, max_rows - 2, 0);
    ui->command = create_newwin(1, max_cols, max_rows - 1, 0);

    scrollok(ui->main, TRUE);
    prefresh(ui->main, 0, 0, 0, 0, max_rows -3, max_cols - 1);
    if (has_colors()) {
        wbkgd(ui->status, COLOR_PAIR(1));
    } // if

    return ui;
} // init_ui

void destroy_ui(chat_ui *ui) {
    destroy_win(ui->main);
    destroy_win(ui->status);
    destroy_win(ui->command);
    end_ncurses();
    free(ui);
} // destroy_ui

void resize_ui(chat_ui *ui) {
    clear();
    mvwprintw(ui->main, 0, 0, "COLS = %d, LINES = %d", COLS, LINES);
    wrefresh(ui->main);
} // resize_ui 

int handle_key(chat_ui *ui, int key) {
    int flag = FLAG_PASS;
    int x, y;
    if (ui->mode == MODE_NORMAL) {
        switch(key) {
            case 'q':
                flag = FLAG_QUIT;
                break;
            case 'c':
                ui->mode = MODE_CHAT;
                print_ui_status(ui);
                wmove(ui->command, 0, 0);
                wrefresh(ui->command);
                reset_ui_buffer(ui);
                break;
            case 'j':
                if (ui->main_top < PAD_ROWS - 1) {
                    ui->main_top++;
                } // if
                prefresh(ui->main, ui->main_top, 0, 0, 0, LINES - 3, COLS - 1);
                break;
            case 'k':
                if (ui->main_top > 0) {
                    ui->main_top--;
                } // if
                prefresh(ui->main, ui->main_top, 0, 0, 0, LINES - 3, COLS - 1);
                break;
            default:
                break;
        } // switch
    } else { // in chat mode
        switch(key) {
            case 27: // escape
                ui->mode = MODE_NORMAL;
                wclear(ui->command);
                wrefresh(ui->command);
                print_ui_status(ui);
                break;
            case KEY_BACKSPACE: // backspace
                if (ui->buf_pos != ui->buffer) {
                    ui->buf_pos--;
                    *(ui->buf_pos) = '\0';
                    getyx(ui->command, y, x);
                    wmove(ui->command, y, x - 1);
                    wdelch(ui->command);
                    wrefresh(ui->command);
                } // if
                break; 
            case '\n': // enter
                ui->buf_pos = ui->buffer;
                wclear(ui->command);
                wmove(ui->command, 0, 0);
                wrefresh(ui->command);
                //wprintw(ui->main, "%s\n", ui->buffer);
                //prefresh(ui->main, ui->main_top, 0, 0, 0, LINES - 3, COLS - 1);
                flag = FLAG_SEND;
                break;
            default:
                *(ui->buf_pos) = key;
                ui->buf_pos++;
                waddch(ui->command, key);
                wrefresh(ui->command);
                break;
        } // switch
    } // else
    return flag;
} // handle_key

void print_ui_status(chat_ui *ui) {
    if (ui->mode == MODE_NORMAL) {
        mvwprintw(ui->status, 0, 0, "%s     ", NORMAL_STR);
        curs_set(0);
    } else {
        mvwprintw(ui->status, 0, 0, "%s     ", COMMAND_STR);
        curs_set(1);
    } // else
    wrefresh(ui->status);
} // print_ui_status

void print_ui_main(chat_ui *ui, const char *time_str, const char *message) {
    int y, x, diff;
    wprintw(ui->main, "[%s] %s\n", time_str, message);
    getyx(ui->main, y, x);
    if (ui->main_top + LINES - 2 < y) {
        diff = y - (ui->main_top + LINES - 2);
        ui->main_top += diff;
    } // if
    prefresh(ui->main, ui->main_top, 0, 0, 0, LINES - 3, COLS - 1);
} // print_ui_main

void reset_ui_buffer(chat_ui *ui) {
    memset(ui->buffer, 0, ui->buf_size);
    ui->buf_pos = ui->buffer;
} // reset_ui_buffer
