#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <cmath>

#define BLOCK 24
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define PI 3.14159265

// Define structures
struct Vec2 {
    float x, y;
};

struct Ray {
    Vec2 origin;
    float angle;
};

// Define map
std::vector<std::vector<int>> map = {
    {1, 1, 1, 1, 1,1,1,1,1,1},
    {1, 0, 0, 0, 0,0,0,0,0,1},
    {1, 0, 1, 0, 0,0,0,0,0,1},
    {1, 0, 0, 0, 0,0,0,0,0,1},
    {1, 0, 0, 0, 1,0,0,0,0,1},
    {1, 0, 0, 0, 0,0,0,0,1,1},
    {1, 0, 0, 0, 0,0,0,0,0,1},
    {1, 1, 1, 1, 1,1,1,1,1,1}
};

// Raycasting function
void castRays(Vec2 cameraPos, float cameraAngle, int screenWidth, int screenHeight, SDL_Renderer* renderer) {
    for (int x = 0; x < screenWidth; ++x) {
        float angle = cameraAngle + (x - screenWidth / 2) * (60.0f / screenWidth);
        Ray ray = {cameraPos, angle};

        // Calculate ray direction
        float rayDirX = cos(angle * PI / 180);
        float rayDirY = sin(angle * PI / 180);

        // Delta distance for x and y
        float deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
        float deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));

        // Step for x and y
        int stepX, stepY;
        int mapX = static_cast<int>(ray.origin.x / BLOCK);
        int mapY = static_cast<int>(ray.origin.y / BLOCK);

        float sideDistX, sideDistY;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (ray.origin.x - mapX * BLOCK) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX * BLOCK + BLOCK - ray.origin.x) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (ray.origin.y - mapY * BLOCK) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY * BLOCK + BLOCK - ray.origin.y) * deltaDistY;
        }

        // Perform DDA (Digital Differential Analysis) algorithm
        bool hit = false;
        while (!hit) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
            }

            if (map[mapY][mapX] == 1) {
                hit = true;
            }
        }

        // Calculate distance to the wall
        float perpWallDist;
        if (sideDistX < sideDistY) {
            perpWallDist = (mapX - ray.origin.x + (1 - stepX) / 2) / rayDirX;
        } else {
            perpWallDist = (mapY - ray.origin.y + (1 - stepY) / 2) / rayDirY;
        }

        // Calculate height of the line to draw on the screen
        int lineHeight = static_cast<int>(screenHeight / perpWallDist) * 2; // Adjusted for increased wall height

        // Calculate start and end points for the line
        int drawStart = -lineHeight / 2 + screenHeight / 2;
        if (drawStart < 0) {
            drawStart = 0;
        }
        int drawEnd = lineHeight / 2 + screenHeight / 2;
        if (drawEnd >= screenHeight) {
            drawEnd = screenHeight - 1;
        }

        // Draw the vertical line on the screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, x, drawStart, x, drawEnd);
    }
}


int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Raycasting with SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (window == nullptr || renderer == nullptr) {
        std::cerr << "Failed to create window or renderer: " << SDL_GetError() << std::endl;
        return 1;
    }

    Vec2 cameraPos = {2.5f * BLOCK, 2.5f * BLOCK}; // Initial player position
    float cameraAngle = 0.0f; // Initial player angle
    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_z:
                        // Calculate movement vector based on player's angle
                        cameraPos.x += 5.0f * cos(cameraAngle * PI / 180);
                        cameraPos.y += 5.0f * sin(cameraAngle * PI / 180);
                        break;
                    case SDLK_s:
                        cameraPos.x -= 5.0f * cos(cameraAngle * PI / 180);
                        cameraPos.y -= 5.0f * sin(cameraAngle * PI / 180);
                        break;
                    case SDLK_q:
                        cameraAngle -= 5.0f; // Rotate player left
                        break;
                    case SDLK_d:
                        cameraAngle += 5.0f; // Rotate player right
                        break;
                    default:
                        break;
                }
            }
        }

        // Clear the renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the player as a small dot
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawPoint(renderer, static_cast<int>(cameraPos.x), static_cast<int>(cameraPos.y));

        // Cast rays and render the scene
        castRays(cameraPos, cameraAngle, SCREEN_WIDTH, SCREEN_HEIGHT, renderer);

        // Present renderer
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
