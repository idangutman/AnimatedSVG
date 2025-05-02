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

#include "ArduinoSVG.h"
#include "ball_bounce_svg.h"

#define TFT_WIDTH           240
#define TFT_HEIGHT          240
#define SVG_BUFFER_HEIGHT   16

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite buffer = TFT_eSprite(&tft);
unsigned char* svgBuffer;
ArduinoSVG* svg;
unsigned long startTime;

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
	int svgOptions = ARDUINO_SVG_OPTION_RGB565 | ARDUINO_SVG_OPTION_SWAP_BYTES;
	svg = new ArduinoSVG(ball_bounce_svg, svgBuffer, TFT_WIDTH, SVG_BUFFER_HEIGHT, svgOptions);

	// Load the SVG.
	if (!svg->load())
	{
		Serial.println("Failed loading SVG!");
		Serial.flush();
		while (true)
			sleep(1000);
	}

	// Store start time in milliseconds.
	startTime = millis();
}

void loop()
{
	// Get current time in milliseconds.
	unsigned long now = millis();
	unsigned long timeMs = now - startTime;

	// Update animation.
	svg->update(timeMs);

	// Clear the buffer and rasterize the image.
	float scale = 1.0f;
	buffer.fillScreen(TFT_BLUE);
	svg->rasterize((unsigned short*)buffer.getPointer(), TFT_WIDTH, TFT_HEIGHT, TFT_WIDTH * 2,
				   TFT_WIDTH*(1-scale)/2, TFT_HEIGHT*(1-scale)/2, scale);

	// Push the double-buffer to the screen.
	buffer.pushSprite(0, 0);

	// Show FPS.
	/*unsigned long time = millis();
	unsigned long diff = time - now;
	float fps = 1000 / (float)diff;
	tft.setTextColor(TFT_WHITE);
	tft.drawFloat(fps, 2, 90, 10, 2);
	tft.drawString("FPS", 130, 10, 2);*/
}
