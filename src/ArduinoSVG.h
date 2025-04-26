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

#ifndef ARDUINO_SVG_H
#define ARDUINO_SVG_H

#define ARDUINO_SVG_OPTION_NO_ANTIALIASING  0x0001      // Do not perform antialiasing of edges (faster).
#define ARDUINO_SVG_OPTION_SWAP_BYTES       0x0002      // Reverse order of bytes.
#define ARDUINO_SVG_OPTION_LARGE_BUFFER     0x0004      // Large buffers allows for faster rasterization.
#define ARDUINO_SVG_OPTION_BGRA8888         0x0008      // Output format is BGRA8888.
#define ARDUINO_SVG_OPTION_RGB565           0x0010      // Output format is RGB565.

// Internal SVG image structure.
typedef struct ArduinoSVGImage ArduinoSVGImage;

// Class for handling SVG for Arduino.
class ArduinoSVG
{
// Constructors and desttructor.
public:
    // Constructor.
    // Rasterize buffer should be in RGBA format (32 bits), but can be smaller than image size.
    ArduinoSVG(const char* svg, unsigned char* rastBuffer, int bufferWidth, int bufferHeight, int options = 0);

    // Destructor.
    ~ArduinoSVG();

// Public methods.
public:

    // Return the image width.
    int width();

    // Return the image height.
    int height();

    // Load the image.
    bool load();

    // Unload the image.
    void unload();

    // Update the animation according to timestamp.
    bool update(long timeMs);

    // Rasterize the image with scale and position.
    void rasterize(void* dst, int dstWidth, int dstHeight, int dstStride,
                   float tx = 0, float ty = 0, float scale = 1);

    // Set the rasterization buffer.
    void setBuffer(unsigned char* rastBuffer, int bufferWidth, int bufferHeight);

    // Get the memory used by the image.
    int getImageUsedMemory();

    // Get the memory used by the rasterize mechanism.
    int getRasterizerUsedMemory();

// Protected methods.
protected:

    // Copy rasterize buffer to destination.
    virtual void copyToDest(void* dstBuffer, int dstStride, int width, int height);

// Private methods.
private:

    // Copy rasterization buffer in RGBA 8:8:8:8 to destination buffer in RGB 5:6:5.
    template <bool ANTIALIASING, bool SWAP_BYTES>
    void copyRgba888ToDstRgb565(void* dstBuffer, int dstStride, int width, int height);

    // Copy rasterization buffer in RGBA 8:8:8:8 to destination buffer in BGRA 8:8:8:8.
    template <bool ANTIALIASING>
    void copyRgba888ToDstBgra8888(void* dstBuffer, int dstStride, int width, int height);

// Data structures.
private:
    struct ArduinoSVGImage* _image;
    const char* _svg;
    unsigned char* _rastBuffer;
    int _bufferWidth;
    int _bufferHeight;
    float _scale;
    int _options;
};

#endif //ARDUINO_SVG_H