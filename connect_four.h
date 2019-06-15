#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <cmath>
#include <cstdio>
#include <vector>
#include <string>
#include <numeric>
using namespace std;

SDL_Window *gWindow = nullptr;
SDL_Renderer *gRenderer = nullptr;
constexpr int GRID_SIZE = 50;
constexpr int SPACE = 0;
constexpr int RED_PLAYER = 1;
constexpr int BLACK_PLAYER = 2;
constexpr int STATUS_LINE_SIZE = 20;

class ConnectFour {
public:
	ConnectFour() = default;

	ConnectFour(int nbWGrids, int nbHGrids) :
		_nbWGrids(nbWGrids),
		_nbHGrids(nbHGrids),
		_nbGrids(nbWGrids * nbHGrids),
		_contents(nbWGrids * nbHGrids, SPACE)
	{
		_winHeight = GRID_SIZE * nbHGrids;
		_winWidth = GRID_SIZE * nbWGrids;
	}

	~ConnectFour() {
		post_run();
	}

	void start() {
		pre_run();
		run();
	}
private:
	int _nbHGrids = 0;
	int _nbWGrids = 0;
	int _nbGrids = 0;
	int _winHeight = 0;
	int _winWidth = 0;
	bool _running = false;
	bool _connected = false;

	int _player = RED_PLAYER;
	int _mousePos = -1;
	int _lastPos = -1;
	int _startPos = -1;
	int _endPos = -1;
	vector<int> _contents;

	Uint32 _startTime = 0;

	SDL_Texture *_grayCircle = nullptr;
	SDL_Texture *_redCircle = nullptr;
	SDL_Texture *_blackCircle = nullptr;
	SDL_Surface *_icon = nullptr;

	TTF_Font *_font = nullptr;

	void pre_run() {
		gWindow = SDL_CreateWindow("Connect Four", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, _winWidth, _winHeight + STATUS_LINE_SIZE, SDL_WINDOW_SHOWN);
		gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);

		_grayCircle = loadFromFile("images\\gray.png");
		_redCircle = loadFromFile("images\\red.png");
		_blackCircle = loadFromFile("images\\blue.png");
		_icon = IMG_Load("images\\icon.png");
		SDL_SetWindowIcon(gWindow, _icon);

