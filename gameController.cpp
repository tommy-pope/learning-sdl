#include "gameController.h"


#include <algorithm>
#include <sstream>

using std::fstream;
using std::cout;
using std::endl;
using std::getline;
using std::stringstream;

float gravity = .07f;

int gameMinutes = 0;
int gameHours = 12;

float sunX = 0;
float sunY = 0;

float moonX = 0;
float moonY = 0;

vector<Chunk> chunks;
vector<Chunk> loadedChunks;
vector<Tile> visibleTiles;
vector<Entity> visibleStaticEntities;

vector<Biome> biomeList;

void gameController::loadBiomes() {
	fstream biomeFile;
	biomeFile.open("data/biomes.csv", std::ios::in);

	string line, word, tmp;

	vector<string> biomeElements;
	vector<string> entityElements;

	int biomeIndex = -1;

	while (getline(biomeFile, line)) {
		biomeElements.clear();
		entityElements.clear();

		stringstream s(line);

		bool biomeFound = false;
		bool biomeElementAdding = false;

		while (getline(s, word, ',')) {

			if (word == "biome") {
				biomeFound = true;
				biomeIndex++;
			}
			else if (biomeFound) {
				biomeElements.push_back(word);
			}
			else if (word == "entity") {
				biomeElementAdding = true;
			}
			else if (biomeElementAdding) {
				entityElements.push_back(word);
			}
		}

		if (biomeFound) {
			Biome b;
			b.biomeType = biomeElements[0];
			b.amplitude = stof(biomeElements[1]);
			b.freq = stof(biomeElements[2]);
			b.tilePath = biomeElements[3];

			biomeList.push_back(b);
		}
		else if (biomeElementAdding) {
			Entity e;
			e.name = entityElements[0].c_str();
			e.filePath = entityElements[1];
			e.getRect()->w = stoi(entityElements[2]);
			e.getRect()->h = stoi(entityElements[3]);
			
			if (entityElements[4] == "static") {
				e.isStatic = true;
				biomeList[biomeIndex].staticBiomeEntityList.push_back(e);

				if (entityElements[5] == "flippable") {
					e.isFlippable = true;
				}
				else {
					e.isFlippable = false;
				}
			}
			else {
				e.isStatic = false;
				biomeList[biomeIndex].nonStaticBiomeEntityList.push_back(e);

				if (entityElements[5] == "flippable") {
					e.isFlippable = true;
				}
				else {
					e.isFlippable = false;
				}
			}
		}


		biomeFound = false;
		biomeElementAdding = false;
	}

	biomeFile.close();
}

void gameController::init() {
	player->init();
	loadBiomes();
	createWorld();
	mainLoop();
}

/*
	TO-DO now:
		- continue work on entity generation, get default entites in each biome

	TO-DO later:
		- create transitional biomes between different biomes, so the difference in color/atmosphere is not as abrupt
		- character sprite and animations
		- more movement options
		- change the sky so it is not bland, sunset/sunrise
		- background textures for biomes, so sky is not bland
		- enemies

	Known problems:
		- dynamic pixel removal is not perfect, some weird pixels get removed sometimes
		- flickering on left side of screen when moving quick, only in dark biomes (deadlands/grassland)
		- chunk borders dont get affected by the dynamic pixel removal
*/

// sets a pixel on surface sur at pos x, y
void gameController::pixelSet(SDL_Surface*& sur, int x, int y, int r, int g, int b, int a) {
	Uint32* pixels = (Uint32*)sur->pixels;
	Uint32* pixel = pixels + y * sur->pitch / 4 + x;

	*pixel = SDL_MapRGBA(sur->format, r, g, b, a);
}

// gets the rbg values of a pixel at x y
void gameController::pixelGet(SDL_Surface* sur, int x, int y, Uint8& r, Uint8& g, Uint8& b) {
	Uint32* pixels = (Uint32*)sur->pixels;
	Uint32* pixel = pixels + y * sur->pitch / 4 + x;
	
	SDL_Color color;
	SDL_GetRGB(*pixel, sur->format, &color.r, &color.g, &color.b);
	r = color.r;
	g = color.g;
	b = color.b;
}

