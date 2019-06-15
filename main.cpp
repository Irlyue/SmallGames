#include "connect_four.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
using namespace std;

bool initialize() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)){
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}
	if (TTF_Init() == -1) {
		return false;
	}
	return true;
}

int main(int, char *[]) {
	if (initialize()) {
		ConnectFour cf(10, 8);
		cf.start();
	}
	return 0;
}
