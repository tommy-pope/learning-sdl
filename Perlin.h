#pragma once
#include <cstdlib>
#include <time.h>

// https://gpfault.net/posts/perlin-noise.txt.html
class Perlin {
private:
	int arr[256];
	int arrSize = 256;

public:
	Perlin () : arr(), arrSize(256) {}

	void init() {
		srand(time(NULL));

		for (int i = 0; i < arrSize; i++) {
			arr[i] = 1 + (rand() % 100);
		}
	}

	float fade(float t) {
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}
	
	float grad(int p) {
		int v = arr[p & 0xff];
		return v >= 50 ? 1.0 : -1.0;
	}

	float noise(float p) {
		float p0 = floor(p);
		float p1 = p0 + 1.0;

		float t = p - p0;
		float fade_t = fade(t);

		float g0 = grad(p0);
		float g1 = grad(p1);

		return (1.0 - fade_t) * g0 * (p - p0) + fade_t * g1 * (p - p1);
	}
};