void gameController::createWorld() {
	Perlin p;
	p.init();

	Biome currentBiome;
	int currentBiomeCounter = 0;

	for (int x = -15; x < 15; x++) {
		if (currentBiome.biomeType == "") {
			int choice = rand() % biomeList.size();

			currentBiome = biomeList[choice];
		}

		if (currentBiomeCounter > 5) {
			int random = rand() % 10 + 1;

			if (random > 5) {
				int choice = rand() % biomeList.size();
				currentBiome = biomeList[choice];
				currentBiomeCounter = 0;
			}
		}

		Chunk c(1920 * x, 1920 * x + 1920);
		c.chunkBiome = currentBiome;

		for (int i = 0; i < 60; i++) {
			float n = p.noise(i * currentBiome.freq) * currentBiome.amplitude;
			int y = n + 800;
			y = (((y + 8 / 2) / 8) * 8);

			SDL_Surface* allocatedSur = IMG_Load(currentBiome.tilePath.c_str());
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, allocatedSur);
			SDL_Rect* rect = new SDL_Rect;

			rect->x = i * 32 + c.startX;
			rect->y = y;
			rect->w = 32;
			rect->h = 32;

			Tile t(texture, rect);
			t.posX = rect->x;
			t.posY = rect->y;
			t.onSurface = true;
			t.setSurface(allocatedSur);

			c.chunkTiles.push_back(t);

			SDL_Surface* allocatedSur2 = SDL_CreateRGBSurface(0, 32, 1080 - y + 32, 32, 0, 0, 0, 0);

			// randomly fills in ground texture

			Uint8 r1, r2, r3, r4, r5, g1, g2, g3, g4, g5, b1, b2, b3, b4, b5;

			pixelGet(allocatedSur, 7, 7, r1, g1, b1);
			pixelGet(allocatedSur, 6, 6, r2, g2, b2);
			pixelGet(allocatedSur, 3, 6, r3, g3, b3);
			pixelGet(allocatedSur, 4, 7, r4, g4, b4);
			pixelGet(allocatedSur, 5, 5, r5, g5, b5);

			for (int p = 0; p < 32; p++) {
				if (p > 0) {
					p--;
					p += 4;

					if (p > 31) {
						break;
					}
				}

				for (int py = 0; py < (1080 - y + 32); py++) {
					if (py > 0) {
						py--;
						py += 4;

						if (py > (1080 - y + 32) - 1) {
							break;
						}
					}

					float choice = (rand() % 100);

					if (choice <= 20) {
						pixelSet(allocatedSur2, p, py, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p+1, py, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p+2, py, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p+3, py, r1, g1, b1, 255);

						pixelSet(allocatedSur2, p, py + 1, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 1, py + 1, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 2, py + 1, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 3, py + 1, r1, g1, b1, 255);

						pixelSet(allocatedSur2, p, py + 2, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 1, py+2, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 2, py+2, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 3, py+2, r1, g1, b1, 255);

						pixelSet(allocatedSur2, p, py+3, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 1, py+3, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 2, py+3, r1, g1, b1, 255);
						pixelSet(allocatedSur2, p + 3, py+3, r1, g1, b1, 255);
					}
					else if (choice > 20 && choice <= 40) {
						pixelSet(allocatedSur2, p, py, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p+1, py, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p+2, py, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p+3, py, r2, g2, b2, 255);

						pixelSet(allocatedSur2, p, py+1, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 1, py+1, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 2, py+1, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 3, py+1, r2, g2, b2, 255);

						pixelSet(allocatedSur2, p, py+2, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 1, py+2, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 2, py+2, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 3, py+2, r2, g2, b2, 255);

						pixelSet(allocatedSur2, p, py+3, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 1, py+3, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 2, py+3, r2, g2, b2, 255);
						pixelSet(allocatedSur2, p + 3, py+3, r2, g2, b2, 255);
					}
					else if (choice > 40 && choice <= 60) {
						pixelSet(allocatedSur2, p, py, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p+1, py, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p+2, py, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p+3, py, r3, g3, b3, 255);

						pixelSet(allocatedSur2, p, py+1, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 1, py+1, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 2, py+1, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 3, py+1, r3, g3, b3, 255);

						pixelSet(allocatedSur2, p, py+2, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 1, py+2, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 2, py+2, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 3, py+2, r3, g3, b3, 255);

						pixelSet(allocatedSur2, p, py+3, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 1, py+3, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 2, py+3, r3, g3, b3, 255);
						pixelSet(allocatedSur2, p + 3, py+3, r3, g3, b3, 255);
					}
					else if (choice > 60 && choice <= 80) {
						pixelSet(allocatedSur2, p, py, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p+1, py, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p+2, py, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p+3, py, r4, g4, b4, 255);

						pixelSet(allocatedSur2, p, py+1, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 1, py+1, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 2, py+1, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 3, py+1, r4, g4, b4, 255);

						pixelSet(allocatedSur2, p, py+2, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 1, py+2, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 2, py+2, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 3, py+2, r4, g4, b4, 255);

						pixelSet(allocatedSur2, p, py+3, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 1, py+3, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 2, py+3, r4, g4, b4, 255);
						pixelSet(allocatedSur2, p + 3, py+3, r4, g4, b4, 255);
					}
					else if (choice > 80 && choice <= 100) {
						pixelSet(allocatedSur2, p, py, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p+1, py, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p+2, py, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p+3, py, r5, g5, b5, 255);

						pixelSet(allocatedSur2, p, py+1, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 1, py+1, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 2, py+1, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 3, py+1, r5, g5, b5, 255);

						pixelSet(allocatedSur2, p, py+2, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 1, py+2, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 2, py+2, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 3, py+2, r5, g5, b5, 255);

						pixelSet(allocatedSur2, p, py+3, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 1, py+3, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 2, py+3, r5, g5, b5, 255);
						pixelSet(allocatedSur2, p + 3, py+3, r5, g5, b5, 255);
					}
				}
			}

			SDL_Texture* texture2 = SDL_CreateTextureFromSurface(renderer, allocatedSur2);

			SDL_Rect* rect2 = new SDL_Rect;
			rect2->x = i * 32 + c.startX;
			rect2->y = y + 32;
			rect2->w = 32;
			rect2->h = 1080 - y + 32;

			Tile fill(texture2, rect2);
			fill.posX = rect2->x;
			fill.posY = rect2->y;
			fill.onSurface = false;
			fill.setSurface(allocatedSur2);

			c.chunkTiles.push_back(fill);

		}
		
		// think of a better way to implement entity generation in biomes, how to store possible entities in each biome
		// maybe a file which gets loaded before the creation of the world, with a list of possible entity types in
		// each biome and some properties about them (size, sprite file path, static, etc)

		// also in world generation, how to decide which entity gets created and when?
		for (int e = 0; e < c.chunkTiles.size(); e++) {
			int randomE = rand() % 100;

			if (randomE < 2) {
				int secondRand = rand() % c.chunkBiome.staticBiomeEntityList.size();

				SDL_Surface* surface = IMG_Load(c.chunkBiome.staticBiomeEntityList[secondRand].filePath.c_str());
				SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
				SDL_Rect* rect = new SDL_Rect;

				rect->x = c.chunkTiles[e].posX;
				rect->y = c.chunkTiles[e].posY - c.chunkBiome.staticBiomeEntityList[secondRand].getRect()->w;
				rect->h = c.chunkBiome.staticBiomeEntityList[secondRand].getRect()->h;
				rect->w = c.chunkBiome.staticBiomeEntityList[secondRand].getRect()->h;

				Entity newEntity(c.chunkBiome.staticBiomeEntityList[secondRand].name, rect->x, rect->y, currentBiome.staticBiomeEntityList[0].isStatic, texture, surface, rect, c.chunkBiome.staticBiomeEntityList[secondRand].isFlippable);
				
				if (newEntity.isFlippable) {
					int random = rand() % 10;

					if (random > 4) {
						newEntity.flipped = true;
					}
					else {
						newEntity.flipped = false;
					}
				}

				c.staticEntities.push_back(newEntity);
			}
		}

		chunks.push_back(c);
		currentBiomeCounter++;
	}

	// after tiles/chunks are made
	for (int x = 0; x < chunks.size(); x++) {
		// https://www.youtube.com/watch?v=-SEfmcy6d-k&t=171s i hate this pixel manipulation stuff, but this guy is the goat.

		// edits surface of image to reflect whether there is a difference in elevation on either side of the tile
		// gets angle between midpoint of tile and end of tile, then checks pixel by pixel and finds pixels within that angle
		// if they are inside of that angle, delete them

		// problems:
		//	- if amp is too high, meaning difference between tiles is too big,
		//	too many pixels get deleted, not a smooth transition between tiles
		//  - doesn't consider chunk bounds, tiles on seperate chunks will not be affected

		for (int i = 0; i < chunks[x].chunkTiles.size(); i++) {

			if (i > 0 && chunks[x].chunkTiles[i].onSurface) {

				// if left tile is lower
				if (chunks[x].chunkTiles[i].posY < chunks[x].chunkTiles[i-2].posY) {
					float diff = chunks[x].chunkTiles[i - 2].posY - chunks[x].chunkTiles[i].posY;
					float len = 16;
					float hypo = sqrt(diff * diff + len * len);

					float angle = asin(diff / hypo) * 180.0 / M_PI;

					for (int px = 0; px < 4; px++) {
						float pxDiff = 16 - px * 4;
						for (int py = 0; py < diff / 4; py++) {

							float pyDiff = py * 4 + 4;
							float hypo2 = sqrt(pxDiff * pxDiff + pyDiff * pyDiff);
							float angle2 = asin(pyDiff / hypo) * 180.0 / M_PI;

							if (angle2 <= angle / 2) {
								SDL_Surface* sur = chunks[x].chunkTiles[i].getSurface();
								pixelSet(sur, px, py, 0, 0, 0, 0);
								chunks[x].chunkTiles[i].setTexture(renderer);
							}
						}
					}

					// if right tile is lower
				} else if (chunks[x].chunkTiles[i].posY > chunks[x].chunkTiles[i - 2].posY) {
					float diff = chunks[x].chunkTiles[i].posY - chunks[x].chunkTiles[i - 2].posY;
					float len = 16;
					float hypo = sqrt(diff * diff + len * len);

					float angle = asin(diff / hypo) * 180.0 / M_PI;

					for (int px = 7; px > 4; px--) {
						float pxDiff = px * 4 - 16;
						for (int py = 0; py < diff / 4; py++) {

							float pyDiff = py * 4 + 4;
							float hypo2 = sqrt(pxDiff * pxDiff + pyDiff * pyDiff);
							float angle2 = asin(pyDiff / hypo) * 180.0 / M_PI;

							if (angle2 <= angle / 2) {
								SDL_Surface* sur = chunks[x].chunkTiles[i - 2].getSurface();
								pixelSet(sur, px, py, 0, 0, 0, 0);
								chunks[x].chunkTiles[i - 2].setTexture(renderer);
							}
						}
					}
				}
			}
		}
	}
}

