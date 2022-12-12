#pragma once
#include <vector>
#include <string>

#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_image.h"

using std::vector;
using std::string;

class Tile {
private:
	SDL_Texture* texture;
	SDL_Surface* surface;
	SDL_Rect* tileRect;

public:
	Tile(SDL_Texture* texture, SDL_Rect* tileRect) : texture(texture), tileRect(tileRect), posX(0), posY(0), surface(NULL), onSurface() {}
	Tile() : texture(nullptr), tileRect(nullptr), posX(0), posY(0), surface(NULL), onSurface() {}

	string tileName;
	int posX;
	int posY;
	bool onSurface;

	SDL_Texture* getTexture();
	SDL_Rect* getRect();
	SDL_Surface* getSurface();

	void setTexture(SDL_Renderer* renderer);
	void setSurface(SDL_Surface* sur);
};

