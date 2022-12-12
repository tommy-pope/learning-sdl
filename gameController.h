#pragma once
#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_ttf.h"

#include "Player.h"
#include "Tile.h"
#include "Chunk.h"
#include "Perlin.h"

#include <iostream>
#include <vector>
#include <fstream>

class gameController {
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	Player* player;

public:
	gameController(SDL_Window* window, SDL_Renderer* renderer, Player* player) : window(window), renderer(renderer), player(player) {};

	void pixelSet(SDL_Surface*& sur, int x, int y, int r, int g, int b, int a);
	void pixelGet(SDL_Surface* sur, int x, int y, Uint8& r, Uint8& g, Uint8& b);


	void init();
	void loadBiomes();
	void createWorld();

	void chunkController();
	
	void updateVisibleTiles();
	void updateVisibleEntities();

	void renderTiles();
	void renderEntities();

	void backgroundRender();
	void tileCollision();
	void mainLoop();

	// debugging functions

	void debugText();
};