void gameController::chunkController() {
	int leftLimit = round(player->x) - 3840;
	int rightLimit = round(player->x) + 3840;

	for (int i = 0; i < chunks.size(); i++) {

		// if left limit falls inside of a chunk, push that chunk
		if (chunks[i].startX <= leftLimit && leftLimit <= chunks[i].stopX) {
			loadedChunks.push_back(chunks[i]);
			chunks.erase(chunks.begin() + i);
			i--;
		// if entire chunk is to the right of the left limit
		} else if (chunks[i].startX >= leftLimit && chunks[i].stopX >= leftLimit && chunks[i].stopX <= player->x) {
			loadedChunks.push_back(chunks[i]);
			chunks.erase(chunks.begin() + i);
			i--;
		// if right limits falls inside of a chunk
		} else if (chunks[i].startX >= rightLimit && rightLimit >= chunks[i].stopX) {
			loadedChunks.push_back(chunks[i]);
			chunks.erase(chunks.begin() + i);
			i--;
		// if entire chunk is to the left of the right limit
		} else if (chunks[i].startX <= rightLimit && chunks[i].stopX <= rightLimit && chunks[i].stopX >= player->x) {
			loadedChunks.push_back(chunks[i]);
			chunks.erase(chunks.begin() + i);
			i--;
		}

	}

	for (int i = 0; i < loadedChunks.size(); i++) {

		if (leftLimit > loadedChunks[i].stopX || rightLimit < loadedChunks[i].startX) {
			chunks.push_back(loadedChunks[i]);
			loadedChunks.erase(loadedChunks.begin() + i);
			i--;
		}
	}

}

