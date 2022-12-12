#define SDL_MAIN_HANDLED

// SDL library includes
#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_image.h"
#include "SDL2/include/SDL_events.h"
#include "SDL2/include/SDL_ttf.h"

// class includes
#include "gameController.h"

using std::cout;
using std::endl;

int main() {
	// needed to use own defined main function, instead of SDL predefined
	SDL_SetMainReady();

	// initializes every SDL subsystem
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		cout << "SDL failed to initialize: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}

	int flags = IMG_Init(IMG_INIT_PNG);

	if (flags != IMG_INIT_PNG) {
		cout << "Error." << endl;
		SDL_Quit();
		return 1;
	}

	TTF_Init();

	cout << "SDL initialized successfully." << endl;

	// creates SDL window
	SDL_Window* window = SDL_CreateWindow("Testing Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, 0);

	// creates renderer object, with target of window, index of rendering driver (default -1), and any flags
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	Player p(renderer);
	gameController gc(window, renderer, &p);
	gc.init();

	IMG_Quit();
	SDL_Quit();

	return 0;
}