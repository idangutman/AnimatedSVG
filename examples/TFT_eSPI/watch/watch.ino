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
 * This example displays an animated SVG on TFT_eSPI.
 */

#include <TFT_eSPI.h>

#include "AnimatedSVG.h"
#include "watch_svg.h"
#include "animatedsvg_svg.h"

#define TFT_WIDTH           240
#define TFT_HEIGHT          240
#define SVG_BUFFER_HEIGHT   16

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite buffer = TFT_eSprite(&tft);
unsigned char* svgBuffer;
AnimatedSVG* splash;
AnimatedSVG* watch;
unsigned long startTime;

struct ScriptTime
{
	unsigned long durationMsec;
	long addMsec;
	unsigned long moduloMsec;
};
struct ScriptTime script[] = 
{
	// Dur            Add                 Mod
	{ 5000,            60 * 1000,              0 },
	{ 5000,       59 * 60 * 1000,      60 * 1000 },
	{ 5000,  11 * 60 * 60 * 1000, 60 * 60 * 1000 },
	{    0,                    0,              0 }
};

void renderSVG(AnimatedSVG* svg, unsigned long timeMs);

void setup()
{
	Serial.begin(115200);

	// Initialize TFT_eSPI package.
	tft.begin();
	tft.setRotation(0);
	tft.fillScreen(TFT_BLACK);

	// Create screen buffer, so image is not renderded to screen in parts.
	buffer.createSprite(TFT_WIDTH, TFT_HEIGHT);
	buffer.fillScreen(TFT_BLACK);

	// Create buffer for SVG (must be BGRA).
	svgBuffer = (unsigned char*)malloc(TFT_WIDTH * SVG_BUFFER_HEIGHT * 4);

	// Create the SVG.
	int svgOptions = ANIMATED_SVG_OPTION_RGB565 | ANIMATED_SVG_OPTION_SWAP_BYTES;
	splash = new AnimatedSVG(animatedsvg_svg, svgBuffer, TFT_WIDTH, SVG_BUFFER_HEIGHT, svgOptions);
	watch = new AnimatedSVG(watch_svg, svgBuffer, TFT_WIDTH, SVG_BUFFER_HEIGHT, svgOptions);

	// Load the SVG.
	if (!splash->load())
	{
		Serial.println("Failed loading splash SVG!");
		Serial.flush();
		while (true)
			sleep(1000);
	}
	if (!watch->load())
	{
		Serial.println("Failed loading watch SVG!");
		Serial.flush();
		while (true)
			sleep(1000);
	}

	// Show splash-screen.
	startTime = millis();
	unsigned long timeMs = (millis() - startTime);
	while (timeMs < 5 * 1000)
	{
		timeMs = (millis() - startTime);
		renderSVG(splash, timeMs);
		yield();
	}

	startTime = millis();
}

void loop()
{
	const unsigned long watchInitTime = ((10 * 60) + 10) * 60 * 1000;

	// Run clock for 10 seconds.
	unsigned long duration;
	unsigned long watchEndTime = millis() + 10 * 1000;
	for (duration = millis() - startTime; duration < watchEndTime; duration = millis() - startTime)
	{
		renderSVG(watch, watchInitTime + duration);
		delay(10);
	}

	// Run demo.
	unsigned long sumAddMsec = 0;
	for (int i = 0; script[i].durationMsec > 0; i++ )
	{
		unsigned long demoStartTime = millis();
		for (unsigned long demoDur = 0; demoDur < script[i].durationMsec; demoDur = millis() - demoStartTime)
		{
		duration = millis() - startTime;
		unsigned long add = (demoDur / (float)script[i].durationMsec) * script[i].addMsec;
		if (script[i].moduloMsec > 0)
		{
			add -= add % script[i].moduloMsec;
		}
		renderSVG(watch, watchInitTime + duration + sumAddMsec + add);
		yield();
		}
		sumAddMsec += script[i].addMsec;
	}
}

void renderSVG(AnimatedSVG* svg, unsigned long timeMs)
{
	// Update animation.
	svg->update(timeMs);

	// Clear the buffer and rasterize the image.
	float scale = 1.0f;
	buffer.fillScreen(TFT_BLACK);
	svg->rasterize((unsigned short*)buffer.getPointer(), TFT_WIDTH, TFT_HEIGHT, TFT_WIDTH * 2,
		TFT_WIDTH*(1-scale)/2, TFT_HEIGHT*(1-scale)/2, scale);

	// Push the double-buffer to the screen.
	buffer.pushSprite(0, 0);
}