void gameController::updateVisibleTiles() {
	for (int i = 0; i < loadedChunks.size(); i++) {
		for (int x = 0; x < loadedChunks[i].chunkTiles.size(); x++) {
			// check if within 960 x from player on either side
			int diff = std::max(loadedChunks[i].chunkTiles[x].posX, player->x) - std::min(loadedChunks[i].chunkTiles[x].posX, player->x);
			int diff2 = std::max(diff, 960) - std::min(diff, 960);

			// idek dude, in order to prevent visible tile loading we need <= 32 || diff > 960 && diff2 <= 32
			if (diff <= 960 || diff > 960 && diff2 <= 32) {
				bool found = false;
				for (int z = 0; z < visibleTiles.size(); z++) {
					if (loadedChunks[i].chunkTiles[x].posX == visibleTiles[z].posX && loadedChunks[i].chunkTiles[x].posY == visibleTiles[z].posY) {
						found = true;
						break;
					}
				}
				
				if (!found) {
					visibleTiles.push_back(loadedChunks[i].chunkTiles[x]);
				}
			}
		}
	}

	for (int i = 0; i < visibleTiles.size(); i++) {
		int diff = std::max(visibleTiles[i].posX, player->x) - std::min(visibleTiles[i].posX, player->x);
		int diff2 = std::max(diff, 960) - std::min(diff, 960);
 
		if (diff > 960 && diff2 <= 32) {

		}
		else if (diff > 960) {
			visibleTiles.erase(visibleTiles.begin() + i);
			i--;
		}
	}
}

