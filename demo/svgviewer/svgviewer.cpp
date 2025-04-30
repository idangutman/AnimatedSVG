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

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "CmdLineParser.h"
#include "ArduinoSVG.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* surface = NULL;
SDL_Texture* texture = NULL;
SDL_Surface* transparentPatternSurface = NULL;

ArduinoSVG* svg = NULL;

const char* filePath = NULL;
int windowWidth = -1;
int windowHeight = -1;
int bufferWidth = -1;
int bufferHeight = -1;
int fixedBufferWidth = -1;
int fixedBufferHeight = -1;
unsigned char* rastBuffer;
bool largeBuffer = false;
bool showInfo = false;
bool printInfo = false;
int patternSize = 10;
int background = 0;
int scaleMultiplier = 0;
float scaleMultiplierPower = 1.25f;
float scale = 1;
bool zoomToWindow = true;
int panX = 0;
int panY = 0;

float loadTimeMs = 0;
float renderTimeMs = 0;

char* readFileContent(const char* filename);
bool parseArgs(int argc, const char** argv);
std::vector<std::string> getInfo();

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    // Set the default size for the window and buffer.
    windowWidth = 800;
    windowHeight = 600;

    if (!parseArgs(argc, (const char**)argv))
    {
        return SDL_APP_FAILURE;
    }

    bufferWidth = (fixedBufferWidth < 0) ? windowWidth : fixedBufferWidth;
    bufferHeight = (fixedBufferHeight < 0) ? windowHeight : fixedBufferHeight;

    char* svgContent = readFileContent(filePath);
    if (svgContent == NULL)
    {
        SDL_Log("Error reading input file: %s", filePath);
        return SDL_APP_FAILURE;
    }

    rastBuffer = (unsigned char*)malloc(bufferWidth * bufferHeight * 4);
    if (rastBuffer == NULL)
    {
        SDL_Log("Error allocating output buffer (%d bytes)", bufferWidth * bufferHeight * 4);
        return SDL_APP_FAILURE;
    }

    int options = ARDUINO_SVG_OPTION_BGRA8888;
    options |= largeBuffer ? ARDUINO_SVG_OPTION_LARGE_BUFFER : 0;
    svg = new ArduinoSVG(svgContent, rastBuffer, bufferWidth, bufferHeight, options);

    SDL_Time startTime = 0;
    SDL_GetCurrentTime(&startTime);

    if (!svg->load())
    {
        SDL_Log("Error loading SVG file: %s", filePath);
        return SDL_APP_FAILURE;
    }

    SDL_Time endTime = 0;
    SDL_GetCurrentTime(&endTime);
    loadTimeMs = (endTime - startTime) / 1000000.0f;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        SDL_Log("Error initializing SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("SVG Viewer", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
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

    surface = SDL_CreateSurface(windowWidth, windowHeight, SDL_PIXELFORMAT_ARGB8888);
    if (!surface)
    {
        SDL_Log("Error creating SDL surface: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    transparentPatternSurface = SDL_CreateSurface(patternSize * 2, patternSize * 2, SDL_PIXELFORMAT_ARGB8888);
    if (!transparentPatternSurface)
    {
        SDL_Log("Error creating SDL surface for transparent pattern: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    Uint32 transparentBackground = 0x27;
    Uint32 transparentForeground = 0x30;
    Uint32 bg = 0xFF000000 | transparentBackground << 16 | transparentBackground << 8 | transparentBackground;
    Uint32 fg = 0xFF000000 | transparentForeground << 16 | transparentForeground << 8 | transparentForeground;
    SDL_FillSurfaceRect(transparentPatternSurface, NULL, bg);
    SDL_Rect rect { 0, 0, patternSize, patternSize };
    SDL_FillSurfaceRect(transparentPatternSurface, &rect, fg);
    rect.x = patternSize;
    rect.y = patternSize;
    SDL_FillSurfaceRect(transparentPatternSurface, &rect, fg);

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
long overrideTimeMs = 0;
SDL_Time startTime = 0;
long timeMs = 0;
SDL_AppResult SDL_AppIterate(void *appstate)
{
    if (startTime == 0) SDL_GetCurrentTime(&startTime);

    SDL_Time time = 0;
    if (SDL_GetCurrentTime(&time))
    {
        timeMs = (time - startTime) / 1000000;
    }

    if (overrideTimeMs) {
        timeMs = overrideTimeMs;
    }
    //timeMs = 2999 - (timeMs % 3000);
    //timeMs = (timeMs % 1000) + 3000;
    //timeMs = timeMs % 1000;

    int w,h;
    SDL_GetWindowSize(window, &w, &h);
    if (w != windowWidth || h != windowHeight)
    {
        windowWidth = w;
        windowHeight = h;

        SDL_DestroySurface(surface);
        surface = SDL_CreateSurface(windowWidth, windowHeight, SDL_PIXELFORMAT_ARGB8888);
        if (!surface)
        {
            SDL_Log("Error creating SDL surface: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        if ((fixedBufferWidth < 0) || (fixedBufferHeight < 0))
        {
            bufferWidth = (fixedBufferWidth < 0) ? windowWidth : fixedBufferWidth;
            bufferHeight = (fixedBufferHeight < 0) ? windowHeight : fixedBufferHeight;

            rastBuffer = (unsigned char*)realloc(rastBuffer, bufferWidth * bufferHeight * 4);
            if (rastBuffer == NULL)
            {
                SDL_Log("Error allocating output buffer (%d bytes)", bufferWidth * bufferHeight * 4);
                return SDL_APP_FAILURE;
            }
            svg->setBuffer(rastBuffer, bufferWidth, bufferHeight);
        }

        changed = true;
    }

    // Animate the SVG (if it has an animation).
    if (svg->update(timeMs))
    {
        changed = true;
    }

    if (changed)
    {

        // Draw transparent blocks.
        SDL_BlitSurfaceTiled(transparentPatternSurface, NULL, surface, NULL);
        if (background != 0)
        {
            SDL_FillSurfaceRect(surface, NULL, background | 0xFF000000);
        }

        // Calculate scale.
        if (zoomToWindow)
        {
            float imageRatio = svg->width() / (float)svg->height();
            scale = (windowWidth / (float)windowHeight > imageRatio) ?
                    (windowHeight / (float)svg->height()) :
                    (windowWidth / (float)svg->width());
            scale *= SDL_powf(scaleMultiplierPower, scaleMultiplier);
        }
        else
        {
            scale = 1.0f;
            if (scaleMultiplier > 0)
            {
                if (scaleMultiplier < 13)
                {
                    scale += 0.25 * scaleMultiplier;
                }
                else
                {
                    scale += 0.5 * (scaleMultiplier - 6);
                }
            }
            else if (scaleMultiplier < 0)
            {
                if (scaleMultiplier > -10)
                {
                    scale -= scaleMultiplier * (-0.1);
                }
                else
                {
                    scale *= SDL_powf(0.5, -scaleMultiplier - 6);
                }
            }
        }

        SDL_LockSurface(surface);

        SDL_Time rastStartTime, rastEndTime;
        SDL_GetCurrentTime(&rastStartTime);

        svg->rasterize((unsigned short*)surface->pixels, windowWidth, windowHeight, windowWidth * 4,
                       panX + windowWidth/2 - svg->width() * scale/2,
                       panY + windowHeight/2 - svg->height() * scale/2, scale);

        SDL_GetCurrentTime(&rastEndTime);

        renderTimeMs = (rastEndTime - rastStartTime) / 1000000.0f;

        SDL_UnlockSurface(surface);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_DestroyTexture(texture);

        if (showInfo)
        {
            std::vector<std::string> info = getInfo();

            int maxLineLen = 0;
            for (int i = 0; i < info.size(); i++)
            {
                maxLineLen = (maxLineLen > info[i].length() ? maxLineLen : info[i].length());
            }

            SDL_FRect rect = { 10, 10, 8.0f * (maxLineLen + 1), 10.0f * (info.size() + 2) };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 96);
            SDL_RenderFillRect(renderer, &rect);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (int i = 0; i < info.size(); i++)
            {
                SDL_RenderDebugText(renderer, rect.x + 10, rect.y + (i + 1) * 10, info[i].c_str());
            }
        }

        SDL_RenderPresent(renderer);
    }

    if (printInfo)
    {
        std::vector<std::string> info = getInfo();
        SDL_Log("");
        for (int i = 0; i < info.size(); i++)
        {
            if (*info[i].c_str())
            {
                SDL_Log("%s", info[i].c_str());
            }
        }
        printInfo = false;
    }
    //SDL_Delay(10);

    changed = false;

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
        else if (event->key.key == SDLK_SPACE)
        {
            changed = true;
        }
        else if (event->key.key == SDLK_I)
        {
            showInfo = !showInfo;
            changed = true;
        }
        else if (event->key.key == SDLK_Z)
        {
            zoomToWindow = !zoomToWindow;
            changed = true;
        }
        else if (event->key.key == SDLK_P)
        {
            overrideTimeMs = (overrideTimeMs != 0) ? 0 : timeMs;
            changed = true;
        }
        else if (event->key.key == SDLK_RIGHTBRACKET)
        {
            overrideTimeMs++;
            changed = true;
        }
        else if (event->key.key == SDLK_LEFTBRACKET)
        {
            overrideTimeMs--;
            changed = true;
        }
        else if ((event->key.key == SDLK_P) && (event->key.mod & SDL_KMOD_CTRL))
        {
            printInfo = true;
        }
        else if ((event->key.key == SDLK_R) && (event->key.mod & SDL_KMOD_CTRL))
        {
            // Reset the start time.
            SDL_GetCurrentTime(&startTime);
        }
        else if ((SDL_GetKeyFromScancode(event->key.scancode, event->key.mod, false) == SDLK_PLUS) || (event->key.key == SDLK_KP_PLUS))
        {
            scaleMultiplier++;
            changed = true;
        }
        else if ((SDL_GetKeyFromScancode(event->key.scancode, event->key.mod, false) == SDLK_MINUS) || (event->key.key == SDLK_KP_MINUS))
        {
            scaleMultiplier--;
            changed = true;
        }
        else if ((event->key.key == SDLK_UP) || ((event->key.key == SDLK_KP_8) && !(event->key.mod & SDL_KMOD_NUM)))
        {
            panY -= 10 * SDL_max(1, SDL_powf(scaleMultiplierPower, scaleMultiplier));
            changed = true;
        }
        else if ((event->key.key == SDLK_DOWN) || ((event->key.key == SDLK_KP_2) && !(event->key.mod & SDL_KMOD_NUM)))
        {
            panY += 10 * SDL_max(1, SDL_powf(scaleMultiplierPower, scaleMultiplier));
            changed = true;
        }
        else if ((event->key.key == SDLK_LEFT) || ((event->key.key == SDLK_KP_4) && !(event->key.mod & SDL_KMOD_NUM)))
        {
            panX -= 10 * SDL_max(1, SDL_powf(scaleMultiplierPower, scaleMultiplier));
            changed = true;
        }
        else if ((event->key.key == SDLK_RIGHT) || ((event->key.key == SDLK_KP_6) && !(event->key.mod & SDL_KMOD_NUM)))
        {
            panX += 10 * SDL_max(1, SDL_powf(scaleMultiplierPower, scaleMultiplier));
            changed = true;
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        if (event->wheel.y != 0)
        {
            scaleMultiplier += event->wheel.y;
            changed = true;
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        if (event->motion.state & SDL_BUTTON_LEFT)
        {
            panX += event->motion.xrel;
            panY += event->motion.yrel;
            changed = true;
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

bool parseArgs(int argc, const char** argv)
{
    CmdLineParser parser;
    bool syntax = false;

    parser.AddArgument("file path", "Path of the SVG file to be viewed", &filePath);
    parser.AddIntOption("ww", "window-width", "window width", "Set the window width", &windowWidth);
    parser.AddIntOption("wh", "window-height", "window height", "Set the window height", &windowHeight);
    parser.AddIntOption("bw", "buffer-width", "buffer width", "Set the buffer width", &fixedBufferWidth);
    parser.AddIntOption("bh", "buffer-height", "buffer height", "Set the buffer height", &fixedBufferHeight);
    parser.AddFlagOption("lb", "large-buffer", "large buffer", "Use large buffer rasterization (rasterize in single run)", &largeBuffer);
    parser.AddColorOption("bg", "background", "background", "Change background color (e.g. #0000FF)", &background);
    parser.AddBoolOption("z", "zoom", "zoom", "Enable/disable zoom to window", &zoomToWindow);
    parser.AddFlagOption("i", "show-info", "show info", "Show information on the rendered image", &showInfo);
    parser.AddFlagOption("p", "print-info", "print info", "Print information on the rendered image to the console", &printInfo);
    parser.AddFlagOption("h", "help", "help", "Show this help", &syntax);

    bool success = parser.Parse(argc, argv);

    if (!success && parser.GetLastError() != NULL)
    {
        SDL_Log("%s", parser.GetLastError());
        SDL_Log(" ");
    }
    if (!success || syntax)
    {
        const char* exe = argv[0];
        for (const char* ptr = exe; *ptr != '\0'; ptr++)
        {
            if (*ptr == '/' || *ptr == '\\')
            {
                exe = ptr + 1;
            }
        }

        SDL_Log("Syntax: %s %s", exe, parser.GetSyntax());

        return false;
    }

    return true;
}

std::vector<std::string> getInfo()
{
    std::vector<std::string> info;
    char buf[1024];

    sprintf(buf, "Window width:            %d", windowWidth);
    info.push_back(buf);
    sprintf(buf, "Window height:           %d", windowHeight);
    info.push_back(buf);
    sprintf(buf, "Buffer width:            %d", bufferWidth);
    info.push_back(buf);
    sprintf(buf, "Buffer height:           %d", bufferHeight);
    info.push_back(buf);
    sprintf(buf, "Large buffers mode:      %s", largeBuffer ? "true" : "false");
    info.push_back(buf);
    sprintf(buf, "Zoom to window:          %s", zoomToWindow ? "true" : "false");
    info.push_back(buf);
    sprintf(buf, "Effective scale:         %.1f%%", scale * 100);
    info.push_back(buf);
    sprintf(buf, "Pan X:                   %d", panX);
    info.push_back(buf);
    sprintf(buf, "Pan Y:                   %d", panY);
    info.push_back(buf);
    info.push_back("");

    int bufferSize = bufferWidth * bufferHeight * 4;
    if (bufferSize > 1024)
    {
        float readableSize = bufferSize < 1024*1024 ? bufferSize/1024.0 : bufferSize/(1024.0f*1024.0f);
        const char* readableUnit = bufferSize < 1024*1024 ? "KB" : "MB";
        sprintf(buf, "Buffer memory:           %d bytes (%.2f%s)\n", bufferSize, readableSize, readableUnit);
    }
    else
    {
        sprintf(buf, "Buffer memory:           %d bytes\n", bufferSize);
    }
    info.push_back(buf);

    int memSize = svg->getImageUsedMemory();
    if (memSize > 1024)
    {
        float readableSize = memSize < 1024*1024 ? memSize/1024.0 : memSize/(1024.0f*1024.0f);
        const char* readableUnit = memSize < 1024*1024 ? "KB" : "MB";
        sprintf(buf, "Image memory used:       %d bytes (%.2f%s)\n", memSize, readableSize, readableUnit);
    }
    else
    {
        sprintf(buf, "Image memory used:       %d bytes\n", bufferSize);
    }
    info.push_back(buf);
    memSize = svg->getRasterizerUsedMemory();
    if (memSize > 1024)
    {
        float readableSize = memSize < 1024*1024 ? memSize/1024.0 : memSize/(1024.0f*1024.0f);
        const char* readableUnit = memSize < 1024*1024 ? "KB" : "MB";
        sprintf(buf, "Rasterizer memory used:  %d bytes (%.2f%s)\n", memSize, readableSize, readableUnit);
    }
    else
    {
        sprintf(buf, "Rasterizer memory used:  %d bytes\n", bufferSize);
    }
    info.push_back(buf);
    info.push_back("");

    sprintf(buf, "Load time:               %.2fms\n", loadTimeMs);
    info.push_back(buf);
    sprintf(buf, "Render time:             %.2fms\n", renderTimeMs);
    info.push_back(buf);
    //info.push_back("");

    sprintf(buf, "Animation timestamp:     %ldms\n", timeMs);
    info.push_back(buf);

    return info;
}
