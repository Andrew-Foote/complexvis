#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "cdouble.h"

void error_fatal(const char *msg, SDL_Window *window) {
	static char full_msg[256];

	snprintf(full_msg, 255, "%s\n\nSDL error message: %s", msg, SDL_GetError());

	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", full_msg, window))
		fprintf(stderr, "FATAL ERROR: %s\n", full_msg);
	exit(EXIT_FAILURE);
}

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

SDL_Window *window;
void destroy_window(void) { SDL_DestroyWindow(window); }

SDL_Renderer *renderer;
void destroy_renderer(void) { SDL_DestroyRenderer(renderer); }

#define FONT_SIZE 16

TTF_Font *font;
void close_font(void) { TTF_CloseFont(font); }

SDL_Color text_color = { 0, 0, 0, SDL_ALPHA_OPAQUE };

SDL_Texture *load_text(const char* text) {
	SDL_Surface *surface;
	SDL_Texture *texture;

	surface = TTF_RenderText_Blended(font, text, text_color);
	if (!surface) return NULL;

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture) return NULL;

	SDL_FreeSurface(surface);

	return texture;
}

#define INFO_HEIGHT (FONT_SIZE + 2 * (FONT_SIZE / 5))
#define GRAPH_HEIGHT (WINDOW_HEIGHT - INFO_HEIGHT)
#define GRAPH_WIDTH GRAPH_HEIGHT
#define INFO_WIDTH GRAPH_WIDTH

Uint32 *graph_pixels;
void free_graph_pixels(void) { free(graph_pixels); }

struct cdouble cdouble_from_coords(i, j) {
	struct cdouble z;
	z.re = (double)(i - GRAPH_WIDTH / 2) / (GRAPH_WIDTH / 8);
	z.im = (double)(j - GRAPH_HEIGHT / 2) / (GRAPH_HEIGHT / 8);
	return z;
}

SDL_Color cdouble_to_color(struct cdouble z) {
	SDL_Color color;
	int normre = (int)((erf(z.re) + 1) * 128);
	int normim = (int)((erf(z.im) + 1) * 128);

	if (normre >= 256) normre = 255;
	if (normim >= 256) normim = 255;

	color.r = 0;
	color.g = normre;
	color.b = normim;
	return color;
}

void set_graph_pixel(int i, int j, SDL_Color color, SDL_PixelFormat* format) {
	graph_pixels[j * GRAPH_WIDTH + i] = SDL_MapRGBA(format, color.r, color.g, color.b, color.a);
}

typedef struct cdouble(*graphable_fn)(struct cdouble);
graphable_fn graph_fn = cdouble_exp;

void set_graph_pixels(void) {
	int i, j;
	struct cdouble w;
	struct cdouble z;
	SDL_Color color;
	Uint32 pixel = 0;
	SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);

	for (i = 0; i < GRAPH_WIDTH; ++i) {
		for (j = 0; j < GRAPH_HEIGHT; ++j) {
			w = cdouble_from_coords(i, j);
			z = (*graph_fn)(w);
			color = cdouble_to_color(z);
			set_graph_pixel(i, j, color, format);
		}
	}
}

SDL_Texture *info = NULL;

void show_info(int x, int y) {
	struct cdouble w, z;
	char *wstr, *zstr;
	static char text[64];
	SDL_Rect dst;

	w = cdouble_from_coords(x, y);
	z = (*graph_fn)(w);

	wstr = cdouble_str(w);
	zstr = cdouble_str(z);
	if (!(wstr && zstr)) error_fatal("Failed to allocate memory for the info text", window);

	snprintf(text, 63, "f(%s) = %s", wstr, zstr);
	free(wstr);
	free(zstr);

	dst.x = FONT_SIZE / 5;
	dst.y = GRAPH_HEIGHT;

	if (info) {
		if (SDL_QueryTexture(info, NULL, NULL, &dst.w, &dst.h))
			error_fatal("Failed to query info texture", window);
		if (SDL_RenderFillRect(renderer, &dst))
			error_fatal("Failed to clear info area", window);
		SDL_DestroyTexture(info);
	}

	info = load_text(text);
	if (!info) error_fatal("Failed to load text texture.", window);

	SDL_QueryTexture(info, NULL, NULL, &dst.w, &dst.h);

	if (SDL_RenderCopy(renderer, info, NULL, &dst))
		error_fatal("Failed to copy info text to renderer", window);

	SDL_RenderPresent(renderer);
}

SDL_Texture *graph;

void render(void) {
	SDL_Rect dst;

	dst.x = dst.y = 0;
	dst.w = GRAPH_WIDTH;
	dst.h = GRAPH_HEIGHT;

	if (SDL_RenderClear(renderer))
		error_fatal("Failed to clear renderer", window);

	if (SDL_RenderCopy(renderer, graph, NULL, &dst))
		error_fatal("Failed to copy graph texture to renderer", window);

	SDL_RenderPresent(renderer);
}

int main(int argc, char **argv) {
	SDL_Event event;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO))
		error_fatal("Failed to initialize SDL video subsystem.", NULL);
	atexit(SDL_Quit);

	if (TTF_Init())
		error_fatal("Failed to initialize SDL_ttf", NULL);
	atexit(TTF_Quit);

	font = TTF_OpenFont("verdana.ttf", FONT_SIZE);
	if (!font) error_fatal("Failed to open font.", window);
	atexit(close_font);

	window = SDL_CreateWindow(
		"Complex Function Visualizer",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_RESIZABLE
	);
	if (!window) error_fatal("Failed to create window.", NULL);
	atexit(destroy_window);

	SDL_SetWindowTitle(window, "Complex Function Visualizer");

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) error_fatal("Failed to create renderer.", window);
	atexit(destroy_renderer);

	if (SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT))
		error_fatal("Failed to set renderer logical size.", window);

	if (SDL_SetRenderDrawColor(renderer, 192, 192, 192, SDL_ALPHA_OPAQUE))
		error_fatal("Failed to set renderer drawing colour.", window);

	graph = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STATIC,
		GRAPH_WIDTH,
		GRAPH_HEIGHT
	);
	if (!graph) error_fatal("Failed to create graph texture.", window);

	graph_pixels = malloc(GRAPH_WIDTH * GRAPH_HEIGHT * sizeof *graph_pixels);
	if (!graph_pixels) error_fatal("Failed to allocate memory for graph.", window);
	atexit(free_graph_pixels);
	set_graph_pixels();

	if (SDL_UpdateTexture(graph, NULL, graph_pixels, GRAPH_WIDTH * sizeof(Uint32)))
		error_fatal("Failed to update graph texture.", window);

	render();
	
	for (;;) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				goto quit;
				break;
			case SDL_MOUSEMOTION:
				show_info(event.motion.x, event.motion.y);
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					render();
					break;
				}
			}
		}
	}

quit:
	return EXIT_SUCCESS;
}