void gameController::updateVisibleEntities() {
	for (int i = 0; i < loadedChunks.size(); i++) {
		for (int x = 0; x < loadedChunks[i].staticEntities.size(); x++) {

			// check if within 960 x from player on either side
			int diff = std::max(loadedChunks[i].staticEntities[x].x, player->x) - std::min(loadedChunks[i].staticEntities[x].x, player->x);
			int diff2 = std::max(diff, 960) - std::min(diff, 960);

			// idek dude, in order to prevent visible tile loading we need <= 32 || diff > 960 && diff2 <= 32
			if (diff <= 960 || diff > 960 && diff2 <= 16) {
				bool found = false;
				for (int z = 0; z < visibleStaticEntities.size(); z++) {
					if (loadedChunks[i].staticEntities[x].x == visibleStaticEntities[z].x && loadedChunks[i].staticEntities[x].y == visibleStaticEntities[z].y) {
						found = true;
						break;
					}
				}

				if (!found) {
					visibleStaticEntities.push_back(loadedChunks[i].staticEntities[x]);
				}
			}
		}
	}

	for (int i = 0; i < visibleStaticEntities.size(); i++) {
		int diff = std::max(visibleStaticEntities[i].x, player->x) - std::min(visibleStaticEntities[i].x, player->x);
		int diff2 = std::max(diff, 960) - std::min(diff, 960);

		if (diff > 960 && diff2 <= 16) {

		}
		else if (diff > 960) {
			visibleStaticEntities.erase(visibleStaticEntities.begin() + i);
			i--;
		}
	}
}

