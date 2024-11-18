#include <iostream>
#include <random>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "garbage.h"
#define scale 36
#define current_tetramino_arg current_tetramino[0], current_tetramino[1], current_tetramino[2]
#define current_tetramino_phantom_arg current_tetramino_phantom[0], current_tetramino_phantom[1], current_tetramino_phantom[2]
const int screen_width = (scale * 21);
const int screen_height = (scale * 22);
bool running = true;
bool paused = false;
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
std::uniform_int_distribution<> dist{0, 6};
// SDL_Surface* overlay_surface;
SDL_Texture* overlay;

short render_board[10][20] = {};
short current_tetramino[4] = {8, 1, 7, 0}; // Piece, x, y, floor timer
short next_tetramino[4] = {8, 1, 7, 0}; // Piece, x, y, floor timer
short current_tetramino_phantom[4] = {0, 0, 0, 0}; // Version to show floor position.
int score = 0;

int game_tick = 0;
int fall_tick = 30;
int ground_tick = 1;

void draw_block(int x, int y) {
    SDL_Rect rect = {(x + 1) * scale, (y + 1) * scale, scale, scale};
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderCopy(renderer, overlay, nullptr, &rect);
}
void draw_block_phantom(int x, int y) {
    const SDL_Rect rect = {(x + 1) * scale, (y + 1) * scale, scale, scale};
    SDL_RenderDrawRect(renderer, &rect);
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
void draw_tetramino_phantom(int tetramino, int _x, int _y) {
    const SDL_Colour colour = colours[tetrominos[tetramino][16]];
    SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

    for (int i = 0; i < 16; i++) {
        if (tetrominos[tetramino][i] == 0) continue;
        const int x = (i % 4) + (_x + 1);
        const int y = i / 4 + (_y + 1);

        draw_block_phantom(x, y);
    }
}
void draw_block_boxes() {
    SDL_SetRenderDrawColor(renderer, 75, 75, 75, 255);
    SDL_Rect rect = {13*scale, 5*scale, int(scale*6), int(scale*6)};
    SDL_RenderFillRect(renderer, &rect);
    rect.y += 7*scale;
    SDL_RenderFillRect(renderer, &rect);
}
void draw_theory_tetraminos() {
    draw_tetramino(0, 12, 4);
    draw_tetramino(10, 12, 11);
}
void draw_ui() {
    draw_block_boxes();
    draw_theory_tetraminos();
}
void draw() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect rect = {(scale), scale, 10 * scale, 20 * scale};
    SDL_RenderFillRect(renderer, &rect);
    draw_grid();
    draw_board();
    draw_tetramino(current_tetramino_arg);
    draw_tetramino_phantom(current_tetramino_phantom_arg);
    draw_ui();


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
bool move_phantom(short _x, short _y) {
    // Move by _x,_y provided there is nothing in the way
    if (!intersects(current_tetramino_phantom[0], current_tetramino_phantom[1] + _x, current_tetramino_phantom[2] + _y)) {
        current_tetramino_phantom[1] += _x;
        current_tetramino_phantom[2] += _y;
        return true;
    }
    return false;
}
bool fall_phantom() {
    return move_phantom(0, 1);
}
void drop_phantom() {
    std::copy(std::begin(current_tetramino), std::end(current_tetramino), std::begin(current_tetramino_phantom));
    while (fall_phantom()){};
}

void rotate(short direction) {
    short change = ((current_tetramino[0] + direction) % 4 == (direction == 1 ? 0 : 3) ? -3 : 1) * direction;
    if (!intersects(current_tetramino[0] + change, current_tetramino[1], current_tetramino[2])) {
        current_tetramino[0] += change;
        drop_phantom();
    }
}
bool move(short _x, short _y) {
    // Move by _x,_y provided there is nothing in the way
    if (!intersects(current_tetramino[0], current_tetramino[1] + _x, current_tetramino[2] + _y)) {
        current_tetramino[1] += _x;
        current_tetramino[2] += _y;
        drop_phantom();
        return true;
    }
    return false;
}
bool fall() {return move(0, 1);}
void drop() {
    while (fall()){};
    // current_tetramino[3]=ground_tick+1;
}
void gen_new_tetramino() {
    std::random_device seed;
    std::mt19937 gen{seed()};
    current_tetramino[0]= dist(gen)*4; // generate number    // render_board[5][5] = 1;
    current_tetramino[1]=3;
    current_tetramino[2]=0;
    current_tetramino[3]=0;
    drop_phantom();
}
void lock_current_tetramino(){
    for (int i = 0; i < 16; i++) {
        if (tetrominos[current_tetramino[0]][i] == 0) continue;
        const int x = (i % 4) + (current_tetramino[1] + 1);
        const int y = i / 4 + (current_tetramino[2] + 1);
        render_board[x][y]=tetrominos[current_tetramino[0]][16];
    }
    gen_new_tetramino();
}
void loose() {
    std::cout << "Final score: "<< score << std::endl;
    running=false;
}
void clear_line(short y) {
    for (short i = 0; i < 10; i++) {
        render_board[i][y]=0;
    }
    for (short j = y; j > 0; j--) {
        // std::copy(std::begin(render_board[i]), std::end(render_board[i]), std::begin(render_board[i][j-1])); // Yay, thanks past me for ordering the 2D array the stupid way :)
        for (short i = 0; i < 10; i++) {
            render_board[i][j] = render_board[i][j-1];
        }
    }
}
void check_lines() {
    short lines_cleared = 0;
    for (short j = 0; j < 20; j++) {
        for (short i = 0; i < 10; i++) {
            if (render_board[i][j]==0){goto CONTINUE;}
        }
        clear_line(j);
        lines_cleared++;
        CONTINUE:
    }
    switch (lines_cleared){
        case 1:
            score += 40;
            break;
        case 2:
            score += 100;
            break;
        case 3:
            score += 300;
        case 4:
            score += 1200;
        default:
            break;
    };
}


void process() {
    check_lines();
    if (game_tick%30==0) {
        if (!fall()) current_tetramino[3]++;
    }
    if (current_tetramino[3]>ground_tick) {
        lock_current_tetramino();
        if (intersects(current_tetramino_arg)){loose();} // Loose condition
    }
}

int main() {
    gen_new_tetramino();
    // overlay = SDL_CreateTextureFromSurface(renderer, overlay_surface);


    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, 1, 1);
    SDL_SetWindowTitle(window, "Tetris++");
    // SDL_SetWindowResizable(window, SDL_TRUE); // Breaks SOOOO much shite
    overlay = IMG_LoadTexture(renderer, "../overlay.png");
    if (!overlay) {
        std::cerr << "Failed to load overlay texture: " << IMG_GetError() << std::endl;
        return -1;
    }

    Uint64 start = SDL_GetPerformanceCounter(); // FPS cap variable

    while (running) {
        game_tick++;
        if (!paused) {
            process();
        }
        draw();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)(paused = !paused);
                    if (event.key.keysym.sym == SDLK_SPACE)drop();
                    if (event.key.keysym.sym == SDLK_z)rotate(-1);
                    if (event.key.keysym.sym == SDLK_UP) rotate(1);
                    if (event.key.keysym.sym == SDLK_DOWN) move(0, 1);
                    if (event.key.keysym.sym == SDLK_LEFT) move(-1, 0);
                    if (event.key.keysym.sym == SDLK_RIGHT) move(1, 0);
                    if (event.key.keysym.sym == SDLK_y) {
                        clear_line(7);
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
