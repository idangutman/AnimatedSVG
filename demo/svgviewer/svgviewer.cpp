/*
 * Copyright (c) 2025 Idan Gutman
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * This demo project contains a viewer for SVG files that uses ArduinoSVG.
 * It utilizes SDL3 for graphics (https://github.com/libsdl-org/SDL).
 */

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdlib.h>
#include <stdio.h>

#include "ArduinoSVG.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* surface = NULL;
SDL_Texture* texture = NULL;

ArduinoSVG* svg = NULL;

int width, height;
const char* fileName = NULL;
unsigned char* rastBuffer;

char* readFileContent(const char* filename);
bool parseArgs(int argc, char* argv[]);

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    // Set the default size.
    width = 800;
    height = 600;

    if (!parseArgs(argc, argv))
    {
        return SDL_APP_FAILURE;
    }

    char* svgContent = readFileContent(fileName);
    if (svgContent == NULL)
    {
        SDL_Log("Error reading input file: %s", fileName);
        return SDL_APP_FAILURE;
    }

    rastBuffer = (unsigned char*)malloc(width * height * 4);
    if (rastBuffer == NULL)
    {
        SDL_Log("Error allocating output buffer (%d bytes)", width * height * 4);
        return SDL_APP_FAILURE;
    }

    int options = ARDUINO_SVG_OPTION_BGRA8888 | ARDUINO_SVG_OPTION_ANTIALIASING;
    svg = new ArduinoSVG(svgContent, rastBuffer, width, height, options);
    if (!svg->load())
    {
        SDL_Log("Error loading SVG file: %s", fileName);
        return SDL_APP_FAILURE;
    }

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        SDL_Log("Error initializing SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("My title", width, height, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        SDL_Log("Error creating SDL window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        SDL_Log("Error creating SDL renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
    if (!surface)
    {
        SDL_Log("Error creating SDL surface: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    delete svg;
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    SDL_DestroySurface(surface);
    surface = NULL;
    SDL_DestroyWindow(window);
    window = NULL;

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

bool changed = true;
SDL_AppResult SDL_AppIterate(void *appstate)
{
    if (!changed)
    {
        return SDL_APP_CONTINUE;
    }

    int w,h;
    SDL_GetWindowSize(window, &w, &h);
    if (w != width || h != height)
    {
        width = w;
        height = h;
        SDL_DestroySurface(surface);
        surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
        if (!surface)
        {
            SDL_Log("Error creating SDL surface: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        rastBuffer = (unsigned char*)realloc(rastBuffer, w * h * 4);
        svg->setBuffer(rastBuffer, w, h);
    }

    // Calculate scale.
    float imageRatio = svg->width()/(float)svg->height();
    float scale = (width/(float)height > imageRatio) ? (height / (float)svg->height()) : width / (float)svg->width();

    SDL_LockSurface(surface);

    SDL_FillSurfaceRect(surface, NULL, 0xFF0000FF);

    SDL_Time rastStartTime, rastEndTime;
    SDL_GetCurrentTime(&rastStartTime);

    svg->rasterize((unsigned short*)surface->pixels, width, height, width * 4, width/2-svg->width()*scale/2, height/2-svg->height()*scale/2, scale);

    SDL_GetCurrentTime(&rastEndTime);

    SDL_UnlockSurface(surface);

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN)
    {
        if (event->key.key == SDLK_ESCAPE)
        {
            return SDL_APP_SUCCESS;
        }
    }

    return SDL_APP_CONTINUE;
}

char* readFileContent(const char* filename)
{
	FILE* fp = NULL;
	size_t size;
	char* data = NULL;

	fp = fopen(filename, "rb");
	if (!fp) goto error;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)malloc(size+1);
	if (data == NULL) goto error;
	if (fread(data, 1, size, fp) != size) goto error;
	data[size] = '\0';
	fclose(fp);

	return data;

error:
	if (fp) fclose(fp);
	if (data) free(data);
	return NULL;
}

bool parseArgs(int argc, char* argv[])
{
    int valuesCount = 0;
    bool success = true;

    for (int i = 0; success && i < argc; i++)
    {
        // Check if this is a value or flag.
        if (*argv[i] != '-')
        {
            if (valuesCount > 0)
            {
                SDL_Log("Too many arguments");
                success = false;
            }
            fileName = argv[i];
        }
        else
        {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
                success = false;
            }
            else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0)
            {
                i++;
                if (i < argc)
                {
                    width = atoi(argv[i]);
                    if (width == 0)
                    {
                        SDL_Log("Invalid width: %s", argv[i]);
                        success = false;
                    }
                }
            }
            else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--height") == 0)
            {
                i++;
                if (i < argc)
                {
                    height = atoi(argv[i]);
                    if (height == 0)
                    {
                        SDL_Log("Invalid height: %s", argv[i]);
                        success = false;
                    }
                }
            }
        }
    }

    // Make sure input was provided.
    if (fileName == NULL)
    {
        SDL_Log("Missing input file argument");
    }

    if (!success)
    {
        const char* exe = argv[0];
        for (const char* ptr = exe; *ptr != '\0'; ptr++)
        {
            if (*ptr == '/' || *ptr == '\\')
            {
                exe = ptr + 1;
            }
        }

        SDL_Log("Syntax: %s [options] <svg-file>", exe);
        SDL_Log("    -w, --width    window width");
        SDL_Log("    -g, --height   window height");
        SDL_Log("    -h, --help     show this help");
    }

    return success;
}