void gameController::renderTiles() {

	// problems:
	// - the fix for the texture on the left side of the screen looks kinda bad, maybe
	//   dynamically change the texture based on how much is showing instead to avoid a shrinking or growing
	//   effect?

	for (int i = 0; i < visibleTiles.size(); i++) {
		int diff = std::max(visibleTiles[i].posX, player->x) - std::min(visibleTiles[i].posX, player->x);
		int yDiff = std::max(visibleTiles[i].posY, player->y) - std::min(visibleTiles[i].posY, player->y);

		if (visibleTiles[i].posX < player->x) {
			if (diff > 960) {
				visibleTiles[i].getRect()->x = 0;
				visibleTiles[i].getRect()->w = abs(diff - 960 - 32);
			}
			else {
				visibleTiles[i].getRect()->x = std::max(diff, 960) - std::min(diff, 960);
				visibleTiles[i].getRect()->w = 32;
			}

		}

		if (visibleTiles[i].posX == round(player->x)) {
			visibleTiles[i].getRect()->x = 960;
		}

		if (visibleTiles[i].posX > player->x) {
			visibleTiles[i].getRect()->x = diff + 960;
		}

		if (visibleTiles[i].posY < player->y) {
			visibleTiles[i].getRect()->y = abs(yDiff - 668);
		}

		if (visibleTiles[i].posY == round(player->y)) {
			visibleTiles[i].getRect()->y = 668;
		}

		if (visibleTiles[i].posY > player->y) {
			visibleTiles[i].getRect()->y = yDiff + 668;
		}
	}

	std::sort(visibleTiles.begin(), visibleTiles.end(), [](Tile& t1, Tile& t2) {
			return t1.getRect()->x < t2.getRect()->x;
		});

	for (int i = 0; i < visibleTiles.size(); i++) {
		SDL_RenderCopy(renderer, visibleTiles[i].getTexture(), NULL, visibleTiles[i].getRect());
	}
}

void gameController::renderEntities() {
	for (int i = 0; i < visibleStaticEntities.size(); i++) {

		int diff = std::max(visibleStaticEntities[i].x, player->x) - std::min(visibleStaticEntities[i].x, player->x);
		int yDiff = std::max(visibleStaticEntities[i].y, player->y) - std::min(visibleStaticEntities[i].y, player->y);

		if (visibleStaticEntities[i].x < player->x) {
			if (diff > 960) {
				visibleStaticEntities[i].getRect()->x = 0;
				visibleStaticEntities[i].getRect()->w = abs(diff - 960 - 32);
			}
			else {
				visibleStaticEntities[i].getRect()->x = std::max(diff, 960) - std::min(diff, 960);
				visibleStaticEntities[i].getRect()->w = 32;
			}

		}

		if (visibleStaticEntities[i].x == round(player->x)) {
			visibleStaticEntities[i].getRect()->x = 960;
		}

		if (visibleStaticEntities[i].x > player->x) {
			visibleStaticEntities[i].getRect()->x = diff + 960;
		}

		if (visibleStaticEntities[i].y < player->y) {
			visibleStaticEntities[i].getRect()->y = abs(yDiff - 668);
		}

		if (visibleStaticEntities[i].y == round(player->y)) {
			visibleStaticEntities[i].getRect()->y = 668;
		}

		if (visibleStaticEntities[i].y > player->y) {
			visibleStaticEntities[i].getRect()->y = yDiff + 668;
		}
	}

	std::sort(visibleStaticEntities.begin(), visibleStaticEntities.end(), [](Entity& t1, Entity& t2) {
		return t1.getRect()->x < t2.getRect()->x;
		});

	for (int i = 0; i < visibleStaticEntities.size(); i++) {
		if (visibleStaticEntities[i].flipped) {
			SDL_RenderCopyEx(renderer, visibleStaticEntities[i].getTexture(), NULL, visibleStaticEntities[i].getRect(), 0.0f, NULL, SDL_FLIP_HORIZONTAL);
		}
		else {
			SDL_RenderCopyEx(renderer, visibleStaticEntities[i].getTexture(), NULL, visibleStaticEntities[i].getRect(), 0.0f, NULL, SDL_FLIP_NONE);
		}
	}
}

