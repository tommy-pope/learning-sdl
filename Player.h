#pragma once

#include <vector>
#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_image.h"

using std::vector;


class Player {
private:

	// SDL properties
	SDL_Renderer* renderer;
	SDL_Texture* currentTexture;
	vector<SDL_Texture*> sprites;
	SDL_Rect* playerRect;

public:
	Player() : x(0), y(0), xVel(0), yVel(0), jumping(true), renderer(NULL), currentTexture(NULL), sprites(NULL), playerRect(playerRect) {};
	Player(SDL_Renderer* renderer) : x(0), y(0), xVel(0), yVel(0), jumping(true), renderer(renderer), currentTexture(NULL), sprites(NULL), playerRect(playerRect) {};

	// physical properties
	int x;
	int y;
	float xVel;
	float yVel;
	bool jumping;


	int init();
	int render(int x, int y);
	int loadSprites();
	int setTexture();
	int setRect();
	SDL_Texture* getTexture();
	SDL_Rect* getRect();
};