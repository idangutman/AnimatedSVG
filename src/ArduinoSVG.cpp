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
 * ArduinoSVG uses a modified version of NanoSVG to allow viewing SVG files on low-end devices such as Arduino.
 */

#include <stdlib.h>
#include <string.h>

#include "ArduinoSVG.h"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#define ARDUINO_SVG_UNITS  "px"
#define ARDUINO_SVG_DPI    96

// Structure containing the SVG data-types.
struct ArduinoSVGImage
{
    NSVGimage* svgImage;
    NSVGrasterizer* svgRasterizer;
    bool isAnimated;
};

// Global rasterizer, used by all instances.
static NSVGrasterizer* g_svgRasterizer = NULL;
static int g_svgRasterizerRefCount = 0;

// Constructor.
// Rasterize buffer should be in RGBA format (32 bits), but can be smaller than image size.
ArduinoSVG::ArduinoSVG(const char* svg, unsigned char* rastBuffer, int bufferWidth, int bufferHeight, int options)
{
    _svg = svg;
    _scale = 1;
    _options = options;

    _image = NULL;

    setBuffer(rastBuffer, bufferWidth, bufferHeight);
}

// Destructor.
ArduinoSVG::~ArduinoSVG()
{
    unload();
}

// Return the image width.
int ArduinoSVG::width()
{
    if (_image != NULL)
    {
        return _image->svgImage->width;
    }

    return 0;
}

// Return the image height.
int ArduinoSVG::height()
{
    if (_image != NULL)
    {
        return _image->svgImage->height;
    }

    return 0;
}

// Load the image.
bool ArduinoSVG::load()
{
    // Check not already loaded.
    if (_image != NULL)
    {
        return true;
    }

    // Allocate image.
    _image = new struct ArduinoSVGImage;
    if (_image == NULL)
    {
        return false;
    }
    memset(_image, 0, sizeof(ArduinoSVGImage));

    // Parse the SVG image.
    _image->svgImage = nsvgParse((char*)_svg, ARDUINO_SVG_UNITS, ARDUINO_SVG_DPI);
    if (_image->svgImage == NULL)
    {
        unload();
        return false;
    }
    _image->isAnimated = nsvgIsAnimated(_image->svgImage) ? true : false;

    // Create rasterizer.
    if (g_svgRasterizer == NULL)
    {
        g_svgRasterizer = nsvgCreateRasterizer();
        if (g_svgRasterizer == NULL)
        {
            unload();
            return false;
        }
    }
    _image->svgRasterizer = g_svgRasterizer;
    g_svgRasterizerRefCount++;

    // Create rasterized image.
    if (!(_options & ARDUINO_SVG_OPTION_LARGE_BUFFER))
    {
        // Prepare the rasterized image once, to allocate most of the memory on load.
        nsvgRasterizePrepare(_image->svgRasterizer, _image->svgImage, _scale);
    }

    return true;
}

// Unload the image.
void ArduinoSVG::unload()
{
    // Check not already loaded.
    if (_image == NULL)
    {
        return;
    }

    if (_image->svgImage != NULL)
    {
        nsvgDelete(_image->svgImage);
        _image->svgImage = NULL;
    }

    if (_image->svgRasterizer != NULL)
    {
        g_svgRasterizerRefCount--;
        if (g_svgRasterizerRefCount == 0)
        {
            nsvgDeleteRasterizer(g_svgRasterizer);
            g_svgRasterizer = NULL;
        }
    }

    delete _image;
    _image = NULL;
}

// Update the animation according to timestamp.
bool ArduinoSVG::update(long timeMs)
{
    // Check that image was loaded.
    if (_image == NULL || !_image->isAnimated)
    {
        return false;
    }

    // Animate the image.
    return nsvgAnimate(_image->svgImage, timeMs) ? true : false;
}