// dynamically change the color of the sky, maybe use a gradient effect for sunset/sunrise
void gameController::backgroundRender() {
	int input = gameMinutes + gameHours * 60;

	if (input > 1150 || input < 230) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	}
	else {
		SDL_SetRenderDrawColor(renderer, 173, 216, 230, 255);
	}

	SDL_RenderClear(renderer);

	SDL_Surface* sunSurface = IMG_Load("assets/sprites/sunSprite.png");
	SDL_Texture* sunTexture = SDL_CreateTextureFromSurface(renderer, sunSurface);
	SDL_Rect* sunRect = new SDL_Rect;

	int inputStart = 0;
	int inputEnd = 1439;

	int outputStart = 0;
	int outputEnd = 1888;

	sunX = (input - inputStart) * (outputEnd - outputStart) / (inputEnd - inputStart) + outputStart;

	sunY = (.002*pow((sunX - 944), 2));

	sunRect->x = sunX;
	sunRect->y = sunY;
	sunRect->w = 128;
	sunRect->h = 128;

	SDL_RenderCopy(renderer, sunTexture, NULL, sunRect);
	SDL_FreeSurface(sunSurface);

	SDL_Surface* moonSurface = IMG_Load("assets/sprites/moonSprite.png");
	SDL_Texture* moonTexture = SDL_CreateTextureFromSurface(renderer, moonSurface);
	SDL_Rect* moonRect = new SDL_Rect;

	input = (gameMinutes + gameHours * 60) + 600;

	if (input > 1439) {
		input = input - 1439;
	}

	inputStart = 0;
	inputEnd = 1439;

	outputStart = 0;
	outputEnd = 1888;

	moonX = (input - inputStart) * (outputEnd - outputStart) / (inputEnd - inputStart) + outputStart;

	moonY = (.002 * pow((moonX-944), 2));

	moonRect->x = moonX;
	moonRect->y = moonY;
	moonRect->w = 128;
	moonRect->h = 128;

	SDL_RenderCopy(renderer, moonTexture, NULL, moonRect);
	SDL_FreeSurface(moonSurface);
}

void gameController::tileCollision() {
	int startX = round(player->x) + 32;
	int stopX = startX + player->getRect()->w - 32;

	int startY = round(player->y);
	int stopY = startY + player->getRect()->h;

	for (int i = 0; i < visibleTiles.size(); i++) {
		if (visibleTiles[i].onSurface) {
			int tileStartY = visibleTiles[i].posY;
			int tileStopY = visibleTiles[i].posY + visibleTiles[i].getRect()->h;

			int tileStartX = visibleTiles[i].posX;
			int tileStopX = visibleTiles[i].posX + visibleTiles[i].getRect()->w;

			// if tile startY or tile stopY is in between startY and stopY

			/*
			if (tileStartY >= startY && tileStartY <= stopY) {

			}
			else if (tileStopY >= startY && tileStopY <= stopY) {

			}
			*/

			// if player is touching from above
			if (stopY >= tileStartY && stopX >= tileStartX && stopX <= tileStopX) {
				if (player->jumping && player->yVel < 0) {

				}
				else if (player->jumping && player->yVel >= 0) {
					player->yVel = 0;
					player->y = tileStartY - 128;
					player->jumping = false;
				}
				else {
					player->yVel = 0;
					player->y = tileStartY - 128;
				}
			}
			else if (stopY >= tileStartY && startX >= tileStartX && startX <= tileStopX) {
				if (player->jumping && player->yVel < 0) {

				}
				else if (player->jumping && player->yVel >= 0) {
					player->yVel = 0;
					player->y = tileStartY - 128;
					player->jumping = false;
				}
				else {
					player->yVel = 0;
					player->y = tileStartY - 128;
				}
			}
		}
	}
}

