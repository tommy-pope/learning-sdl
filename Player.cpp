#include "Player.h"
#include <iostream>
using std::cout;
using std::endl;

int Player::init() {
	x = 960;
	y = 500;
	loadSprites();
	setTexture();
	setRect();
	return 0;
}

// sets player rect parameter
int Player::setRect() {
	playerRect = new SDL_Rect;
	playerRect->w = 128;
	playerRect->h = 128;
	return 0;
}

SDL_Rect* Player::getRect() {
	return playerRect;
}

// sets current texture of player
int Player::setTexture() {
	currentTexture = sprites[0];
	return 0;
}

// simply returns the texture stored in the Player object
SDL_Texture* Player::getTexture() {
	return currentTexture;
}

// sends the current texture the player is storing to the renderer
int Player::render(int x, int y) {
	playerRect->x = 960;
	playerRect->y = 668;
	SDL_RenderCopy(renderer, getTexture(), NULL, playerRect);
	return 0;
}

// loads img of player and adds to vector
int Player::loadSprites() {
	SDL_Surface* img = IMG_Load("assets/sprites/playerSprite.png");
	SDL_Texture* imgTexture = SDL_CreateTextureFromSurface(renderer, img);
	sprites.push_back(imgTexture);
	return 0;
}