// Rasterize the image with scale and position.
void ArduinoSVG::rasterize(void* dst, int dstWidth, int dstHeight, int dstStride,
                           float tx, float ty, float scale)
{
    // Check that image was loaded.
    if (_image == NULL)
    {
        return;
    }

    if (scale != _scale)
    {
        _scale = scale;
    }

    if (!(_options & ARDUINO_SVG_OPTION_LARGE_BUFFER))
    {
        nsvgRasterizePrepare(_image->svgRasterizer, _image->svgImage, _scale);
    }

    int pitch = (_options & ARDUINO_SVG_OPTION_BGRA8888) ? 4 : 
                (_options & ARDUINO_SVG_OPTION_RGB565) ? 2 : 0;

    int bufWidth = (dstWidth <= _bufferWidth) ? dstWidth : _bufferWidth;
    int bufHeight = (dstHeight <= _bufferHeight) ? dstHeight : _bufferHeight;
    int nx = (dstWidth + bufWidth - 1) / bufWidth;
    int ny = (dstHeight + bufHeight - 1) / bufHeight;
    for (int y = 0; y < ny; y++)
    {
        for (int x = 0; x < nx; x++)
        {
            // Clear the buffer.
            memset(_rastBuffer, 0, _bufferWidth * bufHeight * 4);

            // Rasterize section of image.
            if (!(_options & ARDUINO_SVG_OPTION_LARGE_BUFFER))
            {
                nsvgRasterizeFinish(_image->svgRasterizer, tx - x * bufWidth, ty - y * bufHeight,
                                    _rastBuffer, bufWidth, bufHeight, _bufferWidth * 4);
            }
            else
            {
                nsvgRasterize(_image->svgRasterizer, _image->svgImage, tx - x * bufWidth, ty - y * bufHeight, _scale,
                              _rastBuffer, bufWidth, bufHeight, _bufferWidth * 4);
            }

            // Copy rasterized buffer.
            unsigned char* ptr = (unsigned char*)dst + x * bufWidth * pitch + y * bufHeight * dstStride;
            int w = (x + 1) * bufWidth <= dstWidth ? bufWidth : dstWidth - x * bufWidth;
            int h = (y + 1) * bufHeight <= dstHeight ? bufHeight : dstHeight - y * bufHeight;
            copyToDest(ptr, dstStride, w, h);
        }
    }
}

// Set the rasterization buffer.
void ArduinoSVG::setBuffer(unsigned char* rastBuffer, int bufferWidth, int bufferHeight)
{
    _rastBuffer = rastBuffer;
    _bufferWidth = bufferWidth;
    _bufferHeight = bufferHeight;
}

// Get the memory used by the image.
int ArduinoSVG::getImageUsedMemory()
{
    if (_image == NULL)
    {
        return 0;
    }

    return _image->svgImage->memorySize;
}

// Get the memory used by the rasterize mechanism.
int ArduinoSVG::getRasterizerUsedMemory()
{
    if (_image == NULL)
    {
        return 0;
    }

    return _image->svgRasterizer->memorySize;
}

// Copy rasterize buffer to destination.
void ArduinoSVG::copyToDest(void* dstBuffer, int dstStride, int width, int height)
{
    if (_options & ARDUINO_SVG_OPTION_BGRA8888)
    {
        if (_options & ARDUINO_SVG_OPTION_NO_ANTIALIASING)
        {
            copyRgba888ToDstBgra8888<false>((unsigned char*)dstBuffer, dstStride, width, height);
        }
        else
        {
            copyRgba888ToDstBgra8888<true>((unsigned char*)dstBuffer, dstStride, width, height);
        }
    }
    else if (_options & ARDUINO_SVG_OPTION_RGB565)
    {
        if (_options & ARDUINO_SVG_OPTION_NO_ANTIALIASING)
        {
            if (_options & ARDUINO_SVG_OPTION_SWAP_BYTES)
            {
                copyRgba888ToDstRgb565<false, true>((unsigned short*)dstBuffer, dstStride, width, height);
            }
            else
            {
                copyRgba888ToDstRgb565<false, false>((unsigned short*)dstBuffer, dstStride, width, height);
            }
        }
        else
        {
            if (_options & ARDUINO_SVG_OPTION_SWAP_BYTES)
            {
                copyRgba888ToDstRgb565<true, true>((unsigned short*)dstBuffer, dstStride, width, height);
            }
            else
            {
                copyRgba888ToDstRgb565<true, false>((unsigned short*)dstBuffer, dstStride, width, height);
            }
        }
    }
}