void gameController::mainLoop() {
	int fps = 144;
	int frameDelay = 1000 / fps;
	Uint32 frameStart;
	int frameTime;

	int nextMinute = 0;

	Uint64 lastUpdate = SDL_GetTicks64();

	while (true) {
		frameStart = SDL_GetTicks();
		
		SDL_Event event;
		// polls for current events
		while (SDL_PollEvent(&event)) {
		}

		const Uint8* keystates = SDL_GetKeyboardState(NULL);

		if (keystates[SDL_SCANCODE_A]) {
			player->xVel -= 1;
		}

		if (keystates[SDL_SCANCODE_D]) {
			player->xVel += 1;
		}

		if (keystates[SDL_SCANCODE_SPACE]) {
			if (!player->jumping) {
				player->yVel = -3;
				player->jumping = true;
			}
		}

		// if A and D are not being pressed, slow x velocity
		if (!keystates[SDL_SCANCODE_A] && !keystates[SDL_SCANCODE_D]) {
			if (player->xVel < 0) {
				player->xVel += 15;

				if (player->xVel > 0) {
					player->xVel = 0;
				}
			} else {
				player->xVel -= 15;

				if (player->xVel < 0) {
					player->xVel = 0;
				}
			}
		}

		if (player->xVel > 3) {
			player->xVel = 3;
		}
		else if (player->xVel < -3) {
			player->xVel = -3;
		}

		if (keystates[SDL_SCANCODE_0]) {
			return;
		}

		////////////////////////////////////////////////////////////////

		// physics
		tileCollision();
		
		/////////////////////////////////////////////

		if (nextMinute > 144) {
			nextMinute = 0;
			gameMinutes++;
			
			if (gameMinutes > 59) {
				gameHours++;
				gameMinutes = 0;
				if (gameHours > 23) {
					gameHours = 0;
				}
			}
		}

		chunkController();
		updateVisibleTiles();
		updateVisibleEntities();

		// renderering

		frameTime = SDL_GetTicks() - frameStart;

		if (frameDelay > frameTime) {
			SDL_Delay(frameDelay - frameTime);
		}

		player->yVel += gravity;

		if (player->yVel > 4) {
			player->yVel = 4;
		}

		player->x += player->xVel;
		player->y += player->yVel;

		backgroundRender();
		renderEntities();
		renderTiles();
		//debugText();

		player->render(player->x, player->y);
		SDL_RenderPresent(renderer);

		
		nextMinute++;
	}
}

void gameController::debugText() {
	TTF_Font* font;
	font = TTF_OpenFont("assets/fonts/debugFont.ttf", 24);

	SDL_Surface* dText;

	SDL_Color color = { 255,255,255 };

	string sHours = std::to_string(moonY);
	const char* hours = sHours.c_str();

	const char* dt = hours;

	dText = TTF_RenderText_Solid(font, dt, color);

	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, dText);
	SDL_Rect* textRect = new SDL_Rect;
	textRect->x = 5;
	textRect->y = 15;
	textRect->w = dText->w;
	textRect->h = dText->h;

	SDL_RenderCopy(renderer, textTexture, NULL, textRect);
}