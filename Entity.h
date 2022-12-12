#pragma once
#include "SDL2/Include/SDL.h"

#include <string>

using std::string;

class Entity {
private:
	SDL_Texture* texture;
	SDL_Surface* surface;
	SDL_Rect* rect = new SDL_Rect;

public:
	Entity(string name, int x, int y, bool isStatic, SDL_Texture* texture, SDL_Surface* surface, SDL_Rect* rect, bool isFlippable) : name(name), filePath(""), x(x), y(y), isStatic(isStatic), xVel(0.0f), yVel(0.0f), texture(texture), surface(surface), rect(rect), isFlippable(isFlippable), flipped() {}
	Entity() : name(""), filePath(""), isStatic(), x(0), y(0), xVel(0.0f), yVel(0.0f), texture(nullptr), surface(nullptr), isFlippable(), flipped() {}

	string name;
	string filePath;

	bool isStatic;
	bool isFlippable;
	bool flipped;

	int x;
	int y;
	float xVel;
	float yVel;


	SDL_Texture* getTexture() {
		return texture;
	}

	SDL_Rect* getRect() {
		return rect;
	}
};