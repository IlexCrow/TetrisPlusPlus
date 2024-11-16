#include <iostream>
#include <SDL2/SDL.h>
#include "garbage.h"
#define scale 36
#define current_tetramino_arg current_tetramino[0], current_tetramino[1], current_tetramino[2]
const int screen_width = (scale * 12);
const int screen_height = (scale * 22);
bool running = true;
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

short render_board[10][20] = {};
short current_tetramino[3] = {8, 1, 7};

void draw_block(int x, int y) {
    const SDL_Rect rect = {(x + 1) * scale, (y + 1) * scale, scale, scale};
    SDL_RenderFillRect(renderer, &rect);
}

void draw_grid() {
    SDL_SetRenderDrawColor(renderer, 75, 75, 75, 255);
    SDL_Rect rect = {(scale), scale, scale, scale};
    for (int i = 0; i < 20 * 10; i++) {
        const int x = i % 10;
        const int y = i / 10;
        rect.x = (x + 1) * scale;
        rect.y = (y + 1) * scale;
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void draw_board() {
    for (int i = 0; i < 20 * 10; i++) {
        const int x = i % 10;
        const int y = i / 10;
        if (render_board[x][y] == 0) continue;
        const SDL_Colour colour = colours[render_board[x][y]];
        SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
        draw_block(x, y);
    }
}

void draw_tetramino(int tetramino, int _x, int _y) {
    const SDL_Colour colour = colours[tetrominos[tetramino][16]];
    SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

    for (int i = 0; i < 16; i++) {
        if (tetrominos[tetramino][i] == 0) continue;
        const int x = (i % 4) + (_x + 1);
        const int y = i / 4 + (_y + 1);

        draw_block(x, y);
    }
}

void draw() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    const SDL_Rect rect = {(scale), scale, 10 * scale, 20 * scale};
    SDL_RenderFillRect(renderer, &rect);
    draw_grid();
    draw_board();
    draw_tetramino(current_tetramino_arg);

    SDL_RenderPresent(renderer);
}

bool intersects(short tetramino, short _x, short _y) {
    for (int i = 0; i < 16; i++) {
        if (tetrominos[tetramino][i] == 0) continue;
        const int x = (i % 4) + (_x + 1);
        const int y = i / 4 + (_y + 1);
        if (0 > x || 9 < x || y < 0 || y > 19) return true;
        if (render_board[x][y]) return true;
    }
    return false;
}

void rotate(short direction) {
    short change = ((current_tetramino[0] + direction) % 4 == (direction == 1 ? 0 : 3) ? -3 : 1) * direction;
    if (!intersects(current_tetramino[0] + change, current_tetramino[1], current_tetramino[2])) {
        current_tetramino[0] += change;
    }
}

void move(short _x, short _y) {
    // Move by _x,_y provided there is nothing in the way
    if (!intersects(current_tetramino[0], current_tetramino[1] + _x, current_tetramino[2] + _y)) {
        current_tetramino[1] += _x;
        current_tetramino[2] += _y;
    }
}

void process() {
}

int main() {
    render_board[5][5] = 1;
    render_board[5][6] = 2;
    render_board[5][7] = 3;
    render_board[5][8] = 4;
    render_board[4][5] = 5;
    render_board[3][5] = 6;
    render_board[2][5] = 7;


    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, 1, 1);
    SDL_SetWindowTitle(window, "Tetris++");

    Uint64 start = SDL_GetPerformanceCounter(); // FPS cap variable

    while (running) {
        draw();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_SPACE)rotate(1);
                    if (event.key.keysym.sym == SDLK_z)rotate(-1);
                    if (event.key.keysym.sym == SDLK_UP) move(0, -1);
                    if (event.key.keysym.sym == SDLK_DOWN) move(0, 1);
                    if (event.key.keysym.sym == SDLK_LEFT) move(-1, 0);
                    if (event.key.keysym.sym == SDLK_RIGHT) move(1, 0);
                    if (event.key.keysym.sym == SDLK_y) {
                    };
                    break;
            }
        }

        // FPS cap at 60
        Uint64 end = SDL_GetPerformanceCounter();
        float elapsedMS = (end - start) / (float) SDL_GetPerformanceFrequency() * 1000.0f;
        SDL_Delay(floor(16.666f - elapsedMS));
        start = SDL_GetPerformanceCounter();
    }

    return 0;
}