		_font = TTF_OpenFont("C:\\Windows\\Fonts\\cambria.ttc", STATUS_LINE_SIZE);
	}

	void post_run() {
		SDL_FreeSurface(_icon);
		SDL_DestroyTexture(_grayCircle);
		SDL_DestroyTexture(_redCircle);
		SDL_DestroyTexture(_blackCircle);
		TTF_CloseFont(_font);

		SDL_DestroyRenderer(gRenderer);
		SDL_DestroyWindow(gWindow);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
	}

	SDL_Texture *loadFromFile(string path) {
		SDL_Texture *texture = nullptr;
		SDL_Surface *surface = IMG_Load(path.c_str());
		if (surface) {
			texture = SDL_CreateTextureFromSurface(gRenderer, surface);
			SDL_FreeSurface(surface);
		}
		else {
			printf("Failed to load `%s`! IMG Error: %s\n", path.c_str(), IMG_GetError());
		}
		return texture;
	}

	void run() {
		_running = true;
		_startTime = SDL_GetTicks();
		Uint32 tic, elapsed;
		while (_running) {
			tic = SDL_GetTicks();
			handleEvent();
			clearScreen();
			checkCollision();
			renderScreen();
			SDL_RenderPresent(gRenderer);
			elapsed = SDL_GetTicks() - tic;
			if (elapsed < 30)
				SDL_Delay(30 - elapsed);
		}
	}

	void checkCollision() {
		if (_lastPos != -1) {
			checkFromPos(_lastPos);
		}
	}

	void checkFromPos(int pos) {
		int tx = pos % _nbWGrids, ty = pos / _nbWGrids;
		int i, j, x, y;

		// check horizontally
		y = ty;
		for (x = pos % _nbWGrids; x < pos % _nbWGrids + 4; x++) {
			if (x >= _nbWGrids || _contents[x + y * _nbWGrids] != _contents[pos])
				break;
		}
		i = (x - 1) + y * _nbWGrids;

		for (x = pos % _nbWGrids; x > pos % _nbWGrids - 4; x--) {
			if (x < 0 || _contents[x + y * _nbWGrids] != _contents[pos])
				break;
		}
		j = (x + 1) + y * _nbWGrids;
		if (i - j >= 3) {
			_connected = true;
			_startPos = i;
			_endPos = _startPos - 3;
			return;
		}

		// check vertically
		x = tx;
		for (y = pos / _nbWGrids; y < pos / _nbWGrids + 4; y++) {
			if (y >= _nbHGrids || _contents[x + y * _nbWGrids] != _contents[pos])
				break;
		}
		i = x + (y - 1) * _nbWGrids;

		for (y = pos / _nbWGrids; y > pos / _nbWGrids - 4; y--) {
			if (y < 0 || _contents[x + y * _nbWGrids] != _contents[pos])
				break;
		}
		j = x + (y + 1) * _nbWGrids;

		if (i - j >= 3 * _nbWGrids) {
			_connected = true;
			_startPos = i;
			_endPos = i - 3 * _nbWGrids;
			return;
		}

		// check diagonally
		// case 1: top left -> bottom right
		for (x = pos % _nbWGrids, y = pos / _nbWGrids; x < pos % _nbWGrids + 4;  x++, y++) {
			if (x >= _nbWGrids || y >= _nbHGrids || _contents[x + y * _nbWGrids] != _contents[pos])
				break;
		}
		i = (x - 1) + (y - 1) * _nbWGrids;

		for (x = pos % _nbWGrids, y = pos / _nbWGrids; x > pos % _nbWGrids - 4;  x--, y--) {
			if (x < 0 || y < 0 || _contents[x + y * _nbWGrids] != _contents[pos])
				break;
		}
		j = (x + 1) + (y + 1) * _nbWGrids;

		if (i - j >= 3 * (_nbWGrids + 1)) {
			_connected = true;
			_startPos = i;
			_endPos = _startPos - 3 * (_nbWGrids + 1);
			return;
		}

		// case 2: top right -> bottom left
		for (x = pos % _nbWGrids, y = pos / _nbWGrids; x > pos % _nbWGrids - 4 ;x--, y++) {
			if (x < 0 || y >= _nbHGrids || _contents[x+y*_nbWGrids] != _contents[pos])
				break;
		}
		i = (x + 1) + (y - 1) * _nbWGrids;

		for (x = pos % _nbWGrids, y = pos / _nbWGrids; x < pos % _nbWGrids + 4; x++, y--) {
			if (x >= _nbWGrids || y < 0 || _contents[x + y * _nbWGrids] != _contents[pos])
				break;
		}
		j = (x - 1) + (y + 1) * _nbWGrids;

		if (i - j >= 3 * (_nbWGrids - 1)) {
			_connected = true;
			_startPos = i;
			_endPos = _startPos - 3 * (_nbWGrids - 1);
			return;
		}
	}

	void handleEvent() {
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				_running = false;
			if(!_connected)
				handleMouseEvent(e);
		}
	}

	void handleMouseEvent(SDL_Event &e) {
		_mousePos = -1;
		if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONUP) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			if (x < 0 || y < 0 || x >= _winWidth || y >= _winHeight)
				return;
			int gx = x / GRID_SIZE, gy = y / GRID_SIZE;
			_mousePos = gx + gy * _nbWGrids;
			if (e.type == SDL_MOUSEBUTTONUP) {
				if (_contents[_mousePos] == SPACE) {
					_contents[_mousePos] = _player;
					_lastPos = _mousePos;
					switchPlayer();
				}
			}
		}
	}

	void switchPlayer(){
		_player = _player == BLACK_PLAYER ? RED_PLAYER : BLACK_PLAYER;
	}

	void clearScreen() {
		SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0xff, 0xff);
		SDL_RenderClear(gRenderer);
	}

	void renderScreen() {
		renderPieces();
		renderPiecePreview();
		renderConnectLine();
		renderStatusBar();
	}

	void renderPieces() {
		for (int i = 0; i < _nbGrids; i++) {
			int gx = i % _nbWGrids, gy = i / _nbWGrids;
			SDL_Rect rect = { gx * GRID_SIZE, gy * GRID_SIZE, GRID_SIZE, GRID_SIZE };
			if (_contents[i] == SPACE) {
				SDL_RenderCopy(gRenderer, _grayCircle, nullptr, &rect);
			}
			else {
				SDL_Texture *target = _contents[i] == BLACK_PLAYER ? _blackCircle : _redCircle;
				SDL_RenderCopy(gRenderer, target, nullptr, &rect);
			}
		}
	}

	void renderPiecePreview() {
		if (_mousePos != -1 && _contents[_mousePos] == SPACE) {
			SDL_Texture *target = _player == BLACK_PLAYER ? _blackCircle : _redCircle;
			int gx = _mousePos % _nbWGrids, gy = _mousePos / _nbWGrids;
			SDL_Rect rect = { gx * GRID_SIZE, gy * GRID_SIZE, GRID_SIZE, GRID_SIZE };
			SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0xff, 0xff);
			SDL_RenderFillRect(gRenderer, &rect);
			int s = 5;
			rect = { gx * GRID_SIZE + s, gy * GRID_SIZE + s, GRID_SIZE - 2 * s, GRID_SIZE - 2 * s };
			SDL_RenderCopy(gRenderer, target, nullptr, &rect);
		}
	}

	void renderConnectLine() {
		if (_connected) {
			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xff);
			int x1 = static_cast<int>((_startPos % _nbWGrids) * GRID_SIZE + 0.5 * GRID_SIZE);
			int y1 = static_cast<int>((_startPos / _nbWGrids) * GRID_SIZE + 0.5 * GRID_SIZE);
			int x2 = static_cast<int>((_endPos % _nbWGrids) * GRID_SIZE + 0.5 * GRID_SIZE);
			int y2 = static_cast<int>((_endPos / _nbWGrids) * GRID_SIZE + 0.5 * GRID_SIZE);
			drawLine(x1, y1, x2, y2, 10);
		}
	}

	void renderStatusBar() {
		char s[64];
		auto count_zero = [](int a, int b) {
			return a + (b != 0);
		};
		int occupied = accumulate(_contents.cbegin(), _contents.cend(), 0, count_zero);
		Uint32 timeElapsed = (SDL_GetTicks() - _startTime) / 1000;
		sprintf_s(s, "Pieces %d / %d %02d:%02d:%02d is %sconnected", occupied, _contents.size(),
			timeElapsed / 3600, timeElapsed / 60, timeElapsed % 60, _connected ? "" : "not ");
		SDL_Surface *text = TTF_RenderText_Solid(_font, s, { 0, 0, 0 });
		SDL_Texture *texture = SDL_CreateTextureFromSurface(gRenderer, text);
		SDL_Rect rect = { 5, _winHeight, text->w, text->h };
		SDL_FreeSurface(text);
		SDL_RenderCopy(gRenderer, texture, nullptr, &rect);
	}

	void drawLine(int x1, int y1, int x2, int y2, int thickness=1) {
		if(y1 == y2) {
			for (int i = 0; i < thickness; i++) {
				int dx = 0;
				int dy = i - thickness / 2;
				SDL_RenderDrawLine(gRenderer, x1 + dx, y1 + dy, x2 + dx, y2 + dy);
			}
		}
		else {
			double k = -(x1 - x2) / (y1 - y2);
			double theta = atan(k);
			for (int i = 0; i < thickness; i++) {
				int dx = static_cast<int>(cos(theta) * (i - thickness / 2));
				int dy = static_cast<int>(sin(theta) * (i - thickness / 2));
				SDL_RenderDrawLine(gRenderer, x1 + dx, y1 + dy, x2 + dx, y2 + dy);
			}
		}
	}
};