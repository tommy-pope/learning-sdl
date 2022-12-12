#pragma once

#include "Entity.h"
#include <vector>

class Biome {

public:
	Biome(string biomeType, float amplitude, float freq, string tilePath) : biomeType(biomeType), amplitude(amplitude), freq(freq), tilePath(tilePath), staticBiomeEntityList(), nonStaticBiomeEntityList() {}
	Biome() : biomeType(""), amplitude(0.0f), freq(0.0f), tilePath() {}

	string biomeType;
	float amplitude;
	float freq;
	string tilePath;

	vector<Entity> staticBiomeEntityList;
	vector<Entity> nonStaticBiomeEntityList;
};