#pragma once
#include "Tile.h"
#include "Biome.h"
#include "Entity.h"

#include <vector>

using std::vector;

class Chunk {
public:
	Chunk(int startX, int stopX) : startX(startX), stopX(stopX), chunkTiles(0), staticEntities(0), nonStaticEntities(0), chunkBiome() {}

	int startX;
	int stopX;

	vector<Tile> chunkTiles;
	vector<Entity> staticEntities;
	vector<Entity> nonStaticEntities;

	Biome chunkBiome;
};