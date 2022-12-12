#include "Tile.h"

SDL_Texture* Tile::getTexture()
{
    return texture;
}

SDL_Rect* Tile::getRect() {
    return tileRect;
}

SDL_Surface* Tile::getSurface() {
    return surface;
}

void Tile::setTexture(SDL_Renderer* renderer) {
    texture = SDL_CreateTextureFromSurface(renderer, surface);
}

void Tile::setSurface(SDL_Surface* sur) {
    surface = sur;
}