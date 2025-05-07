# AnimatedSVG

![animated gif of SVG animation running on screen and on ESP32 display](/samples/AnimatedSVG.gif)

AnimatedSVG is an implementation of a simple SVG parser and rasterizer (NanoSVG) that supports simple animations.  
This fork of NanoSVG has reduced memory usage for parsing and supports rasterization in parts for small-memory usages (e.g. Arduino).

|SVG image (source*)|Rasterized (left* - viewer on Windows, right - Arduino example on ESP32)|
|-|-|
|<div style='background:blue;'>![SVG animation](/samples/ball_bounce.svg)</div>|![animated gif of SVG animation running on screen and on ESP32 display](/samples/bouncing_ball_esp32.gif)|

Above example (right) is running on ESP32-S3 board with TFT_eSPI library. It uses 15KB for buffer and 33KB for AnimatedSVG.  
> *Note: Blue background is not part of source image, added to compare with Arduino example.

## Example Usage

``` CPP
// Create buffer for SVG (must be BGRA).
svgBuffer = (unsigned char*)malloc(TFT_WIDTH * SVG_BUFFER_HEIGHT * 4);

// Create the SVG.
int svgOptions = ANIMATED_SVG_OPTION_RGB565 | ANIMATED_SVG_OPTION_SWAP_BYTES;
AnimatedSVG(ball_bounce_svg, svgBuffer, TFT_WIDTH, SVG_BUFFER_HEIGHT, svgOptions);

// Load the SVG.
if (!svg->load())
{
	Serial.println("Failed loading SVG!");
	return;
}

// Clear the buffer and rasterize the image.
float scale = 1.0f;
svg->rasterize((unsigned short*)buffer.getPointer(), TFT_WIDTH, TFT_HEIGHT, TFT_WIDTH * 2,
				TFT_WIDTH*(1-scale)/2, TFT_HEIGHT*(1-scale)/2, scale);
```

# Nano SVG

## Parser

![screenshot of some splines rendered with the sample program](/samples/svg_splines.png?raw=true)

NanoSVG is a simple stupid single-header-file SVG parse. The output of the parser is a list of cubic bezier shapes.

The library suits well for anything from rendering scalable icons in your editor application to prototyping a game.

NanoSVG supports a wide range of SVG features, but something may be missing, feel free to create a pull request!

The shapes in the SVG images are transformed by the viewBox and converted to specified units.
That is, you should get the same looking data as your designed in your favorite app.

NanoSVG can return the paths in few different units. For example if you want to render an image, you may choose
to get the paths in pixels, or if you are feeding the data into a CNC-cutter, you may want to use millimeters. 

The units passed to NanoSVG should be one of: 'px', 'pt', 'pc' 'mm', 'cm', or 'in'.
DPI (dots-per-inch) controls how the unit conversion is done.

If you don't know or care about the units stuff, "px" and 96 should get you going.

## Rasterizer

|SVG image (source)|Rasterized image (PNG)|
|-|-|
|![tiger.svg](/samples/tiger.svg?raw=true)|![screenshot of tiger.svg rendered with NanoSVG rasterizer](/samples/tiger.png?raw=true)|

The parser library is accompanied with really simpler SVG rasterizer. Currently it only renders flat filled shapes.

The intended usage for the rasterizer is to for example bake icons of different size into a texture. The rasterizer is not particular fast or accurate, but it's small and packed in one header file.


## Example Usage

``` C
// Load
struct NSVGimage* image;
image = nsvgParseFromFile("test.svg", "px", 96);
printf("size: %f x %f\n", image->width, image->height);
// Use...
for (shape = image->shapes; shape != NULL; shape = shape->next) {
	for (path = shape->paths; path != NULL; path = path->next) {
		for (i = 0; i < path->npts-1; i += 3) {
			float* p = &path->pts[i*2];
			drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
		}
	}
}
// Delete
nsvgDelete(image);
```

## Using NanoSVG in your project

In order to use NanoSVG in your own project, just copy nanosvg.h to your project.
In one C/C++ define `NANOSVG_IMPLEMENTATION` before including the library to expand the NanoSVG implementation in that file.
NanoSVG depends on `stdio.h` ,`string.h` and `math.h`, they should be included where the implementation is expanded before including NanoSVG. 

``` C
#include <stdio.h>
#include <string.h>
#include <math.h>
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"
```

By default, NanoSVG parses only the most common colors. In order to get support for full list of [SVG color keywords](http://www.w3.org/TR/SVG11/types.html#ColorKeywords), define `NANOSVG_ALL_COLOR_KEYWORDS` before expanding the implementation.

``` C
#include <stdio.h>
#include <string.h>
#include <math.h>
#define NANOSVG_ALL_COLOR_KEYWORDS	// Include full list of color keywords.
#define NANOSVG_IMPLEMENTATION		// Expands implementation
#include "nanosvg.h"
```

Alternatively, you can install the library using CMake and import it into your project using the standard CMake `find_package` command.

```CMake
add_executable(myexe main.c)

find_package(NanoSVG REQUIRED)

target_link_libraries(myexe NanoSVG::nanosvg NanoSVG::nanosvgrast)
```

# License

The library is licensed under [zlib license](LICENSE.txt)