// Copy rasterization buffer in RGBA 8:8:8:8 to destination buffer in RGB 5:6:5.
template <bool ANTIALIASING, bool SWAP_BYTES>
void ArduinoSVG::copyRgba888ToDstRgb565(void* dstBuffer, int dstStride, int width, int height)
{
    for (int y = 0; y < height; y++)
    {
        unsigned char* src = _rastBuffer + y * _bufferWidth * 4;
        unsigned short* dst = (unsigned short*)((unsigned char*)dstBuffer + y * dstStride);
        for (int x = 0; x < width; x++)
        {
            unsigned short a = src[3];
            if (a)
            {
                if (!ANTIALIASING || (a == 0xFF))
                {
                    unsigned short d = ((src[0] & 0b11111000) << 8) | ((src[1] & 0b11111100) << 3) | (src[2] >> 3);
                    *dst = SWAP_BYTES ? d << 8 | (d >> 8) : d;
                }
                else if (ANTIALIASING)
                {
                    // antialiasing.
                    unsigned short a_1 = 256 - a;
                    unsigned short d = *dst;
                    d = SWAP_BYTES ? d << 8 | (d >> 8) : d;
                    unsigned short c0 = (d >> 8) & 0b11111000;
                    unsigned short c1 = (d >> 3) & 0b11111100;
                    unsigned short c2 = (d << 3) & 0b11111000;
                    c0 = (c0 * a_1 + src[0] * a) >> 8;
                    c1 = (c1 * a_1 + src[1] * a) >> 8;
                    c2 = (c2 * a_1 + src[2] * a) >> 8;
                    d = ((c0 & 0b11111000) << 8) | ((c1 & 0b11111100) << 3) | (c2 >> 3);
                    *dst = SWAP_BYTES ? d << 8 | (d >> 8) : d;
                }
            }
            src += 4;
            dst += 1;
        }
    }
}

// Copy rasterization buffer in RGBA 8:8:8:8 to destination buffer in RGBA 8:8:8:8.
template <bool ANTIALIASING>
void ArduinoSVG::copyRgba888ToDstBgra8888(void* dstBuffer, int dstStride, int width, int height)
{
    for (int y = 0; y < height; y++)
    {
        unsigned char* src = _rastBuffer + y * _bufferWidth * 4;
        unsigned char* dst = (unsigned char*)dstBuffer + y * dstStride;
        for (int x = 0; x < width; x++)
        {
            unsigned short a = src[3];
            if (a)
            {
                if (!ANTIALIASING || (a == 0xFF))
                {
                    dst[0] = src[2];
                    dst[1] = src[1];
                    dst[2] = src[0];
                }
                else if (ANTIALIASING)
                {
                    // antialiasing.
                    unsigned short a_1 = 256 - a;
                    unsigned short d0 = dst[0];
                    unsigned short d1 = dst[1];
                    unsigned short d2 = dst[2];
                    d0 = (d0 * a_1 + src[2] * a) >> 8;
                    d1 = (d1 * a_1 + src[1] * a) >> 8;
                    d2 = (d2 * a_1 + src[0] * a) >> 8;
                    dst[0] = d0;
                    dst[1] = d1;
                    dst[2] = d2;
                }
            }
            src += 4;
            dst += 4;
        }
    }
}
