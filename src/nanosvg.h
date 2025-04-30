/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
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
 * The SVG parser is based on Anti-Grain Geometry 2.4 SVG example
 * Copyright (C) 2002-2004 Maxim Shemanarev (McSeem) (http://www.antigrain.com/)
 *
 * Arc calculation code based on canvg (https://code.google.com/p/canvg/)
 *
 * Bounding box calculation based on http://blog.hackers-cafe.net/2009/06/how-to-calculate-bezier-curves-bounding.html
 *
 */

#ifndef NANOSVG_H
#define NANOSVG_H

#ifndef NANOSVG_CPLUSPLUS
#ifdef __cplusplus
extern "C" {
#endif
#endif

// NanoSVG is a simple stupid single-header-file SVG parse. The output of the parser is a list of cubic bezier shapes.
//
// The library suits well for anything from rendering scalable icons in your editor application to prototyping a game.
//
// NanoSVG supports a wide range of SVG features, but something may be missing, feel free to create a pull request!
//
// The shapes in the SVG images are transformed by the viewBox and converted to specified units.
// That is, you should get the same looking data as your designed in your favorite app.
//
// NanoSVG can return the paths in few different units. For example if you want to render an image, you may choose
// to get the paths in pixels, or if you are feeding the data into a CNC-cutter, you may want to use millimeters.
//
// The units passed to NanoSVG should be one of: 'px', 'pt', 'pc' 'mm', 'cm', or 'in'.
// DPI (dots-per-inch) controls how the unit conversion is done.
//
// If you don't know or care about the units stuff, "px" and 96 should get you going.


/* Example Usage:
	// Load SVG
	NSVGimage* image;
	image = nsvgParseFromFile("test.svg", "px", 96);
	printf("size: %f x %f\n", image->width, image->height);
	// Use...
	for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
		for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
			for (int i = 0; i < path->npts-1; i += 3) {
				float* p = &path->pts[i*2];
				drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
			}
		}
	}
	// Delete
	nsvgDelete(image);
*/

enum NSVGpaintType {
	NSVG_PAINT_UNDEF = -1,
	NSVG_PAINT_NONE = 0,
	NSVG_PAINT_COLOR = 1,
	NSVG_PAINT_LINEAR_GRADIENT = 2,
	NSVG_PAINT_RADIAL_GRADIENT = 3
};

enum NSVGspreadType {
	NSVG_SPREAD_PAD = 0,
	NSVG_SPREAD_REFLECT = 1,
	NSVG_SPREAD_REPEAT = 2
};

enum NSVGlineJoin {
	NSVG_JOIN_MITER = 0,
	NSVG_JOIN_ROUND = 1,
	NSVG_JOIN_BEVEL = 2
};

enum NSVGlineCap {
	NSVG_CAP_BUTT = 0,
	NSVG_CAP_ROUND = 1,
	NSVG_CAP_SQUARE = 2
};

enum NSVGfillRule {
	NSVG_FILLRULE_NONZERO = 0,
	NSVG_FILLRULE_EVENODD = 1
};

enum NSVGflags {
	NSVG_FLAGS_VISIBLE = 0x01
};

enum NSVGanimateType {
	NSVG_ANIMATE_TYPE_TRANSFORM_TRANSLATE = 0,
	NSVG_ANIMATE_TYPE_TRANSFORM_SCALE = 1,
	NSVG_ANIMATE_TYPE_TRANSFORM_ROTATE = 2,
	NSVG_ANIMATE_TYPE_TRANSFORM_SKEWX = 3,
	NSVG_ANIMATE_TYPE_TRANSFORM_SKEWY = 4,
	NSVG_ANIMATE_TYPE_OPACITY = 5,
	NSVG_ANIMATE_TYPE_FILL = 6,
	NSVG_ANIMATE_TYPE_FILL_OPACITY = 7,
	NSVG_ANIMATE_TYPE_STROKE = 8,
	NSVG_ANIMATE_TYPE_STROKE_OPACITY = 9,
	NSVG_ANIMATE_TYPE_STROKE_WIDTH = 10,
	NSVG_ANIMATE_TYPE_STROKE_DASHOFFSET = 11,
	NSVG_ANIMATE_TYPE_STROKE_DASHARRAY = 12,
	// Internal
	NSVG_ANIMATE_TYPE_SPLINE = -1,
	NSVG_ANIMATE_TYPE_NUMBER = -2,
};

enum NSVGanimateCalcMode {
	NSVG_ANIMATE_CALC_MODE_LINEAR = 0,
	NSVG_ANIMATE_CALC_MODE_DISCRETE = 1,
	NSVG_ANIMATE_CALC_MODE_PACED = 2,
	NSVG_ANIMATE_CALC_MODE_SPLINE = 3
};

enum NSVGanimateFill {
	NSVG_ANIMATE_FILL_REMOVE = 0,
	NSVG_ANIMATE_FILL_FREEZE = 1
};

enum NSVGanimateAdditive {
	NSVG_ANIMATE_ADDITIVE_REPLACE = 0,
	NSVG_ANIMATE_ADDITIVE_SUM = 1
};

enum NSVGanimateFlags {
	NSVG_ANIMATE_FLAG_GROUP_FIRST = 0x1,
	NSVG_ANIMATE_FLAG_GROUP_LAST = 0x2
};

typedef struct NSVGgradientStop {
	unsigned int color;
	float offset;
} NSVGgradientStop;

typedef struct NSVGgradient {
	float xform[6];
	// Original values for animations.
	struct {
		float xform[6];
	} orig;
	char spread;
	float fx, fy;
	int nstops;
	NSVGgradientStop stops[1];
} NSVGgradient;

typedef struct NSVGpaint {
	signed char type;
	union {
		unsigned int color;
		NSVGgradient* gradient;
	};
} NSVGpaint;

typedef struct NSVGpath
{
	float* pts;					// Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
	int npts;					// Total number of bezier points.
	char closed;				// Flag indicating if shapes should be treated as closed.
	float xform[6];				// Path transform.
	float bounds[4];			// Tight bounding box of the shape [minx,miny,maxx,maxy].
	struct NSVGpath* next;		// Pointer to next path, or NULL if last element.
	// Original values for animations.
	struct {
		float* pts;				// Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
		float xform[6];			// Path transform.
	} orig;
	char scaled;				// Flag whether path was scaled to viewbox.
} NSVGpath;

typedef struct NSVGid
{
	char id[64];
	struct NSVGid* next;
} NSVGid;

typedef struct NSVGshape
{
	NSVGid* id;					// Optional 'id' attr of the shape or its group
	NSVGpaint fill;				// Fill paint
	NSVGpaint stroke;			// Stroke paint
	float opacity;				// Opacity of the shape.
	float strokeWidth;			// Stroke width (scaled).
	float strokeDashOffset;		// Stroke dash offset (scaled).
	float strokeDashArray[8];	// Stroke dash array (scaled).
	char strokeDashCount;		// Number of dash values in dash array.
	char strokeLineJoin;		// Stroke join type.
	char strokeLineCap;			// Stroke cap type.
	float miterLimit;			// Miter limit
	char fillRule;				// Fill rule, see NSVGfillRule.
	unsigned char flags;		// Logical or of NSVG_FLAGS_* flags
	float bounds[4];			// Tight bounding box of the shape [minx,miny,maxx,maxy].
	NSVGid* fillGradient;		// Optional 'id' of fill gradient
	NSVGid* strokeGradient;		// Optional 'id' of stroke gradient
	float xform[6];				// Root transformation for fill/stroke gradient
	NSVGpath* paths;			// Linked list of paths in the image.
	// Original values for animations.
	struct {
		float opacity;			// Opacity of the shape.
		float xform[6];			// Root transformation for fill/stroke gradient
		NSVGpaint fill;			// Fill paint
		NSVGpaint stroke;		// Stroke paint
		float strokeWidth;		// Stroke width.
		float strokeDashOffset;	// Stroke dash offset.
		float strokeDashArray[8];// Stroke dash array.
		char strokeDashCount;	// Number of dash values in dash array.
	} orig;
	char strokeScaled;			// Flag whether stroke was scaled to viewbox.
} NSVGshape;

#define NSVG_ANIMATE

typedef struct NSVGanimate
{
	long begin;					// Beginning time of animation in milliseconds.
	long end;					// End time of animation in milliseconds, or -1 for no end.
	long dur;					// Duration of animation in milliseconds.
	long groupDur;				// Duration of entire animation group in milliseconds.
	int repeatCount;			// Number of times to repeat animation, or -1 for indefinite.

	float src[10];				// Source values for animating shape (depends on type).
	float dst[10];				// Destination values for animating shape (depends on type).
	float spline[4];			// Spline for the animation progression.
	int srcNa;					// Number of source values.
	int dstNa;					// Number of destination values.

	char type;					// Animation type, see NSVGanimateType.
	char calcMode;				// Animation interpolation mode, see NSVGanimateCalcMode.
	char additive;				// Animation additive mode, see NSVGanimateAdditive.
	char fill;					// Animation fill mode, see NSVGanimateFill.
	char flags;					// Flags for this element.

	struct NSVGanimate* next;	// Pointer to next animate, or NULL if last element.
} NSVGanimate;

typedef struct NSVGshapeNode
{
	int shapeDepth;					// Depth of the shape in the shapes tree.
	struct NSVGshape* shape;		// Pointer to the shape.
	struct NSVGshapeNode* prev;		// Pointer to previous shape node, or NULL if first element.
	struct NSVGshapeNode* next;		// Pointer to next shape node, or NULL if last element.
	struct NSVGshapeNode* parent;	// Pointer to parent shape node, or NULL if root element.
	NSVGanimate* animates;			// Linked list of animations for the shape.
	NSVGanimate* animatesTail;		// Tail of linked list of animations for the shape.
} NSVGshapeNode;

typedef struct NSVGimage
{
	float width;				// Width of the image.
	float height;				// Height of the image.
	float viewMinx;				// Viewbox min X.
	float viewMiny;				// Viewbox min Y.
	float viewWidth;			// Viewbox width.
	float viewHeight;			// Viewbox height.
	float fontSize;				// Font size.
	float dpi;					// Image DPI.
	int alignX;					// Alignment X.
	int alignY;					// Alignment Y.
	int alignType;				// Alignment type.
	char units[3];				// Units.
	NSVGshapeNode* shapes;		// Linked list of shapes in the image.
	int memorySize;				// Amount of memory in bytes that was allocated by the image.
} NSVGimage;

// Parses SVG file from a file, returns SVG image as paths.
NSVGimage* nsvgParseFromFile(const char* filename, const char* units, float dpi);

// Parses SVG file from a null terminated string, returns SVG image as paths.
// Important note: changes the string.
NSVGimage* nsvgParse(char* input, const char* units, float dpi);

// Duplicates a path.
NSVGpath* nsvgDuplicatePath(NSVGpath* p);

// Deletes an image.
void nsvgDelete(NSVGimage* image);

// Animate SVG by time. Returns whether image was updated.
char nsvgAnimate(NSVGimage* image, long timeMs);

#ifndef NANOSVG_CPLUSPLUS
#ifdef __cplusplus
}
#endif
#endif

#ifdef NANOSVG_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define NSVG_PI (3.14159265358979323846264338327f)
#define NSVG_KAPPA90 (0.5522847493f)	// Length proportional to radius of a cubic bezier handle for 90deg arcs.

#define NSVG_ALIGN_MIN 0
#define NSVG_ALIGN_MID 1
#define NSVG_ALIGN_MAX 2
#define NSVG_ALIGN_NONE 0
#define NSVG_ALIGN_MEET 1
#define NSVG_ALIGN_SLICE 2

#define NSVG_NOTUSED(v) do { (void)(1 ? (void)0 : ( (void)(v) ) ); } while(0)
#define NSVG_RGB(r, g, b) (((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16))

#ifdef _MSC_VER
	#pragma warning (disable: 4996) // Switch off security warnings
	#pragma warning (disable: 4100) // Switch off unreferenced formal parameter warnings
	#ifdef __cplusplus
	#define NSVG_INLINE inline
	#else
	#define NSVG_INLINE
	#endif
#else
	#define NSVG_INLINE inline
#endif


static int nsvg__isspace(char c)
{
	return strchr(" \t\n\v\f\r", c) != 0;
}

static int nsvg__isdigit(char c)
{
	return c >= '0' && c <= '9';
}

static NSVG_INLINE float nsvg__minf(float a, float b) { return a < b ? a : b; }
static NSVG_INLINE float nsvg__maxf(float a, float b) { return a > b ? a : b; }


// Simple XML parser

#define NSVG_XML_TAG 1
#define NSVG_XML_CONTENT 2
#define NSVG_XML_COMMENT 3
#define NSVG_XML_MAX_ATTRIBS 64

typedef struct NSVGattrValue
{
	const char* name;
	int nameLen;
	const char* value;
	int valueLen;
} NSVGattrValue;

static void nsvg__parseContent(const char* content, int contentLen,
							   void (*contentCb)(void* userData, const char* content, int contentLen),
							   void* userData)
{
	// Trim start white spaces
	for (;contentLen > 0 && nsvg__isspace(*content); content++, contentLen--);

	if (contentLen <= 0) return;

	if (contentCb)
		(*contentCb)(userData, content, contentLen);
}

static void nsvg__parseElement(const char* elementStart, int elementLen,
							   void (*startelCb)(void* userData, const char* elName, int elNameLen, NSVGattrValue* attr, int nattr),
							   void (*endelCb)(void* userData, const char* elName, int elNameLen),
							   void* userData)
{
	NSVGattrValue attr[NSVG_XML_MAX_ATTRIBS];
	int nattr = 0;
	const char* name;
	const char* s = elementStart;
	int len = elementLen;
	int nameLen;
	int start = 0;
	int end = 0;
	char quote;

	// Skip white space after the '<'
	for (; *s && len > 0 && nsvg__isspace(*s); s++, len--);

	// Check if the tag is end tag
	if (*s == '/') {
		s++;
		len--;
		end = 1;
	} else {
		start = 1;
	}

	// Skip comments, data and preprocessor stuff.
	if (!*s || (len <= 0) || *s == '?' || *s == '!')
		return;

	// Get tag name
	name = s;
	for (; *s && len > 0 && !nsvg__isspace(*s); s++, len--);
	nameLen = s - name;

	// Get attribs
	while (!end && *s && len > 0 && nattr < NSVG_XML_MAX_ATTRIBS-3) {
		const char* attrName = NULL;
		const char* attrValue = NULL;
		int attrNameLen = 0;
		int attrValueLen = 0;

		// Skip white space before the attrib name
		for (; *s && len > 0 && nsvg__isspace(*s); s++, len--);
		if (!*s || len <= 0) break;
		if (*s == '/') {
			end = 1;
			break;
		}
		attrName = s;
		// Find end of the attrib name.
		for (; *s && len > 0 && !nsvg__isspace(*s) && *s != '='; s++, len--);
		if (!*s || len > 0) { 
			attrNameLen = s - attrName;
			s++;
			len--;
		}
		// Skip until the beginning of the value.
		for (; *s && (len > 0) && *s != '\"' && *s != '\''; s++, len--);
		if (!*s || len <= 0) break;
		quote = *s;
		s++;
		len--;
		// Store value and find the end of it.
		attrValue = s;
		for (; *s && (len > 0) && *s != quote; s++, len--);
		if (*s && len > 0) { 
			attrValueLen = s - attrValue;
			s++;
			len--;
		}

		// Store only well formed attributes
		if (attrName && attrValue) {
			attr[nattr].name = attrName;
			attr[nattr].nameLen = attrNameLen;
			attr[nattr].value = attrValue;
			attr[nattr].valueLen = attrValueLen;
			nattr++;
		}
	}

	// Call callbacks.
	if (start && startelCb)
		(*startelCb)(userData, name, nameLen, attr, nattr);
	if (end && endelCb)
		(*endelCb)(userData, name, nameLen);
}

int nsvg__parseXML(const char* input,
				   void (*startelCb)(void* userData, const char* elName, int elNameLen, NSVGattrValue* attr, int nattr),
				   void (*endelCb)(void* userData, const char* elName, int elNameLen),
				   void (*contentCb)(void* userData, const char* content, int contentLen),
				   void* userData)
{
	const char* s = input;
	const char* mark = s;
	int state = NSVG_XML_CONTENT;
	while (*s) {
		if (state == NSVG_XML_CONTENT && *s == '<') {
			if (mark < s) {
				nsvg__parseContent(mark, s - mark, contentCb, userData);
			}
			// Start of a tag
			if (strncmp(s, "<!--", 4) == 0) {
				state = NSVG_XML_COMMENT;
				s += 4;
			} else {
				mark = ++s;
				state = NSVG_XML_TAG;
			}
		} else if (state == NSVG_XML_TAG && *s == '>') {
			// Start of a content or new tag.
			nsvg__parseElement(mark, s - mark, startelCb, endelCb, userData);
			mark = ++s;
			state = NSVG_XML_CONTENT;
		} else if (state == NSVG_XML_COMMENT && strncmp(s, "-->", 3) == 0) {
			state = NSVG_XML_CONTENT;
			s += 3;
			mark = s;
		} else {
			s++;
		}
	}

	return 1;
}

/* Simple SVG parser. */

enum NSVGgradientUnits {
	NSVG_USER_SPACE = 0,
	NSVG_OBJECT_SPACE = 1
};

#define NSVG_MAX_DASHES 8

enum NSVGunits {
	NSVG_UNITS_USER,
	NSVG_UNITS_PX,
	NSVG_UNITS_PT,
	NSVG_UNITS_PC,
	NSVG_UNITS_MM,
	NSVG_UNITS_CM,
	NSVG_UNITS_IN,
	NSVG_UNITS_PERCENT,
	NSVG_UNITS_EM,
	NSVG_UNITS_EX
};

typedef struct NSVGcoordinate {
	float value;
	int units;
} NSVGcoordinate;

typedef struct NSVGlinearData {
	NSVGcoordinate x1, y1, x2, y2;
} NSVGlinearData;

typedef struct NSVGradialData {
	NSVGcoordinate cx, cy, r, fx, fy;
} NSVGradialData;

typedef struct NSVGgradientData
{
	NSVGid* id;
	NSVGid* ref;
	signed char type;
	union {
		NSVGlinearData linear;
		NSVGradialData radial;
	};
	char spread;
	char units;
	float xform[6];
	int nstops;
	NSVGgradientStop* stops;
	struct NSVGgradientData* next;
} NSVGgradientData;

typedef struct NSVGattrib
{
	NSVGid* id;
	float xform[6];
	unsigned int fillColor;
	unsigned int strokeColor;
	float opacity;
	float fillOpacity;
	float strokeOpacity;
	NSVGid* fillGradient;
	NSVGid* strokeGradient;
	float strokeWidth;
	float strokeDashOffset;
	float strokeDashArray[NSVG_MAX_DASHES];
	int strokeDashCount;
	char strokeLineJoin;
	char strokeLineCap;
	float miterLimit;
	char fillRule;
	float fontSize;
	unsigned int stopColor;
	float stopOpacity;
	float stopOffset;
	char hasFill;
	char hasStroke;
	char visible;
	struct NSVGattrib* next;
} NSVGattrib;

typedef struct NSVGparser
{
	NSVGattrib* attrHead;
	NSVGattrib* attrFree;
	NSVGid* idFree;
	float* pts;
	int npts;
	int cpts;
	NSVGpath* plist;
	NSVGimage* image;
	NSVGgradientData* gradients;
	NSVGshapeNode* shapesTail;
	char pathFlag;
	char defsFlag;
	int shapeDepth;
} NSVGparser;

static void nsvg__xformIdentity(float* t)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetTranslation(float* t, float tx, float ty)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = tx; t[5] = ty;
}

static void nsvg__xformSetScale(float* t, float sx, float sy)
{
	t[0] = sx; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = sy;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetSkewX(float* t, float a)
{
	a = a/180.0f*NSVG_PI; // deg 2 rad
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = tanf(a); t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetSkewY(float* t, float a)
{
	a = a/180.0f*NSVG_PI; // deg 2 rad
	t[0] = 1.0f; t[1] = tanf(a);
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformSetRotation(float* t, float a)
{
	float cs, sn;
	a = a/180.0f*NSVG_PI; // deg 2 rad
	cs = cosf(a);
	sn = sinf(a);
	t[0] = cs; t[1] = sn;
	t[2] = -sn; t[3] = cs;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void nsvg__xformMultiply(float* t, float* s)
{
	float t0 = t[0] * s[0] + t[1] * s[2];
	float t2 = t[2] * s[0] + t[3] * s[2];
	float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
	t[1] = t[0] * s[1] + t[1] * s[3];
	t[3] = t[2] * s[1] + t[3] * s[3];
	t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
	t[0] = t0;
	t[2] = t2;
	t[4] = t4;
}

static void nsvg__xformSetNonCenterRotation(float* t, float a, float tx, float ty)
{
	float m[6];
	nsvg__xformIdentity(m);

	nsvg__xformSetTranslation(t, -tx, -ty);
	nsvg__xformMultiply(m, t);

	nsvg__xformSetRotation(t, a);
	nsvg__xformMultiply(m, t);

	nsvg__xformSetTranslation(t, tx, ty);
	nsvg__xformMultiply(m, t);

	memcpy(t, m, sizeof(float)*6);
}

static void nsvg__xformInverse(float* inv, float* t)
{
	double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
	if (det > -1e-6 && det < 1e-6) {
		nsvg__xformIdentity(t);
		return;
	}
	invdet = 1.0 / det;
	inv[0] = (float)(t[3] * invdet);
	inv[2] = (float)(-t[2] * invdet);
	inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
	inv[1] = (float)(-t[1] * invdet);
	inv[3] = (float)(t[0] * invdet);
	inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
}

static void nsvg__xformPremultiply(float* t, float* s)
{
	float s2[6];
	memcpy(s2, s, sizeof(float)*6);
	nsvg__xformMultiply(s2, t);
	memcpy(t, s2, sizeof(float)*6);
}

static void nsvg__xformPoint(float* dx, float* dy, float x, float y, float* t)
{
	*dx = x*t[0] + y*t[2] + t[4];
	*dy = x*t[1] + y*t[3] + t[5];
}

static void nsvg__xformVec(float* dx, float* dy, float x, float y, float* t)
{
	*dx = x*t[0] + y*t[2];
	*dy = x*t[1] + y*t[3];
}

#define NSVG_EPSILON (1e-12)

static int nsvg__ptInBounds(float* pt, float* bounds)
{
	return pt[0] >= bounds[0] && pt[0] <= bounds[2] && pt[1] >= bounds[1] && pt[1] <= bounds[3];
}

static double nsvg__evalBezier(double t, double p0, double p1, double p2, double p3)
{
	double it = 1.0-t;
	return it*it*it*p0 + 3.0*it*it*t*p1 + 3.0*it*t*t*p2 + t*t*t*p3;
}

static void nsvg__curveBounds(float* bounds, float* curve)
{
	int i, j, count;
	double roots[2], a, b, c, b2ac, t, v;
	float* v0 = &curve[0];
	float* v1 = &curve[2];
	float* v2 = &curve[4];
	float* v3 = &curve[6];

	// Start the bounding box by end points
	bounds[0] = nsvg__minf(v0[0], v3[0]);
	bounds[1] = nsvg__minf(v0[1], v3[1]);
	bounds[2] = nsvg__maxf(v0[0], v3[0]);
	bounds[3] = nsvg__maxf(v0[1], v3[1]);

	// Bezier curve fits inside the convex hull of it's control points.
	// If control points are inside the bounds, we're done.
	if (nsvg__ptInBounds(v1, bounds) && nsvg__ptInBounds(v2, bounds))
		return;

	// Add bezier curve inflection points in X and Y.
	for (i = 0; i < 2; i++) {
		a = -3.0 * v0[i] + 9.0 * v1[i] - 9.0 * v2[i] + 3.0 * v3[i];
		b = 6.0 * v0[i] - 12.0 * v1[i] + 6.0 * v2[i];
		c = 3.0 * v1[i] - 3.0 * v0[i];
		count = 0;
		if (fabs(a) < NSVG_EPSILON) {
			if (fabs(b) > NSVG_EPSILON) {
				t = -c / b;
				if (t > NSVG_EPSILON && t < 1.0-NSVG_EPSILON)
					roots[count++] = t;
			}
		} else {
			b2ac = b*b - 4.0*c*a;
			if (b2ac > NSVG_EPSILON) {
				t = (-b + sqrt(b2ac)) / (2.0 * a);
				if (t > NSVG_EPSILON && t < 1.0-NSVG_EPSILON)
					roots[count++] = t;
				t = (-b - sqrt(b2ac)) / (2.0 * a);
				if (t > NSVG_EPSILON && t < 1.0-NSVG_EPSILON)
					roots[count++] = t;
			}
		}
		for (j = 0; j < count; j++) {
			v = nsvg__evalBezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
			bounds[0+i] = nsvg__minf(bounds[0+i], (float)v);
			bounds[2+i] = nsvg__maxf(bounds[2+i], (float)v);
		}
	}
}

static void* nsvg__malloc(NSVGimage* image, int size)
{
	void* ptr = malloc(size);
	if (ptr == NULL)
	{
		return NULL;
	}

	image->memorySize += size;

	return ptr;
}

static void* nsvg__realloc(NSVGimage* image, void* ptr, int size, int prevSize)
{
	void* ptr2 = realloc(ptr, size);
	if (ptr2 == NULL)
	{
		return NULL;
	}

	image->memorySize += size - prevSize;

	return ptr2;
}

static void nsvg__free(NSVGimage* image, void* ptr, int size)
{
	if (ptr == NULL) return;
	free(ptr);
	image->memorySize -= size;
}

static NSVGparser* nsvg__createParser(void)
{
	NSVGparser* p;
	NSVGattrib* attr;

	p = (NSVGparser*)malloc(sizeof(NSVGparser));
	if (p == NULL) goto error;
	memset(p, 0, sizeof(NSVGparser));

	p->image = (NSVGimage*)malloc(sizeof(NSVGimage));
	if (p->image == NULL) goto error;
	memset(p->image, 0, sizeof(NSVGimage));

	attr = (NSVGattrib*)malloc(sizeof(NSVGattrib));
	if (attr == NULL) goto error;
	memset(attr, 0, sizeof(NSVGattrib));

	p->attrHead = attr;

	// Init style
	nsvg__xformIdentity(attr->xform);
	attr->fillColor = NSVG_RGB(0,0,0);
	attr->strokeColor = NSVG_RGB(0,0,0);
	attr->opacity = 1;
	attr->fillOpacity = 1;
	attr->strokeOpacity = 1;
	attr->stopOpacity = 1;
	attr->strokeWidth = 1;
	attr->strokeLineJoin = NSVG_JOIN_MITER;
	attr->strokeLineCap = NSVG_CAP_BUTT;
	attr->miterLimit = 4;
	attr->fillRule = NSVG_FILLRULE_NONZERO;
	attr->hasFill = 1;
	attr->visible = 1;

	return p;

error:
	if (p) {
		if (p->image) free(p->image);
		free(p);
	}
	return NULL;
}

static void nsvg__deletePaths(NSVGimage* image, NSVGpath* path)
{
	while (path) {
		NSVGpath *next = path->next;
		if (path->pts != NULL)
			nsvg__free(image, path->pts, path->npts*2*sizeof(float));
		if (path->orig.pts != NULL)
			nsvg__free(image, path->orig.pts, path->npts*2*sizeof(float));
		nsvg__free(image, path, sizeof(NSVGpath));
		path = next;
	}
}

static void nsvg__deletePaint(NSVGimage* image, NSVGpaint* paint)
{
	if (paint->type == NSVG_PAINT_LINEAR_GRADIENT || paint->type == NSVG_PAINT_RADIAL_GRADIENT)
		nsvg__free(image, paint->gradient, sizeof(NSVGgradient));
}

static void nsvg__deleteGradientData(NSVGparser* p, NSVGgradientData* grad)
{
	NSVGgradientData* next;
	while (grad != NULL) {
		next = grad->next;
		nsvg__free(p->image, grad->id, sizeof(NSVGid));
		nsvg__free(p->image, grad->ref, sizeof(NSVGid));
		nsvg__free(p->image, grad->stops, sizeof(NSVGgradientStop)*grad->nstops);
		nsvg__free(p->image, grad, sizeof(NSVGgradientData));
		grad = next;
	}
}

static void nsvg__deleteParser(NSVGparser* p)
{
	NSVGattrib* attr;
	NSVGid* id;

	if (p != NULL) {
		nsvg__deletePaths(p->image, p->plist);
		nsvg__deleteGradientData(p, p->gradients);
		free(p->pts);

		for (attr = p->attrHead; attr != NULL;) {
			NSVGattrib* next = attr->next;
			free(attr);
			attr = next;
		}
		for (attr = p->attrFree; attr != NULL;) {
			NSVGattrib* next = attr->next;
			free(attr);
			attr = next;
		}

		for (id = p->idFree; id != NULL;) {
			NSVGid* next = id->next;
			nsvg__free(p->image, id, sizeof(NSVGid));
			id = next;
		}

		free(p);
	}
}

static void nsvg__resetPath(NSVGparser* p)
{
	p->npts = 0;
}

static void nsvg__addPoint(NSVGparser* p, float x, float y)
{
	if (p->npts+1 > p->cpts) {
		int cpts = p->cpts ? p->cpts*2 : 8;
		p->pts = (float*)nsvg__realloc(p->image, p->pts, cpts*2*sizeof(float), p->cpts*2*sizeof(float));
		p->cpts = cpts;
		if (!p->pts) return;
	}
	p->pts[p->npts*2+0] = x;
	p->pts[p->npts*2+1] = y;
	p->npts++;
}

static void nsvg__moveTo(NSVGparser* p, float x, float y)
{
	if (p->npts > 0) {
		p->pts[(p->npts-1)*2+0] = x;
		p->pts[(p->npts-1)*2+1] = y;
	} else {
		nsvg__addPoint(p, x, y);
	}
}

static void nsvg__lineTo(NSVGparser* p, float x, float y)
{
	float px,py, dx,dy;
	if (p->npts > 0) {
		px = p->pts[(p->npts-1)*2+0];
		py = p->pts[(p->npts-1)*2+1];
		dx = x - px;
		dy = y - py;
		nsvg__addPoint(p, px + dx/3.0f, py + dy/3.0f);
		nsvg__addPoint(p, x - dx/3.0f, y - dy/3.0f);
		nsvg__addPoint(p, x, y);
	}
}

static void nsvg__cubicBezTo(NSVGparser* p, float cpx1, float cpy1, float cpx2, float cpy2, float x, float y)
{
	if (p->npts > 0) {
		nsvg__addPoint(p, cpx1, cpy1);
		nsvg__addPoint(p, cpx2, cpy2);
		nsvg__addPoint(p, x, y);
	}
}

static NSVGid* nsvg__allocId(NSVGparser* p)
{
	NSVGid* id;

	if (p->idFree != NULL) {
		id = p->idFree;
		p->idFree = p->idFree->next;
	} else {
		id = (NSVGid*)nsvg__malloc(p->image, sizeof(NSVGid));
		if (id == NULL) return NULL;
	}
	id->next = NULL;
	return id;
}

static void nsvg__freeId(NSVGparser* p, NSVGid* id)
{
	if (id == NULL) return;

	id->next = p->idFree;
	p->idFree = id;
}

static NSVGid* nsvg__cloneId(NSVGparser* p, NSVGid* id)
{
	NSVGid* clone;

	if (id == NULL) return NULL;
	
	clone = nsvg__allocId(p);
	if (clone == NULL) return NULL;

	*clone = *id;
	clone->next = NULL;

	return clone;
}

static NSVGattrib* nsvg__getAttr(NSVGparser* p)
{
	return p->attrHead;
}

static void nsvg__pushAttr(NSVGparser* p)
{
	NSVGattrib* prevHead = p->attrHead;
	NSVGattrib* attr;

	if (p->attrFree != NULL) {
		attr = p->attrFree;
		p->attrFree = p->attrFree->next;
	} else {
		attr = (NSVGattrib*)malloc(sizeof(NSVGattrib));
		if (attr == NULL) return;
	}
	*attr = *prevHead;
	attr->next = prevHead;
	p->attrHead = attr;

	// Do not copy IDs
	attr->id = NULL;
	attr->fillGradient = nsvg__cloneId(p, prevHead->fillGradient);
	attr->strokeGradient = nsvg__cloneId(p, prevHead->strokeGradient);
}

static void nsvg__popAttr(NSVGparser* p)
{
	NSVGattrib* prevHead = p->attrHead;

	if (p->attrHead->next != NULL) {
		p->attrHead = p->attrHead->next;

		prevHead->next = p->attrFree;
		p->attrFree = prevHead;

		// Free the used.
		nsvg__freeId(p, prevHead->id);
		nsvg__freeId(p, prevHead->fillGradient);
		nsvg__freeId(p, prevHead->strokeGradient);
	}
}

static float nsvg__actualOrigX(NSVGparser* p)
{
	return p->image->viewMinx;
}

static float nsvg__actualOrigY(NSVGparser* p)
{
	return p->image->viewMiny;
}

static float nsvg__actualWidth(NSVGparser* p)
{
	return p->image->viewWidth;
}

static float nsvg__actualHeight(NSVGparser* p)
{
	return p->image->viewHeight;
}

static float nsvg__actualLength(NSVGparser* p)
{
	float w = nsvg__actualWidth(p), h = nsvg__actualHeight(p);
	return sqrtf(w*w + h*h) / sqrtf(2.0f);
}

static float nsvg__convertToPixels(NSVGimage* image, NSVGcoordinate c, float orig, float length)
{
	switch (c.units) {
		case NSVG_UNITS_USER:		return c.value;
		case NSVG_UNITS_PX:			return c.value;
		case NSVG_UNITS_PT:			return c.value / 72.0f * image->dpi;
		case NSVG_UNITS_PC:			return c.value / 6.0f * image->dpi;
		case NSVG_UNITS_MM:			return c.value / 25.4f * image->dpi;
		case NSVG_UNITS_CM:			return c.value / 2.54f * image->dpi;
		case NSVG_UNITS_IN:			return c.value * image->dpi;
		case NSVG_UNITS_EM:			return c.value * image->fontSize;
		case NSVG_UNITS_EX:			return c.value * image->fontSize * 0.52f; // x-height of Helvetica.
		case NSVG_UNITS_PERCENT:	return orig + c.value / 100.0f * length;
		default:					return c.value;
	}
	return c.value;
}

static NSVGgradientData* nsvg__findGradientData(NSVGparser* p, const char* id)
{
	NSVGgradientData* grad = p->gradients;
	if (id == NULL || *id == '\0')
		return NULL;
	while (grad != NULL) {
		if ((grad->id != NULL) && (strcmp(grad->id->id, id) == 0))
			return grad;
		grad = grad->next;
	}
	return NULL;
}

static NSVGgradient* nsvg__createGradient(NSVGparser* p, const char* id, const float* localBounds, float *xform, signed char* paintType)
{
	NSVGgradientData* data = NULL;
	NSVGgradientData* ref = NULL;
	NSVGgradientStop* stops = NULL;
	NSVGgradient* grad;
	float ox, oy, sw, sh, sl;
	int nstops = 0;
	int refIter;

	data = nsvg__findGradientData(p, id);
	if (data == NULL) return NULL;

	// TODO: use ref to fill in all unset values too.
	ref = data;
	refIter = 0;
	while (ref != NULL) {
		NSVGgradientData* nextRef = NULL;
		if (stops == NULL && ref->stops != NULL) {
			stops = ref->stops;
			nstops = ref->nstops;
			break;
		}
		nextRef = nsvg__findGradientData(p, ref->ref == NULL ? NULL : ref->ref->id);
		if (nextRef == ref) break; // prevent infite loops on malformed data
		ref = nextRef;
		refIter++;
		if (refIter > 32) break; // prevent infite loops on malformed data
	}
	if (stops == NULL) return NULL;

	grad = (NSVGgradient*)nsvg__malloc(p->image, sizeof(NSVGgradient) + sizeof(NSVGgradientStop)*(nstops-1));
	if (grad == NULL) return NULL;

	// The shape width and height.
	if (data->units == NSVG_OBJECT_SPACE) {
		ox = localBounds[0];
		oy = localBounds[1];
		sw = localBounds[2] - localBounds[0];
		sh = localBounds[3] - localBounds[1];
	} else {
		ox = nsvg__actualOrigX(p);
		oy = nsvg__actualOrigY(p);
		sw = nsvg__actualWidth(p);
		sh = nsvg__actualHeight(p);
	}
	sl = sqrtf(sw*sw + sh*sh) / sqrtf(2.0f);

	if (data->type == NSVG_PAINT_LINEAR_GRADIENT) {
		float x1, y1, x2, y2, dx, dy;
		x1 = nsvg__convertToPixels(p->image, data->linear.x1, ox, sw);
		y1 = nsvg__convertToPixels(p->image, data->linear.y1, oy, sh);
		x2 = nsvg__convertToPixels(p->image, data->linear.x2, ox, sw);
		y2 = nsvg__convertToPixels(p->image, data->linear.y2, oy, sh);
		// Calculate transform aligned to the line
		dx = x2 - x1;
		dy = y2 - y1;
		grad->xform[0] = dy; grad->xform[1] = -dx;
		grad->xform[2] = dx; grad->xform[3] = dy;
		grad->xform[4] = x1; grad->xform[5] = y1;
	} else {
		float cx, cy, fx, fy, r;
		cx = nsvg__convertToPixels(p->image, data->radial.cx, ox, sw);
		cy = nsvg__convertToPixels(p->image, data->radial.cy, oy, sh);
		fx = nsvg__convertToPixels(p->image, data->radial.fx, ox, sw);
		fy = nsvg__convertToPixels(p->image, data->radial.fy, oy, sh);
		r = nsvg__convertToPixels(p->image, data->radial.r, 0, sl);
		// Calculate transform aligned to the circle
		grad->xform[0] = r; grad->xform[1] = 0;
		grad->xform[2] = 0; grad->xform[3] = r;
		grad->xform[4] = cx; grad->xform[5] = cy;
		grad->fx = fx / r;
		grad->fy = fy / r;
	}

	nsvg__xformMultiply(grad->xform, data->xform);
	nsvg__xformMultiply(grad->xform, xform);

	memcpy(grad->orig.xform, grad->xform, sizeof(float)*6);

	grad->spread = data->spread;
	memcpy(grad->stops, stops, nstops*sizeof(NSVGgradientStop));
	grad->nstops = nstops;

	*paintType = data->type;

	return grad;
}

static float nsvg__getAverageScale(float* t)
{
	float sx = sqrtf(t[0]*t[0] + t[2]*t[2]);
	float sy = sqrtf(t[1]*t[1] + t[3]*t[3]);
	return (sx + sy) * 0.5f;
}

static void nsvg__getLocalBounds(float* bounds, NSVGshape *shape, float* xform)
{
	NSVGpath* path;
	float curve[4*2], curveBounds[4];
	int i, first = 1;
	for (path = shape->paths; path != NULL; path = path->next) {
		nsvg__xformPoint(&curve[0], &curve[1], path->pts[0], path->pts[1], xform);
		for (i = 0; i < path->npts-1; i += 3) {
			nsvg__xformPoint(&curve[2], &curve[3], path->pts[(i+1)*2], path->pts[(i+1)*2+1], xform);
			nsvg__xformPoint(&curve[4], &curve[5], path->pts[(i+2)*2], path->pts[(i+2)*2+1], xform);
			nsvg__xformPoint(&curve[6], &curve[7], path->pts[(i+3)*2], path->pts[(i+3)*2+1], xform);
			nsvg__curveBounds(curveBounds, curve);
			if (first) {
				bounds[0] = curveBounds[0];
				bounds[1] = curveBounds[1];
				bounds[2] = curveBounds[2];
				bounds[3] = curveBounds[3];
				first = 0;
			} else {
				bounds[0] = nsvg__minf(bounds[0], curveBounds[0]);
				bounds[1] = nsvg__minf(bounds[1], curveBounds[1]);
				bounds[2] = nsvg__maxf(bounds[2], curveBounds[2]);
				bounds[3] = nsvg__maxf(bounds[3], curveBounds[3]);
			}
			curve[0] = curve[6];
			curve[1] = curve[7];
		}
	}
}

static void nsvg__updateShapeBounds(NSVGshape* shape)
{
	NSVGpath* path;

	// Calculate shape bounds
	shape->bounds[0] = shape->paths->bounds[0];
	shape->bounds[1] = shape->paths->bounds[1];
	shape->bounds[2] = shape->paths->bounds[2];
	shape->bounds[3] = shape->paths->bounds[3];
	for (path = shape->paths->next; path != NULL; path = path->next) {
		shape->bounds[0] = nsvg__minf(shape->bounds[0], path->bounds[0]);
		shape->bounds[1] = nsvg__minf(shape->bounds[1], path->bounds[1]);
		shape->bounds[2] = nsvg__maxf(shape->bounds[2], path->bounds[2]);
		shape->bounds[3] = nsvg__maxf(shape->bounds[3], path->bounds[3]);
	}
}

static void nsvg__scaleShapeStroke(NSVGshape* shape, float* xform)
{
	float scale = nsvg__getAverageScale(xform);
	int i;

	shape->strokeWidth *= scale;
	shape->strokeDashOffset *= scale;
	for (i = 0; i < shape->strokeDashCount; i++)
		shape->strokeDashArray[i] *= scale;
	
	// Mark stroke as not scaled, so it is scaled to viewbox later.
	shape->strokeScaled = 0;
}

static void nsvg__addShape(NSVGparser* p)
{
	NSVGattrib* attr = nsvg__getAttr(p);
	float scale = 1.0f;
	NSVGshapeNode* shapeNode;
	NSVGshape* shape;
	NSVGpath* path;
	int i;

	if (p->plist == NULL)
		return;

	shape = (NSVGshape*)nsvg__malloc(p->image, sizeof(NSVGshape));
	if (shape == NULL) goto error;
	memset(shape, 0, sizeof(NSVGshape));

	shapeNode = (NSVGshapeNode*)nsvg__malloc(p->image, sizeof(NSVGshapeNode));
	if (shapeNode == NULL) goto error;
	memset(shapeNode, 0, sizeof(NSVGshapeNode));

	shapeNode->shapeDepth = p->shapeDepth;
	shapeNode->shape = shape;

	shape->id = attr->id;
	shape->fillGradient = attr->fillGradient;
	shape->strokeGradient = attr->strokeGradient;
	memcpy(shape->xform, attr->xform, sizeof shape->xform);
	shape->strokeWidth = attr->strokeWidth;
	shape->strokeDashOffset = attr->strokeDashOffset;
	for (i = 0; i < attr->strokeDashCount; i++)
		shape->strokeDashArray[i] = attr->strokeDashArray[i];
	shape->strokeDashCount = (char)attr->strokeDashCount;
	nsvg__scaleShapeStroke(shape, shape->xform);
	shape->strokeLineJoin = attr->strokeLineJoin;
	shape->strokeLineCap = attr->strokeLineCap;
	shape->miterLimit = attr->miterLimit;
	shape->fillRule = attr->fillRule;
	shape->opacity = attr->opacity;

	shape->paths = p->plist;
	p->plist = NULL;

	// Remove the IDs from the attribute (moved to shape).
	attr->id = NULL;
	attr->fillGradient = NULL;
	attr->strokeGradient = NULL;

	// Calculate shape bounds
	shape->bounds[0] = shape->paths->bounds[0];
	shape->bounds[1] = shape->paths->bounds[1];
	shape->bounds[2] = shape->paths->bounds[2];
	shape->bounds[3] = shape->paths->bounds[3];
	for (path = shape->paths->next; path != NULL; path = path->next) {
		shape->bounds[0] = nsvg__minf(shape->bounds[0], path->bounds[0]);
		shape->bounds[1] = nsvg__minf(shape->bounds[1], path->bounds[1]);
		shape->bounds[2] = nsvg__maxf(shape->bounds[2], path->bounds[2]);
		shape->bounds[3] = nsvg__maxf(shape->bounds[3], path->bounds[3]);
	}

	// Set fill
	if (attr->hasFill == 0) {
		shape->fill.type = NSVG_PAINT_NONE;
	} else if (attr->hasFill == 1) {
		shape->fill.type = NSVG_PAINT_COLOR;
		shape->fill.color = attr->fillColor;
		shape->fill.color |= (unsigned int)(attr->fillOpacity*255) << 24;
	} else if (attr->hasFill == 2) {
		shape->fill.type = NSVG_PAINT_UNDEF;
	}

	// Set stroke
	if (attr->hasStroke == 0) {
		shape->stroke.type = NSVG_PAINT_NONE;
	} else if (attr->hasStroke == 1) {
		shape->stroke.type = NSVG_PAINT_COLOR;
		shape->stroke.color = attr->strokeColor;
		shape->stroke.color |= (unsigned int)(attr->strokeOpacity*255) << 24;
	} else if (attr->hasStroke == 2) {
		shape->stroke.type = NSVG_PAINT_UNDEF;
	}

	// Set flags
	shape->flags = (attr->visible ? NSVG_FLAGS_VISIBLE : 0x00);

	// Store original values for animation.
	shape->orig.opacity = attr->opacity;
	memcpy(shape->orig.xform, shape->xform, sizeof shape->xform);
	memcpy(&shape->orig.fill, &shape->fill, sizeof(shape->orig.fill));
	memcpy(&shape->orig.stroke, &shape->stroke, sizeof(shape->orig.stroke));
	shape->orig.strokeWidth = shape->strokeWidth;
	shape->orig.strokeDashOffset = shape->strokeDashOffset;
	for (i = 0; i < shape->strokeDashCount; i++)
		shape->orig.strokeDashArray[i] = shape->strokeDashArray[i];
	shape->orig.strokeDashCount = shape->strokeDashCount;

	// Add to tail
	if (p->image->shapes == NULL) {
		p->image->shapes = shapeNode;
	} else {
		p->shapesTail->next = shapeNode;
		shapeNode->prev = p->shapesTail;
	}
	p->shapesTail = shapeNode;

	return;

error:
	if (shapeNode) nsvg__free(p->image, shapeNode, sizeof(NSVGshapeNode));
	if (shape) nsvg__free(p->image, shape, sizeof(NSVGshape));
}

static void nsvg__transformPath(NSVGpath* path, float* xform)
{
	float bounds[4];
	float* curve;
	int i;

	// Transform path.
	for (i = 0; i < path->npts; ++i)
		nsvg__xformPoint(&path->pts[i*2], &path->pts[i*2+1], path->orig.pts[i*2], path->orig.pts[i*2+1], xform);

	// Find bounds
	for (i = 0; i < path->npts-1; i += 3) {
		curve = &path->pts[i*2];
		nsvg__curveBounds(bounds, curve);
		if (i == 0) {
			path->bounds[0] = bounds[0];
			path->bounds[1] = bounds[1];
			path->bounds[2] = bounds[2];
			path->bounds[3] = bounds[3];
		} else {
			path->bounds[0] = nsvg__minf(path->bounds[0], bounds[0]);
			path->bounds[1] = nsvg__minf(path->bounds[1], bounds[1]);
			path->bounds[2] = nsvg__maxf(path->bounds[2], bounds[2]);
			path->bounds[3] = nsvg__maxf(path->bounds[3], bounds[3]);
		}
	}
}

static void nsvg__addPath(NSVGparser* p, char closed)
{
	NSVGattrib* attr = nsvg__getAttr(p);
	NSVGpath* path = NULL;
	float bounds[4];
	float* curve;
	int i;

	if (p->npts < 4)
		return;

	if (closed)
		nsvg__lineTo(p, p->pts[0], p->pts[1]);

	// Expect 1 + N*3 points (N = number of cubic bezier segments).
	if ((p->npts % 3) != 1)
		return;

	path = (NSVGpath*)nsvg__malloc(p->image, sizeof(NSVGpath));
	if (path == NULL) goto error;
	memset(path, 0, sizeof(NSVGpath));

	path->pts = (float*)nsvg__malloc(p->image, p->npts*2*sizeof(float));
	if (path->pts == NULL) goto error;
	path->closed = closed;
	path->npts = p->npts;

	memcpy(path->pts, p->pts, p->npts*2*sizeof(float));
	memcpy(path->xform, attr->xform, sizeof(path->xform));

	// Store original values for animation.
	path->orig.pts = (float*)nsvg__malloc(p->image, path->npts*2*sizeof(float));
	if (path->orig.pts == NULL) goto error;
	memcpy(path->orig.pts, path->pts, path->npts*2*sizeof(float));
	memcpy(path->orig.xform, path->xform, sizeof(float)*6);

	nsvg__transformPath(path, path->xform);

	path->next = p->plist;
	p->plist = path;

	return;

error:
	if (path != NULL) {
		if (path->pts != NULL) nsvg__free(p->image, path->pts, p->npts*2*sizeof(float));
		if (path->orig.pts != NULL) nsvg__free(p->image, path->orig.pts, p->npts*2*sizeof(float));
		nsvg__free(p->image, path, sizeof(NSVGpath));
	}
}

// We roll our own string to float because the std library one uses locale and messes things up.
static double nsvg__atof(const char* s)
{
	char* cur = (char*)s;
	char* end = NULL;
	double res = 0.0, sign = 1.0;
	long long intPart = 0, fracPart = 0;
	char hasIntPart = 0, hasFracPart = 0;

	// Parse optional sign
	if (*cur == '+') {
		cur++;
	} else if (*cur == '-') {
		sign = -1;
		cur++;
	}

	// Parse integer part
	if (nsvg__isdigit(*cur)) {
		// Parse digit sequence
		intPart = strtoll(cur, &end, 10);
		if (cur != end) {
			res = (double)intPart;
			hasIntPart = 1;
			cur = end;
		}
	}

	// Parse fractional part.
	if (*cur == '.') {
		cur++; // Skip '.'
		if (nsvg__isdigit(*cur)) {
			// Parse digit sequence
			fracPart = strtoll(cur, &end, 10);
			if (cur != end) {
				res += (double)fracPart / pow(10.0, (double)(end - cur));
				hasFracPart = 1;
				cur = end;
			}
		}
	}

	// A valid number should have integer or fractional part.
	if (!hasIntPart && !hasFracPart)
		return 0.0;

	// Parse optional exponent
	if (*cur == 'e' || *cur == 'E') {
		long expPart = 0;
		cur++; // skip 'E'
		expPart = strtol(cur, &end, 10); // Parse digit sequence with sign
		if (cur != end) {
			res *= pow(10.0, (double)expPart);
		}
	}

	return res * sign;
}


static const char* nsvg__parseNumber(const char* s, char* it, const int size)
{
	const int last = size-1;
	int i = 0;

	// sign
	if (*s == '-' || *s == '+') {
		if (i < last) it[i++] = *s;
		s++;
	}
	// integer part
	while (*s && nsvg__isdigit(*s)) {
		if (i < last) it[i++] = *s;
		s++;
	}
	if (*s == '.') {
		// decimal point
		if (i < last) it[i++] = *s;
		s++;
		// fraction part
		while (*s && nsvg__isdigit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	// exponent
	if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x')) {
		if (i < last) it[i++] = *s;
		s++;
		if (*s == '-' || *s == '+') {
			if (i < last) it[i++] = *s;
			s++;
		}
		while (*s && nsvg__isdigit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	it[i] = '\0';

	return s;
}

static const char* nsvg__getNextPathItemWhenArcFlag(const char* s, int sLen, char* it)
{
	it[0] = '\0';
	for (; *s && sLen > 0 && (nsvg__isspace(*s) || *s == ','); s++, sLen--);
	if (!*s || sLen <= 0) return s;
	if (*s == '0' || *s == '1') {
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}
	return s;
}

static const char* nsvg__getNextPathItem(const char* s, int sLen, char* it)
{
	int len;
	it[0] = '\0';
	// Skip white spaces and commas
	for (; *s && sLen > 0 && (nsvg__isspace(*s) || *s == ','); s++, sLen--);
	if (!*s || sLen <= 0) return s;
	if (*s == '-' || *s == '+' || *s == '.' || nsvg__isdigit(*s)) {
		len = sLen+1 < 64 ? sLen+1 : 64;
		s = nsvg__parseNumber(s, it, len);
	} else {
		// Parse command
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}

	return s;
}

static unsigned int nsvg__parseColorHex(const char* str)
{
	unsigned int r=0, g=0, b=0;
	if (sscanf(str, "#%2x%2x%2x", &r, &g, &b) == 3 )		// 2 digit hex
		return NSVG_RGB(r, g, b);
	if (sscanf(str, "#%1x%1x%1x", &r, &g, &b) == 3 )		// 1 digit hex, e.g. #abc -> 0xccbbaa
		return NSVG_RGB(r*17, g*17, b*17);			// same effect as (r<<4|r), (g<<4|g), ..
	return NSVG_RGB(128, 128, 128);
}

// Parse rgb color. The pointer 'str' must point at "rgb(" (4+ characters).
// This function returns gray (rgb(128, 128, 128) == '#808080') on parse errors
// for backwards compatibility. Note: other image viewers return black instead.

static unsigned int nsvg__parseColorRGB(const char* str)
{
	int i;
	unsigned int rgbi[3];
	float rgbf[3];
	// try decimal integers first
	if (sscanf(str, "rgb(%u, %u, %u)", &rgbi[0], &rgbi[1], &rgbi[2]) != 3) {
		// integers failed, try percent values (float, locale independent)
		const char delimiter[3] = {',', ',', ')'};
		str += 4; // skip "rgb("
		for (i = 0; i < 3; i++) {
			while (*str && (nsvg__isspace(*str))) str++; 	// skip leading spaces
			if (*str == '+') str++;				// skip '+' (don't allow '-')
			if (!*str) break;
			rgbf[i] = nsvg__atof(str);

			// Note 1: it would be great if nsvg__atof() returned how many
			// bytes it consumed but it doesn't. We need to skip the number,
			// the '%' character, spaces, and the delimiter ',' or ')'.

			// Note 2: The following code does not allow values like "33.%",
			// i.e. a decimal point w/o fractional part, but this is consistent
			// with other image viewers, e.g. firefox, chrome, eog, gimp.

			while (*str && nsvg__isdigit(*str)) str++;		// skip integer part
			if (*str == '.') {
				str++;
				if (!nsvg__isdigit(*str)) break;		// error: no digit after '.'
				while (*str && nsvg__isdigit(*str)) str++;	// skip fractional part
			}
			if (*str == '%') str++; else break;
			while (*str && nsvg__isspace(*str)) str++;
			if (*str == delimiter[i]) str++;
			else break;
		}
		if (i == 3) {
			rgbi[0] = roundf(rgbf[0] * 2.55f);
			rgbi[1] = roundf(rgbf[1] * 2.55f);
			rgbi[2] = roundf(rgbf[2] * 2.55f);
		} else {
			rgbi[0] = rgbi[1] = rgbi[2] = 128;
		}
	}
	// clip values as the CSS spec requires
	for (i = 0; i < 3; i++) {
		if (rgbi[i] > 255) rgbi[i] = 255;
	}
	return NSVG_RGB(rgbi[0], rgbi[1], rgbi[2]);
}

typedef struct NSVGNamedColor {
	const char* name;
	unsigned int color;
} NSVGNamedColor;

NSVGNamedColor nsvg__colors[] = {

	{ "red", NSVG_RGB(255, 0, 0) },
	{ "green", NSVG_RGB( 0, 128, 0) },
	{ "blue", NSVG_RGB( 0, 0, 255) },
	{ "yellow", NSVG_RGB(255, 255, 0) },
	{ "cyan", NSVG_RGB( 0, 255, 255) },
	{ "magenta", NSVG_RGB(255, 0, 255) },
	{ "black", NSVG_RGB( 0, 0, 0) },
	{ "grey", NSVG_RGB(128, 128, 128) },
	{ "gray", NSVG_RGB(128, 128, 128) },
	{ "white", NSVG_RGB(255, 255, 255) },

#ifdef NANOSVG_ALL_COLOR_KEYWORDS
	{ "aliceblue", NSVG_RGB(240, 248, 255) },
	{ "antiquewhite", NSVG_RGB(250, 235, 215) },
	{ "aqua", NSVG_RGB( 0, 255, 255) },
	{ "aquamarine", NSVG_RGB(127, 255, 212) },
	{ "azure", NSVG_RGB(240, 255, 255) },
	{ "beige", NSVG_RGB(245, 245, 220) },
	{ "bisque", NSVG_RGB(255, 228, 196) },
	{ "blanchedalmond", NSVG_RGB(255, 235, 205) },
	{ "blueviolet", NSVG_RGB(138, 43, 226) },
	{ "brown", NSVG_RGB(165, 42, 42) },
	{ "burlywood", NSVG_RGB(222, 184, 135) },
	{ "cadetblue", NSVG_RGB( 95, 158, 160) },
	{ "chartreuse", NSVG_RGB(127, 255, 0) },
	{ "chocolate", NSVG_RGB(210, 105, 30) },
	{ "coral", NSVG_RGB(255, 127, 80) },
	{ "cornflowerblue", NSVG_RGB(100, 149, 237) },
	{ "cornsilk", NSVG_RGB(255, 248, 220) },
	{ "crimson", NSVG_RGB(220, 20, 60) },
	{ "darkblue", NSVG_RGB( 0, 0, 139) },
	{ "darkcyan", NSVG_RGB( 0, 139, 139) },
	{ "darkgoldenrod", NSVG_RGB(184, 134, 11) },
	{ "darkgray", NSVG_RGB(169, 169, 169) },
	{ "darkgreen", NSVG_RGB( 0, 100, 0) },
	{ "darkgrey", NSVG_RGB(169, 169, 169) },
	{ "darkkhaki", NSVG_RGB(189, 183, 107) },
	{ "darkmagenta", NSVG_RGB(139, 0, 139) },
	{ "darkolivegreen", NSVG_RGB( 85, 107, 47) },
	{ "darkorange", NSVG_RGB(255, 140, 0) },
	{ "darkorchid", NSVG_RGB(153, 50, 204) },
	{ "darkred", NSVG_RGB(139, 0, 0) },
	{ "darksalmon", NSVG_RGB(233, 150, 122) },
	{ "darkseagreen", NSVG_RGB(143, 188, 143) },
	{ "darkslateblue", NSVG_RGB( 72, 61, 139) },
	{ "darkslategray", NSVG_RGB( 47, 79, 79) },
	{ "darkslategrey", NSVG_RGB( 47, 79, 79) },
	{ "darkturquoise", NSVG_RGB( 0, 206, 209) },
	{ "darkviolet", NSVG_RGB(148, 0, 211) },
	{ "deeppink", NSVG_RGB(255, 20, 147) },
	{ "deepskyblue", NSVG_RGB( 0, 191, 255) },
	{ "dimgray", NSVG_RGB(105, 105, 105) },
	{ "dimgrey", NSVG_RGB(105, 105, 105) },
	{ "dodgerblue", NSVG_RGB( 30, 144, 255) },
	{ "firebrick", NSVG_RGB(178, 34, 34) },
	{ "floralwhite", NSVG_RGB(255, 250, 240) },
	{ "forestgreen", NSVG_RGB( 34, 139, 34) },
	{ "fuchsia", NSVG_RGB(255, 0, 255) },
	{ "gainsboro", NSVG_RGB(220, 220, 220) },
	{ "ghostwhite", NSVG_RGB(248, 248, 255) },
	{ "gold", NSVG_RGB(255, 215, 0) },
	{ "goldenrod", NSVG_RGB(218, 165, 32) },
	{ "greenyellow", NSVG_RGB(173, 255, 47) },
	{ "honeydew", NSVG_RGB(240, 255, 240) },
	{ "hotpink", NSVG_RGB(255, 105, 180) },
	{ "indianred", NSVG_RGB(205, 92, 92) },
	{ "indigo", NSVG_RGB( 75, 0, 130) },
	{ "ivory", NSVG_RGB(255, 255, 240) },
	{ "khaki", NSVG_RGB(240, 230, 140) },
	{ "lavender", NSVG_RGB(230, 230, 250) },
	{ "lavenderblush", NSVG_RGB(255, 240, 245) },
	{ "lawngreen", NSVG_RGB(124, 252, 0) },
	{ "lemonchiffon", NSVG_RGB(255, 250, 205) },
	{ "lightblue", NSVG_RGB(173, 216, 230) },
	{ "lightcoral", NSVG_RGB(240, 128, 128) },
	{ "lightcyan", NSVG_RGB(224, 255, 255) },
	{ "lightgoldenrodyellow", NSVG_RGB(250, 250, 210) },
	{ "lightgray", NSVG_RGB(211, 211, 211) },
	{ "lightgreen", NSVG_RGB(144, 238, 144) },
	{ "lightgrey", NSVG_RGB(211, 211, 211) },
	{ "lightpink", NSVG_RGB(255, 182, 193) },
	{ "lightsalmon", NSVG_RGB(255, 160, 122) },
	{ "lightseagreen", NSVG_RGB( 32, 178, 170) },
	{ "lightskyblue", NSVG_RGB(135, 206, 250) },
	{ "lightslategray", NSVG_RGB(119, 136, 153) },
	{ "lightslategrey", NSVG_RGB(119, 136, 153) },
	{ "lightsteelblue", NSVG_RGB(176, 196, 222) },
	{ "lightyellow", NSVG_RGB(255, 255, 224) },
	{ "lime", NSVG_RGB( 0, 255, 0) },
	{ "limegreen", NSVG_RGB( 50, 205, 50) },
	{ "linen", NSVG_RGB(250, 240, 230) },
	{ "maroon", NSVG_RGB(128, 0, 0) },
	{ "mediumaquamarine", NSVG_RGB(102, 205, 170) },
	{ "mediumblue", NSVG_RGB( 0, 0, 205) },
	{ "mediumorchid", NSVG_RGB(186, 85, 211) },
	{ "mediumpurple", NSVG_RGB(147, 112, 219) },
	{ "mediumseagreen", NSVG_RGB( 60, 179, 113) },
	{ "mediumslateblue", NSVG_RGB(123, 104, 238) },
	{ "mediumspringgreen", NSVG_RGB( 0, 250, 154) },
	{ "mediumturquoise", NSVG_RGB( 72, 209, 204) },
	{ "mediumvioletred", NSVG_RGB(199, 21, 133) },
	{ "midnightblue", NSVG_RGB( 25, 25, 112) },
	{ "mintcream", NSVG_RGB(245, 255, 250) },
	{ "mistyrose", NSVG_RGB(255, 228, 225) },
	{ "moccasin", NSVG_RGB(255, 228, 181) },
	{ "navajowhite", NSVG_RGB(255, 222, 173) },
	{ "navy", NSVG_RGB( 0, 0, 128) },
	{ "oldlace", NSVG_RGB(253, 245, 230) },
	{ "olive", NSVG_RGB(128, 128, 0) },
	{ "olivedrab", NSVG_RGB(107, 142, 35) },
	{ "orange", NSVG_RGB(255, 165, 0) },
	{ "orangered", NSVG_RGB(255, 69, 0) },
	{ "orchid", NSVG_RGB(218, 112, 214) },
	{ "palegoldenrod", NSVG_RGB(238, 232, 170) },
	{ "palegreen", NSVG_RGB(152, 251, 152) },
	{ "paleturquoise", NSVG_RGB(175, 238, 238) },
	{ "palevioletred", NSVG_RGB(219, 112, 147) },
	{ "papayawhip", NSVG_RGB(255, 239, 213) },
	{ "peachpuff", NSVG_RGB(255, 218, 185) },
	{ "peru", NSVG_RGB(205, 133, 63) },
	{ "pink", NSVG_RGB(255, 192, 203) },
	{ "plum", NSVG_RGB(221, 160, 221) },
	{ "powderblue", NSVG_RGB(176, 224, 230) },
	{ "purple", NSVG_RGB(128, 0, 128) },
	{ "rosybrown", NSVG_RGB(188, 143, 143) },
	{ "royalblue", NSVG_RGB( 65, 105, 225) },
	{ "saddlebrown", NSVG_RGB(139, 69, 19) },
	{ "salmon", NSVG_RGB(250, 128, 114) },
	{ "sandybrown", NSVG_RGB(244, 164, 96) },
	{ "seagreen", NSVG_RGB( 46, 139, 87) },
	{ "seashell", NSVG_RGB(255, 245, 238) },
	{ "sienna", NSVG_RGB(160, 82, 45) },
	{ "silver", NSVG_RGB(192, 192, 192) },
	{ "skyblue", NSVG_RGB(135, 206, 235) },
	{ "slateblue", NSVG_RGB(106, 90, 205) },
	{ "slategray", NSVG_RGB(112, 128, 144) },
	{ "slategrey", NSVG_RGB(112, 128, 144) },
	{ "snow", NSVG_RGB(255, 250, 250) },
	{ "springgreen", NSVG_RGB( 0, 255, 127) },
	{ "steelblue", NSVG_RGB( 70, 130, 180) },
	{ "tan", NSVG_RGB(210, 180, 140) },
	{ "teal", NSVG_RGB( 0, 128, 128) },
	{ "thistle", NSVG_RGB(216, 191, 216) },
	{ "tomato", NSVG_RGB(255, 99, 71) },
	{ "turquoise", NSVG_RGB( 64, 224, 208) },
	{ "violet", NSVG_RGB(238, 130, 238) },
	{ "wheat", NSVG_RGB(245, 222, 179) },
	{ "whitesmoke", NSVG_RGB(245, 245, 245) },
	{ "yellowgreen", NSVG_RGB(154, 205, 50) },
#endif
};

static unsigned int nsvg__parseColorName(const char* str, int len)
{
	int i, ncolors = sizeof(nsvg__colors) / sizeof(NSVGNamedColor);

	for (i = 0; i < ncolors; i++) {
		if (strncmp(nsvg__colors[i].name, str, len) == 0) {
			return nsvg__colors[i].color;
		}
	}

	return NSVG_RGB(128, 128, 128);
}

static unsigned int nsvg__parseColor(const char* str, int len)
{
	for (; *str == ' '; ++str, len--);
	if (len >= 1 && *str == '#')
		return nsvg__parseColorHex(str);
	else if (len >= 4 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == '(')
		return nsvg__parseColorRGB(str);
	return nsvg__parseColorName(str, len);
}

static float nsvg__parseOpacity(const char* str)
{
	float val = nsvg__atof(str);
	if (val < 0.0f) val = 0.0f;
	if (val > 1.0f) val = 1.0f;
	return val;
}

static float nsvg__parseMiterLimit(const char* str)
{
	float val = nsvg__atof(str);
	if (val < 0.0f) val = 0.0f;
	return val;
}

static int nsvg__parseUnits(const char* units)
{
	if (units[0] == 'p' && units[1] == 'x')
		return NSVG_UNITS_PX;
	else if (units[0] == 'p' && units[1] == 't')
		return NSVG_UNITS_PT;
	else if (units[0] == 'p' && units[1] == 'c')
		return NSVG_UNITS_PC;
	else if (units[0] == 'm' && units[1] == 'm')
		return NSVG_UNITS_MM;
	else if (units[0] == 'c' && units[1] == 'm')
		return NSVG_UNITS_CM;
	else if (units[0] == 'i' && units[1] == 'n')
		return NSVG_UNITS_IN;
	else if (units[0] == '%')
		return NSVG_UNITS_PERCENT;
	else if (units[0] == 'e' && units[1] == 'm')
		return NSVG_UNITS_EM;
	else if (units[0] == 'e' && units[1] == 'x')
		return NSVG_UNITS_EX;
	return NSVG_UNITS_USER;
}

static int nsvg__isCoordinate(const char* s)
{
	// optional sign
	if (*s == '-' || *s == '+')
		s++;
	// must have at least one digit, or start by a dot
	return (nsvg__isdigit(*s) || *s == '.');
}

static NSVGcoordinate nsvg__parseCoordinateRaw(const char* str, int strLen)
{
	NSVGcoordinate coord = {0, NSVG_UNITS_USER};
	char buf[64];
	int len = strLen+1 < 64 ? strLen+1 : 64;
	coord.units = nsvg__parseUnits(nsvg__parseNumber(str, buf, len));
	coord.value = nsvg__atof(buf);
	return coord;
}

static NSVGcoordinate nsvg__coord(float v, int units)
{
	NSVGcoordinate coord = {v, units};
	return coord;
}

static float nsvg__parseCoordinate(NSVGparser* p, const char* str, int strLen, float orig, float length)
{
	NSVGcoordinate coord = nsvg__parseCoordinateRaw(str, strLen);
	return nsvg__convertToPixels(p->image, coord, orig, length);
}

static int nsvg__parseTransformArgs(const char* str, int strLen, float* args, int maxNa, int* na, char hasParens)
{
	const char* end;
	const char* ptr;
	char it[64];
	int len;

	*na = 0;
	ptr = str;
	if (hasParens) {
		for (; *ptr && strLen > 0 && *ptr != '('; ++ptr, strLen--);
	}
	if (!*ptr || strLen <= 0)
		return 1;
	end = ptr;
	if (hasParens) {
		for (; *end && strLen > 0 && *end != ')'; ++end, strLen--);
		if (*end == 0 || strLen <= 0)
			return 1;
	} else {
		for (; *end && strLen > 0 && *end != ';'; ++end, strLen--);
	}

	while (ptr < end) {
		if (*ptr == '-' || *ptr == '+' || *ptr == '.' || nsvg__isdigit(*ptr)) {
			if (*na >= maxNa) return 0;
			len = end - ptr + 1 < 64 ? end - ptr + 1 : 64;
			ptr = nsvg__parseNumber(ptr, it, len);
			args[(*na)++] = (float)nsvg__atof(it);
		} else {
			++ptr;
		}
	}
	return (int)(end - str);
}


static int nsvg__parseMatrix(float* xform, const char* str, int strLen)
{
	float t[6];
	int na = 0;
	int len = nsvg__parseTransformArgs(str, strLen, t, 6, &na, 1);
	if (na != 6) return len;
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseTranslate(float* xform, const char* str, int strLen)
{
	float args[2];
	float t[6];
	int na = 0;
	int len = nsvg__parseTransformArgs(str, strLen, args, 2, &na, 1);
	if (na == 1) args[1] = 0.0;

	nsvg__xformSetTranslation(t, args[0], args[1]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseScale(float* xform, const char* str, int strLen)
{
	float args[2];
	int na = 0;
	float t[6];
	int len = nsvg__parseTransformArgs(str, strLen, args, 2, &na, 1);
	if (na == 1) args[1] = args[0];
	nsvg__xformSetScale(t, args[0], args[1]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseSkewX(float* xform, const char* str, int strLen)
{
	float args[1];
	int na = 0;
	float t[6];
	int len = nsvg__parseTransformArgs(str, strLen, args, 1, &na, 1);
	nsvg__xformSetSkewX(t, args[0]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseSkewY(float* xform, const char* str, int strLen)
{
	float args[1];
	int na = 0;
	float t[6];
	int len = nsvg__parseTransformArgs(str, strLen, args, 1, &na, 1);
	nsvg__xformSetSkewY(t, args[0]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int nsvg__parseRotate(float* xform, const char* str, int strLen)
{
	float args[3];
	int na = 0;
	float t[6];
	int len = nsvg__parseTransformArgs(str, strLen, args, 3, &na, 1);
	if (na == 1)
		args[1] = args[2] = 0.0f;

	if (na > 1) {
		nsvg__xformSetNonCenterRotation(t, args[0], args[1], args[2]);
	} else {
		nsvg__xformSetRotation(t, args[0]);
	}
	memcpy(xform, t, sizeof(float)*6);

	return len;
}

static void nsvg__parseTransform(float* xform, const char* str, int strLen)
{
	float t[6];
	int len;
	nsvg__xformIdentity(xform);
	while (*str && strLen > 0)
	{
		if (strLen >= 6 && strncmp(str, "matrix", 6) == 0)
			len = nsvg__parseMatrix(t, str, strLen);
		else if (strLen >= 9 && strncmp(str, "translate", 9) == 0)
			len = nsvg__parseTranslate(t, str, strLen);
		else if (strLen >= 5 && strncmp(str, "scale", 5) == 0)
			len = nsvg__parseScale(t, str, strLen);
		else if (strLen >= 6 && strncmp(str, "rotate", 6) == 0)
			len = nsvg__parseRotate(t, str, strLen);
		else if (strLen >= 5 && strncmp(str, "skewX", 5) == 0)
			len = nsvg__parseSkewX(t, str, strLen);
		else if (strLen >= 5 && strncmp(str, "skewY", 5) == 0)
			len = nsvg__parseSkewY(t, str, strLen);
		else{
			++str;
			strLen--;
			continue;
		}
		if (len != 0) {
			str += len;
			strLen -= len;
		} else {
			++str;
			strLen--;
			continue;
		}

		nsvg__xformPremultiply(xform, t);
	}
}

static void nsvg__parseUrl(char* id, const char* str, int strLen)
{
	int i = 0;
	str += 4; // "url(";
	if (*str && strLen > 0 && *str == '#') {
		str++;
		strLen--;
	}
	while (i < 63 && *str && strLen > 0 && *str != ')') {
		id[i] = *str++;
		strLen--;
		i++;
	}
	id[i] = '\0';
}

static char nsvg__parseLineCap(const char* str, int strLen)
{
	if (strncmp(str, "butt", strLen) == 0)
		return NSVG_CAP_BUTT;
	else if (strncmp(str, "round", strLen) == 0)
		return NSVG_CAP_ROUND;
	else if (strncmp(str, "square", strLen) == 0)
		return NSVG_CAP_SQUARE;
	// TODO: handle inherit.
	return NSVG_CAP_BUTT;
}

static char nsvg__parseLineJoin(const char* str, int strLen)
{
	if (strncmp(str, "miter", strLen) == 0)
		return NSVG_JOIN_MITER;
	else if (strncmp(str, "round", strLen) == 0)
		return NSVG_JOIN_ROUND;
	else if (strncmp(str, "bevel", strLen) == 0)
		return NSVG_JOIN_BEVEL;
	// TODO: handle inherit.
	return NSVG_JOIN_MITER;
}

static char nsvg__parseFillRule(const char* str, int strLen)
{
	if (strncmp(str, "nonzero", strLen) == 0)
		return NSVG_FILLRULE_NONZERO;
	else if (strncmp(str, "evenodd", strLen) == 0)
		return NSVG_FILLRULE_EVENODD;
	// TODO: handle inherit.
	return NSVG_FILLRULE_NONZERO;
}

static const char* nsvg__getNextDashItem(const char* s, int sLen, char* it)
{
	int n = 0;
	it[0] = '\0';
	// Skip white spaces and commas
	for (; *s && sLen > 0 && (nsvg__isspace(*s) || *s == ','); s++, sLen--);
	// Advance until whitespace, comma or end.
	while (*s && sLen > 0 && (!nsvg__isspace(*s) && *s != ',' && *s != ';')) {
		if (n < 63)
			it[n++] = *s;
		s++;
		sLen--;
	}
	it[n++] = '\0';
	return s;
}

static int nsvg__parseStrokeDashArray(NSVGparser* p, const char* str, int strLen, float* strokeDashArray)
{
	char item[64];
	int count = 0, i;
	float sum = 0.0f;
	const char* str2;

	// Handle "none"
	if (str[0] == 'n')
		return 0;

	// Parse dashes
	while (*str && strLen > 0) {
		str2 = nsvg__getNextDashItem(str, strLen, item);
		strLen -= str2 - str;
		str = str2;

		if (!*item) break;
		if (count < NSVG_MAX_DASHES)
			strokeDashArray[count++] = fabsf(nsvg__parseCoordinate(p, item, 64, 0.0f, nsvg__actualLength(p)));
	}

	for (i = 0; i < count; i++)
		sum += strokeDashArray[i];
	if (sum <= 1e-6f)
		count = 0;

	return count;
}

static void nsvg__parseStyle(NSVGparser* p, const char* str, int strLen);

static int nsvg__parseAttr(NSVGparser* p, const char* name, int nameLen, const char* value, int valueLen)
{
	float xform[6];
	int len;
	NSVGattrib* attr = nsvg__getAttr(p);
	if (!attr) return 0;

	if (strncmp(name, "style", nameLen) == 0) {
		nsvg__parseStyle(p, value, valueLen);
	} else if (strncmp(name, "display", nameLen) == 0) {
		if (strncmp(value, "none", valueLen) == 0)
			attr->visible = 0;
		// Don't reset ->visible on display:inline, one display:none hides the whole subtree

	} else if (strncmp(name, "fill", nameLen) == 0) {
		if (strncmp(value, "none", valueLen) == 0 || strncmp(value, "transparent", valueLen) == 0) {
			attr->hasFill = 0;
		} else if (strncmp(value, "url(", 4) == 0) {
			attr->hasFill = 2;
			if (attr->fillGradient == NULL) attr->fillGradient = nsvg__allocId(p);
			if (attr->fillGradient == NULL) return 0;
			nsvg__parseUrl(attr->fillGradient->id, value, valueLen);
		} else {
			attr->hasFill = 1;
			attr->fillColor = nsvg__parseColor(value, valueLen);
		}
	} else if (strncmp(name, "opacity", nameLen) == 0) {
		attr->opacity = nsvg__parseOpacity(value);
	} else if (strncmp(name, "fill-opacity", nameLen) == 0) {
		attr->fillOpacity = nsvg__parseOpacity(value);
	} else if (strncmp(name, "stroke", nameLen) == 0) {
		if (strncmp(value, "none", valueLen) == 0) {
			attr->hasStroke = 0;
		} else if (strncmp(value, "url(", 4) == 0) {
			attr->hasStroke = 2;
			if (attr->strokeGradient == NULL) attr->strokeGradient = nsvg__allocId(p);
			if (attr->strokeGradient == NULL) return 0;
			nsvg__parseUrl(attr->strokeGradient->id, value, valueLen);
		} else {
			attr->hasStroke = 1;
			attr->strokeColor = nsvg__parseColor(value, valueLen);
		}
	} else if (strncmp(name, "stroke-width", nameLen) == 0) {
		attr->strokeWidth = nsvg__parseCoordinate(p, value, valueLen, 0.0f, nsvg__actualLength(p));
	} else if (strncmp(name, "stroke-dasharray", nameLen) == 0) {
		attr->strokeDashCount = nsvg__parseStrokeDashArray(p, value, valueLen, attr->strokeDashArray);
	} else if (strncmp(name, "stroke-dashoffset", nameLen) == 0) {
		attr->strokeDashOffset = nsvg__parseCoordinate(p, value, valueLen, 0.0f, nsvg__actualLength(p));
	} else if (strncmp(name, "stroke-opacity", nameLen) == 0) {
		attr->strokeOpacity = nsvg__parseOpacity(value);
	} else if (strncmp(name, "stroke-linecap", nameLen) == 0) {
		attr->strokeLineCap = nsvg__parseLineCap(value, valueLen);
	} else if (strncmp(name, "stroke-linejoin", nameLen) == 0) {
		attr->strokeLineJoin = nsvg__parseLineJoin(value, valueLen);
	} else if (strncmp(name, "stroke-miterlimit", nameLen) == 0) {
		attr->miterLimit = nsvg__parseMiterLimit(value);
	} else if (strncmp(name, "fill-rule", nameLen) == 0) {
		attr->fillRule = nsvg__parseFillRule(value, valueLen);
	} else if (strncmp(name, "font-size", nameLen) == 0) {
		attr->fontSize = nsvg__parseCoordinate(p, value, valueLen, 0.0f, nsvg__actualLength(p));
	} else if (strncmp(name, "transform", nameLen) == 0) {
		nsvg__parseTransform(xform, value, valueLen);
		nsvg__xformPremultiply(attr->xform, xform);
	} else if (strncmp(name, "stop-color", nameLen) == 0) {
		attr->stopColor = nsvg__parseColor(value, valueLen);
	} else if (strncmp(name, "stop-opacity", nameLen) == 0) {
		attr->stopOpacity = nsvg__parseOpacity(value);
	} else if (strncmp(name, "offset", nameLen) == 0) {
		attr->stopOffset = nsvg__parseCoordinate(p, value, valueLen, 0.0f, 1.0f);
	} else if (strncmp(name, "id", nameLen) == 0) {
		if (attr->id == NULL) attr->id = nsvg__allocId(p);
		if (attr->id == NULL) return 0;
		len = valueLen < 63 ? valueLen : 63;
		strncpy(attr->id->id, value, len);
		attr->id->id[len] = '\0';
	} else {
		return 0;
	}
	return 1;
}

static int nsvg__parseNameValue(NSVGparser* p, const char* start, const char* end)
{
	const char* str;
	const char* name;
	const char* value;
	int nameLen;
	int valueLen;
	int n;

	str = start;
	while (str < end && *str != ':') ++str;

	value = str;
	while (value < end && (*value == ':' || nsvg__isspace(*value))) ++value;
	valueLen = end - value;

	// Right Trim
	while (str > start && (*str == ':' || nsvg__isspace(*str))) --str;
	++str;

	name = start;
	nameLen = str - start;

	return nsvg__parseAttr(p, name, nameLen, value, valueLen);
}

static void nsvg__parseStyle(NSVGparser* p, const char* str, int strLen)
{
	const char* start;
	const char* end;

	while (*str && strLen > 0) {
		// Left Trim
		for (; *str && strLen > 0 && nsvg__isspace(*str); ++str, strLen--);
		start = str;
		for (; *str && strLen > 0 && *str != ';'; ++str, strLen--);
		end = str;

		// Right Trim
		while (end > start &&  (*end == ';' || nsvg__isspace(*end))) --end;
		++end;

		nsvg__parseNameValue(p, start, end);
		if (*str) ++str;
	}
}

static void nsvg__parseAttribs(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	int i;
	for (i = 0; i < nattr; i++)
	{
		if (strncmp(attr[i].name, "style", attr[i].nameLen) == 0)
			nsvg__parseStyle(p, attr[i].value, attr[i].valueLen);
		else
			nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen);
	}
}

static int nsvg__getArgsPerElement(char cmd)
{
	switch (cmd) {
		case 'v':
		case 'V':
		case 'h':
		case 'H':
			return 1;
		case 'm':
		case 'M':
		case 'l':
		case 'L':
		case 't':
		case 'T':
			return 2;
		case 'q':
		case 'Q':
		case 's':
		case 'S':
			return 4;
		case 'c':
		case 'C':
			return 6;
		case 'a':
		case 'A':
			return 7;
		case 'z':
		case 'Z':
			return 0;
	}
	return -1;
}

static void nsvg__pathMoveTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	nsvg__moveTo(p, *cpx, *cpy);
}

static void nsvg__pathLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathHLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpx += args[0];
	else
		*cpx = args[0];
	nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathVLineTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpy += args[0];
	else
		*cpy = args[0];
	nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathCubicBezTo(NSVGparser* p, float* cpx, float* cpy,
								 float* cpx2, float* cpy2, float* args, int rel)
{
	float x2, y2, cx1, cy1, cx2, cy2;

	if (rel) {
		cx1 = *cpx + args[0];
		cy1 = *cpy + args[1];
		cx2 = *cpx + args[2];
		cy2 = *cpy + args[3];
		x2 = *cpx + args[4];
		y2 = *cpy + args[5];
	} else {
		cx1 = args[0];
		cy1 = args[1];
		cx2 = args[2];
		cy2 = args[3];
		x2 = args[4];
		y2 = args[5];
	}

	nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathCubicBezShortTo(NSVGparser* p, float* cpx, float* cpy,
									  float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx2 = *cpx + args[0];
		cy2 = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx2 = args[0];
		cy2 = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	cx1 = 2*x1 - *cpx2;
	cy1 = 2*y1 - *cpy2;

	nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathQuadBezTo(NSVGparser* p, float* cpx, float* cpy,
								float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx = *cpx + args[0];
		cy = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx = args[0];
		cy = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	// Convert to cubic bezier
	cx1 = x1 + 2.0f/3.0f*(cx - x1);
	cy1 = y1 + 2.0f/3.0f*(cy - y1);
	cx2 = x2 + 2.0f/3.0f*(cx - x2);
	cy2 = y2 + 2.0f/3.0f*(cy - y2);

	nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathQuadBezShortTo(NSVGparser* p, float* cpx, float* cpy,
									 float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		x2 = *cpx + args[0];
		y2 = *cpy + args[1];
	} else {
		x2 = args[0];
		y2 = args[1];
	}

	cx = 2*x1 - *cpx2;
	cy = 2*y1 - *cpy2;

	// Convert to cubix bezier
	cx1 = x1 + 2.0f/3.0f*(cx - x1);
	cy1 = y1 + 2.0f/3.0f*(cy - y1);
	cx2 = x2 + 2.0f/3.0f*(cx - x2);
	cy2 = y2 + 2.0f/3.0f*(cy - y2);

	nsvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static float nsvg__sqr(float x) { return x*x; }
static float nsvg__vmag(float x, float y) { return sqrtf(x*x + y*y); }

static float nsvg__vecrat(float ux, float uy, float vx, float vy)
{
	return (ux*vx + uy*vy) / (nsvg__vmag(ux,uy) * nsvg__vmag(vx,vy));
}

static float nsvg__vecang(float ux, float uy, float vx, float vy)
{
	float r = nsvg__vecrat(ux,uy, vx,vy);
	if (r < -1.0f) r = -1.0f;
	if (r > 1.0f) r = 1.0f;
	return ((ux*vy < uy*vx) ? -1.0f : 1.0f) * acosf(r);
}

static void nsvg__pathArcTo(NSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	// Ported from canvg (https://code.google.com/p/canvg/)
	float rx, ry, rotx;
	float x1, y1, x2, y2, cx, cy, dx, dy, d;
	float x1p, y1p, cxp, cyp, s, sa, sb;
	float ux, uy, vx, vy, a1, da;
	float x, y, tanx, tany, a, px = 0, py = 0, ptanx = 0, ptany = 0, t[6];
	float sinrx, cosrx;
	int fa, fs;
	int i, ndivs;
	float hda, kappa;

	rx = fabsf(args[0]);				// y radius
	ry = fabsf(args[1]);				// x radius
	rotx = args[2] / 180.0f * NSVG_PI;		// x rotation angle
	fa = fabsf(args[3]) > 1e-6 ? 1 : 0;	// Large arc
	fs = fabsf(args[4]) > 1e-6 ? 1 : 0;	// Sweep direction
	x1 = *cpx;							// start point
	y1 = *cpy;
	if (rel) {							// end point
		x2 = *cpx + args[5];
		y2 = *cpy + args[6];
	} else {
		x2 = args[5];
		y2 = args[6];
	}

	dx = x1 - x2;
	dy = y1 - y2;
	d = sqrtf(dx*dx + dy*dy);
	if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f) {
		// The arc degenerates to a line
		nsvg__lineTo(p, x2, y2);
		*cpx = x2;
		*cpy = y2;
		return;
	}

	sinrx = sinf(rotx);
	cosrx = cosf(rotx);

	// Convert to center point parameterization.
	// http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
	// 1) Compute x1', y1'
	x1p = cosrx * dx / 2.0f + sinrx * dy / 2.0f;
	y1p = -sinrx * dx / 2.0f + cosrx * dy / 2.0f;
	d = nsvg__sqr(x1p)/nsvg__sqr(rx) + nsvg__sqr(y1p)/nsvg__sqr(ry);
	if (d > 1) {
		d = sqrtf(d);
		rx *= d;
		ry *= d;
	}
	// 2) Compute cx', cy'
	s = 0.0f;
	sa = nsvg__sqr(rx)*nsvg__sqr(ry) - nsvg__sqr(rx)*nsvg__sqr(y1p) - nsvg__sqr(ry)*nsvg__sqr(x1p);
	sb = nsvg__sqr(rx)*nsvg__sqr(y1p) + nsvg__sqr(ry)*nsvg__sqr(x1p);
	if (sa < 0.0f) sa = 0.0f;
	if (sb > 0.0f)
		s = sqrtf(sa / sb);
	if (fa == fs)
		s = -s;
	cxp = s * rx * y1p / ry;
	cyp = s * -ry * x1p / rx;

	// 3) Compute cx,cy from cx',cy'
	cx = (x1 + x2)/2.0f + cosrx*cxp - sinrx*cyp;
	cy = (y1 + y2)/2.0f + sinrx*cxp + cosrx*cyp;

	// 4) Calculate theta1, and delta theta.
	ux = (x1p - cxp) / rx;
	uy = (y1p - cyp) / ry;
	vx = (-x1p - cxp) / rx;
	vy = (-y1p - cyp) / ry;
	a1 = nsvg__vecang(1.0f,0.0f, ux,uy);	// Initial angle
	da = nsvg__vecang(ux,uy, vx,vy);		// Delta angle

//	if (vecrat(ux,uy,vx,vy) <= -1.0f) da = NSVG_PI;
//	if (vecrat(ux,uy,vx,vy) >= 1.0f) da = 0;

	if (fs == 0 && da > 0)
		da -= 2 * NSVG_PI;
	else if (fs == 1 && da < 0)
		da += 2 * NSVG_PI;

	// Approximate the arc using cubic spline segments.
	t[0] = cosrx; t[1] = sinrx;
	t[2] = -sinrx; t[3] = cosrx;
	t[4] = cx; t[5] = cy;

	// Split arc into max 90 degree segments.
	// The loop assumes an iteration per end point (including start and end), this +1.
	ndivs = (int)(fabsf(da) / (NSVG_PI*0.5f) + 1.0f);
	hda = (da / (float)ndivs) / 2.0f;
	// Fix for ticket #179: division by 0: avoid cotangens around 0 (infinite)
	if ((hda < 1e-3f) && (hda > -1e-3f))
		hda *= 0.5f;
	else
		hda = (1.0f - cosf(hda)) / sinf(hda);
	kappa = fabsf(4.0f / 3.0f * hda);
	if (da < 0.0f)
		kappa = -kappa;

	for (i = 0; i <= ndivs; i++) {
		a = a1 + da * ((float)i/(float)ndivs);
		dx = cosf(a);
		dy = sinf(a);
		nsvg__xformPoint(&x, &y, dx*rx, dy*ry, t); // position
		nsvg__xformVec(&tanx, &tany, -dy*rx * kappa, dx*ry * kappa, t); // tangent
		if (i > 0)
			nsvg__cubicBezTo(p, px+ptanx,py+ptany, x-tanx, y-tany, x, y);
		px = x;
		py = y;
		ptanx = tanx;
		ptany = tany;
	}

	*cpx = x2;
	*cpy = y2;
}

static void nsvg__parsePath(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	const char* s = NULL;
	const char* s2 = NULL;
	int sLen = 0;
	char cmd = '\0';
	float args[10];
	int nargs;
	int rargs = 0;
	char initPoint;
	float cpx, cpy, cpx2, cpy2;
	char closedFlag;
	int i;
	char item[64];

	for (i = 0; i < nattr; i++) {
		if (strncmp(attr[i].name, "d", attr[i].nameLen) == 0) {
			s = attr[i].value;
			sLen = attr[i].valueLen;
		} else {
			nsvg__parseAttribs(p, &attr[i], 1);
		}
	}

	if (s && sLen > 0) {
		nsvg__resetPath(p);
		cpx = 0; cpy = 0;
		cpx2 = 0; cpy2 = 0;
		initPoint = 0;
		closedFlag = 0;
		nargs = 0;

		while (*s && sLen > 0) {
			item[0] = '\0';
			if ((cmd == 'A' || cmd == 'a') && (nargs == 3 || nargs == 4)) {
				s2 = nsvg__getNextPathItemWhenArcFlag(s, sLen, item);
				sLen -= s2 - s;
				s = s2;
			}
			if (!*item) {
				s2 = nsvg__getNextPathItem(s, sLen, item);
				sLen -= s2 - s;
				s = s2;
			}
			if (!*item) break;
			if (cmd != '\0' && nsvg__isCoordinate(item)) {
				if (nargs < 10)
					args[nargs++] = (float)nsvg__atof(item);
				if (nargs >= rargs) {
					switch (cmd) {
						case 'm':
						case 'M':
							nsvg__pathMoveTo(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
							// Moveto can be followed by multiple coordinate pairs,
							// which should be treated as linetos.
							cmd = (cmd == 'm') ? 'l' : 'L';
							rargs = nsvg__getArgsPerElement(cmd);
							cpx2 = cpx; cpy2 = cpy;
							initPoint = 1;
							break;
						case 'l':
						case 'L':
							nsvg__pathLineTo(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'H':
						case 'h':
							nsvg__pathHLineTo(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'V':
						case 'v':
							nsvg__pathVLineTo(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'C':
						case 'c':
							nsvg__pathCubicBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
							break;
						case 'S':
						case 's':
							nsvg__pathCubicBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
							break;
						case 'Q':
						case 'q':
							nsvg__pathQuadBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
							break;
						case 'T':
						case 't':
							nsvg__pathQuadBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 't' ? 1 : 0);
							break;
						case 'A':
						case 'a':
							nsvg__pathArcTo(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						default:
							if (nargs >= 2) {
								cpx = args[nargs-2];
								cpy = args[nargs-1];
								cpx2 = cpx; cpy2 = cpy;
							}
							break;
					}
					nargs = 0;
				}
			} else {
				cmd = item[0];
				if (cmd == 'M' || cmd == 'm') {
					// Commit path.
					if (p->npts > 0)
						nsvg__addPath(p, closedFlag);
					// Start new subpath.
					nsvg__resetPath(p);
					closedFlag = 0;
					nargs = 0;
				} else if (initPoint == 0) {
					// Do not allow other commands until initial point has been set (moveTo called once).
					cmd = '\0';
				}
				if (cmd == 'Z' || cmd == 'z') {
					closedFlag = 1;
					// Commit path.
					if (p->npts > 0) {
						// Move current point to first point
						cpx = p->pts[0];
						cpy = p->pts[1];
						cpx2 = cpx; cpy2 = cpy;
						nsvg__addPath(p, closedFlag);
					}
					// Start new subpath.
					nsvg__resetPath(p);
					nsvg__moveTo(p, cpx, cpy);
					closedFlag = 0;
					nargs = 0;
				}
				rargs = nsvg__getArgsPerElement(cmd);
				if (rargs == -1) {
					// Command not recognized
					cmd = '\0';
					rargs = 0;
				}
			}
		}
		// Commit path.
		if (p->npts)
			nsvg__addPath(p, closedFlag);
	}

	nsvg__addShape(p);
}

static void nsvg__parseRect(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;
	float rx = -1.0f; // marks not set
	float ry = -1.0f;
	int i;

	for (i = 0; i < nattr; i++) {
		if (!nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen)) {
			if (strncmp(attr[i].name, "x", attr[i].nameLen) == 0) x = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigX(p), nsvg__actualWidth(p));
			if (strncmp(attr[i].name, "y", attr[i].nameLen) == 0) y = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigY(p), nsvg__actualHeight(p));
			if (strncmp(attr[i].name, "width", attr[i].nameLen) == 0) w = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, nsvg__actualWidth(p));
			if (strncmp(attr[i].name, "height", attr[i].nameLen) == 0) h = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, nsvg__actualHeight(p));
			if (strncmp(attr[i].name, "rx", attr[i].nameLen) == 0) rx = fabsf(nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, nsvg__actualWidth(p)));
			if (strncmp(attr[i].name, "ry", attr[i].nameLen) == 0) ry = fabsf(nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, nsvg__actualHeight(p)));
		}
	}

	if (rx < 0.0f && ry > 0.0f) rx = ry;
	if (ry < 0.0f && rx > 0.0f) ry = rx;
	if (rx < 0.0f) rx = 0.0f;
	if (ry < 0.0f) ry = 0.0f;
	if (rx > w/2.0f) rx = w/2.0f;
	if (ry > h/2.0f) ry = h/2.0f;

	if (w != 0.0f && h != 0.0f) {
		nsvg__resetPath(p);

		if (rx < 0.00001f || ry < 0.0001f) {
			nsvg__moveTo(p, x, y);
			nsvg__lineTo(p, x+w, y);
			nsvg__lineTo(p, x+w, y+h);
			nsvg__lineTo(p, x, y+h);
		} else {
			// Rounded rectangle
			nsvg__moveTo(p, x+rx, y);
			nsvg__lineTo(p, x+w-rx, y);
			nsvg__cubicBezTo(p, x+w-rx*(1-NSVG_KAPPA90), y, x+w, y+ry*(1-NSVG_KAPPA90), x+w, y+ry);
			nsvg__lineTo(p, x+w, y+h-ry);
			nsvg__cubicBezTo(p, x+w, y+h-ry*(1-NSVG_KAPPA90), x+w-rx*(1-NSVG_KAPPA90), y+h, x+w-rx, y+h);
			nsvg__lineTo(p, x+rx, y+h);
			nsvg__cubicBezTo(p, x+rx*(1-NSVG_KAPPA90), y+h, x, y+h-ry*(1-NSVG_KAPPA90), x, y+h-ry);
			nsvg__lineTo(p, x, y+ry);
			nsvg__cubicBezTo(p, x, y+ry*(1-NSVG_KAPPA90), x+rx*(1-NSVG_KAPPA90), y, x+rx, y);
		}

		nsvg__addPath(p, 1);

		nsvg__addShape(p);
	}
}

static void nsvg__parseCircle(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	float cx = 0.0f;
	float cy = 0.0f;
	float r = 0.0f;
	int i;

	for (i = 0; i < nattr; i++) {
		if (!nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen)) {
			if (strncmp(attr[i].name, "cx", attr[i].nameLen) == 0) cx = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigX(p), nsvg__actualWidth(p));
			if (strncmp(attr[i].name, "cy", attr[i].nameLen) == 0) cy = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigY(p), nsvg__actualHeight(p));
			if (strncmp(attr[i].name, "r", attr[i].nameLen) == 0) r = fabsf(nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, nsvg__actualLength(p)));
		}
	}

	if (r > 0.0f) {
		nsvg__resetPath(p);

		nsvg__moveTo(p, cx+r, cy);
		nsvg__cubicBezTo(p, cx+r, cy+r*NSVG_KAPPA90, cx+r*NSVG_KAPPA90, cy+r, cx, cy+r);
		nsvg__cubicBezTo(p, cx-r*NSVG_KAPPA90, cy+r, cx-r, cy+r*NSVG_KAPPA90, cx-r, cy);
		nsvg__cubicBezTo(p, cx-r, cy-r*NSVG_KAPPA90, cx-r*NSVG_KAPPA90, cy-r, cx, cy-r);
		nsvg__cubicBezTo(p, cx+r*NSVG_KAPPA90, cy-r, cx+r, cy-r*NSVG_KAPPA90, cx+r, cy);

		nsvg__addPath(p, 1);

		nsvg__addShape(p);
	}
}

static void nsvg__parseEllipse(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	float cx = 0.0f;
	float cy = 0.0f;
	float rx = 0.0f;
	float ry = 0.0f;
	int i;

	for (i = 0; i < nattr; i++) {
		if (!nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen)) {
			if (strncmp(attr[i].name, "cx", attr[i].nameLen) == 0) cx = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigX(p), nsvg__actualWidth(p));
			if (strncmp(attr[i].name, "cy", attr[i].nameLen) == 0) cy = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigY(p), nsvg__actualHeight(p));
			if (strncmp(attr[i].name, "rx", attr[i].nameLen) == 0) rx = fabsf(nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, nsvg__actualWidth(p)));
			if (strncmp(attr[i].name, "ry", attr[i].nameLen) == 0) ry = fabsf(nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, nsvg__actualHeight(p)));
		}
	}

	if (rx > 0.0f && ry > 0.0f) {

		nsvg__resetPath(p);

		nsvg__moveTo(p, cx+rx, cy);
		nsvg__cubicBezTo(p, cx+rx, cy+ry*NSVG_KAPPA90, cx+rx*NSVG_KAPPA90, cy+ry, cx, cy+ry);
		nsvg__cubicBezTo(p, cx-rx*NSVG_KAPPA90, cy+ry, cx-rx, cy+ry*NSVG_KAPPA90, cx-rx, cy);
		nsvg__cubicBezTo(p, cx-rx, cy-ry*NSVG_KAPPA90, cx-rx*NSVG_KAPPA90, cy-ry, cx, cy-ry);
		nsvg__cubicBezTo(p, cx+rx*NSVG_KAPPA90, cy-ry, cx+rx, cy-ry*NSVG_KAPPA90, cx+rx, cy);

		nsvg__addPath(p, 1);

		nsvg__addShape(p);
	}
}

static void nsvg__parseLine(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	float x1 = 0.0;
	float y1 = 0.0;
	float x2 = 0.0;
	float y2 = 0.0;
	int i;

	for (i = 0; i < nattr; i++) {
		if (!nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen)) {
			if (strncmp(attr[i].name, "x1", attr[i].nameLen) == 0) x1 = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigX(p), nsvg__actualWidth(p));
			if (strncmp(attr[i].name, "y1", attr[i].nameLen) == 0) y1 = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigY(p), nsvg__actualHeight(p));
			if (strncmp(attr[i].name, "x2", attr[i].nameLen) == 0) x2 = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigX(p), nsvg__actualWidth(p));
			if (strncmp(attr[i].name, "y2", attr[i].nameLen) == 0) y2 = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, nsvg__actualOrigY(p), nsvg__actualHeight(p));
		}
	}

	nsvg__resetPath(p);

	nsvg__moveTo(p, x1, y1);
	nsvg__lineTo(p, x2, y2);

	nsvg__addPath(p, 0);

	nsvg__addShape(p);
}

static void nsvg__parsePoly(NSVGparser* p, NSVGattrValue* attr, int nattr, int closeFlag)
{
	int i;
	const char* s;
	const char* s2;
	int sLen;
	float args[2];
	int nargs, npts = 0;
	char item[64];

	nsvg__resetPath(p);

	for (i = 0; i < nattr; i++) {
		if (!nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen)) {
			if (strncmp(attr[i].name, "points", attr[i].nameLen) == 0) {
				s = attr[i].value;
				sLen = attr[i].valueLen;
				nargs = 0;
				while (*s && sLen > 0) {
					s2 = nsvg__getNextPathItem(s, sLen, item);
					sLen -= s2 - s;
					s = s2;

					args[nargs++] = (float)nsvg__atof(item);
					if (nargs >= 2) {
						if (npts == 0)
							nsvg__moveTo(p, args[0], args[1]);
						else
							nsvg__lineTo(p, args[0], args[1]);
						nargs = 0;
						npts++;
					}
				}
			}
		}
	}

	nsvg__addPath(p, (char)closeFlag);

	nsvg__addShape(p);
}

static void nsvg__parseGroup(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	NSVGshapeNode* shapeNode;

	nsvg__parseAttribs(p, attr, nattr);
	nsvg__resetPath(p);

	shapeNode = (NSVGshapeNode*)nsvg__malloc(p->image, sizeof(NSVGshapeNode));
	if (shapeNode == NULL) return;
	memset(shapeNode, 0, sizeof(NSVGshapeNode));

	shapeNode->shapeDepth = p->shapeDepth;

	// Add to tail
	if (p->image->shapes == NULL) {
		p->image->shapes = shapeNode;
	} else {
		p->shapesTail->next = shapeNode;
		shapeNode->prev = p->shapesTail;
	}
	p->shapesTail = shapeNode;
}

static void nsvg__parseSVG(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	int i;
	for (i = 0; i < nattr; i++) {
		if (!nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen)) {
			if (strncmp(attr[i].name, "width", attr[i].nameLen) == 0) {
				p->image->width = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, 0.0f);
			} else if (strncmp(attr[i].name, "height", attr[i].nameLen) == 0) {
				p->image->height = nsvg__parseCoordinate(p, attr[i].value, attr[i].valueLen, 0.0f, 0.0f);
			} else if (strncmp(attr[i].name, "viewBox", attr[i].nameLen) == 0) {
				const char *s = attr[i].value;
				const char* s2;
				char buf[64];
				int valLen = attr[i].valueLen;
				int len;

				len = valLen+1 < 64 ? valLen+1 : 64;
				s2 = nsvg__parseNumber(s, buf, len);
				valLen -= s2 - s;
				s = s2;
				p->image->viewMinx = nsvg__atof(buf);

				for (; *s && valLen > 0 && (nsvg__isspace(*s) || *s == '%' || *s == ','); s++, valLen--);
				if (!*s || valLen <= 0) return;

				len = valLen+1 < 64 ? valLen+1 : 64;
				s2 = nsvg__parseNumber(s, buf, len);
				valLen -= s2 - s;
				s = s2;
				p->image->viewMiny = nsvg__atof(buf);

				for (; *s && valLen > 0 && (nsvg__isspace(*s) || *s == '%' || *s == ','); s++, valLen--);
				if (!*s || valLen <= 0) return;

				len = valLen+1 < 64 ? valLen+1 : 64;
				s2 = nsvg__parseNumber(s, buf, len);
				valLen -= s2 - s;
				s = s2;
				p->image->viewWidth = nsvg__atof(buf);

				for (; *s && valLen > 0 && (nsvg__isspace(*s) || *s == '%' || *s == ','); s++, valLen--);
				if (!*s || valLen <= 0) return;

				len = valLen+1 < 64 ? valLen+1 : 64;
				s2 = nsvg__parseNumber(s, buf, len);
				valLen -= s2 - s;
				s = s2;
				p->image->viewHeight = nsvg__atof(buf);
			} else if (strncmp(attr[i].name, "preserveAspectRatio", attr[i].nameLen) == 0) {
				if (attr[i].valueLen >= 4) {
					if (strstr(attr[i].value, "none") != 0) {
						// No uniform scaling
						p->image->alignType = NSVG_ALIGN_NONE;
					} else {
						// Parse X align
						if (strstr(attr[i].value, "xMin") != 0)
							p->image->alignX = NSVG_ALIGN_MIN;
						else if (strstr(attr[i].value, "xMid") != 0)
							p->image->alignX = NSVG_ALIGN_MID;
						else if (strstr(attr[i].value, "xMax") != 0)
							p->image->alignX = NSVG_ALIGN_MAX;
						// Parse X align
						if (strstr(attr[i].value, "yMin") != 0)
							p->image->alignY = NSVG_ALIGN_MIN;
						else if (strstr(attr[i].value, "yMid") != 0)
							p->image->alignY = NSVG_ALIGN_MID;
						else if (strstr(attr[i].value, "yMax") != 0)
							p->image->alignY = NSVG_ALIGN_MAX;
						// Parse meet/slice
						p->image->alignType = NSVG_ALIGN_MEET;
						if (attr[i].valueLen >= 5 && strstr(attr[i].value, "slice") != 0)
							p->image->alignType = NSVG_ALIGN_SLICE;
					}
				}
			}
		}
	}
}

static void nsvg__parseGradient(NSVGparser* p, NSVGattrValue* attr, int nattr, signed char type)
{
	int i, len;

	NSVGgradientData* grad = (NSVGgradientData*)nsvg__malloc(p->image, sizeof(NSVGgradientData));
	if (grad == NULL) return;
	memset(grad, 0, sizeof(NSVGgradientData));
	grad->units = NSVG_OBJECT_SPACE;
	grad->type = type;
	if (grad->type == NSVG_PAINT_LINEAR_GRADIENT) {
		grad->linear.x1 = nsvg__coord(0.0f, NSVG_UNITS_PERCENT);
		grad->linear.y1 = nsvg__coord(0.0f, NSVG_UNITS_PERCENT);
		grad->linear.x2 = nsvg__coord(100.0f, NSVG_UNITS_PERCENT);
		grad->linear.y2 = nsvg__coord(0.0f, NSVG_UNITS_PERCENT);
	} else if (grad->type == NSVG_PAINT_RADIAL_GRADIENT) {
		grad->radial.cx = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
		grad->radial.cy = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
		grad->radial.r = nsvg__coord(50.0f, NSVG_UNITS_PERCENT);
	}

	nsvg__xformIdentity(grad->xform);

	for (i = 0; i < nattr; i++) {
		if (strncmp(attr[i].name, "id", attr[i].nameLen) == 0) {
			if (grad->id == NULL) grad->id = nsvg__allocId(p);
			if (grad->id == NULL) return;
			len = attr[i].valueLen < 63 ? attr[i].valueLen : 63;
			strncpy(grad->id->id, attr[i].value, len);
			grad->id->id[len] = '\0';
		} else if (!nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen)) {
			if (strncmp(attr[i].name, "gradientUnits", attr[i].nameLen) == 0) {
				if (strncmp(attr[i].value, "objectBoundingBox", attr[i].valueLen) == 0)
					grad->units = NSVG_OBJECT_SPACE;
				else
					grad->units = NSVG_USER_SPACE;
			} else if (strncmp(attr[i].name, "gradientTransform", attr[i].nameLen) == 0) {
				nsvg__parseTransform(grad->xform, attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "cx", attr[i].nameLen) == 0) {
				grad->radial.cx = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "cy", attr[i].nameLen) == 0) {
				grad->radial.cy = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "r", attr[i].nameLen) == 0) {
				grad->radial.r = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "fx", attr[i].nameLen) == 0) {
				grad->radial.fx = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "fy", attr[i].nameLen) == 0) {
				grad->radial.fy = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "x1", attr[i].nameLen) == 0) {
				grad->linear.x1 = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "y1", attr[i].nameLen) == 0) {
				grad->linear.y1 = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "x2", attr[i].nameLen) == 0) {
				grad->linear.x2 = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "y2", attr[i].nameLen) == 0) {
				grad->linear.y2 = nsvg__parseCoordinateRaw(attr[i].value, attr[i].valueLen);
			} else if (strncmp(attr[i].name, "spreadMethod", attr[i].nameLen) == 0) {
				if (strncmp(attr[i].value, "pad", attr[i].valueLen) == 0)
					grad->spread = NSVG_SPREAD_PAD;
				else if (strncmp(attr[i].value, "reflect", attr[i].valueLen) == 0)
					grad->spread = NSVG_SPREAD_REFLECT;
				else if (strncmp(attr[i].value, "repeat", attr[i].valueLen) == 0)
					grad->spread = NSVG_SPREAD_REPEAT;
			} else if (strncmp(attr[i].name, "xlink:href", attr[i].nameLen) == 0) {
				const char *href = attr[i].value;
				if (grad->ref == NULL) grad->ref = nsvg__allocId(p);
				if (grad->ref == NULL) return;
				len = attr[i].valueLen < 62 ? attr[i].valueLen : 62;
				strncpy(grad->ref->id, href+1, len);
				grad->ref->id[len] = '\0';
			}
		}
	}

	grad->next = p->gradients;
	p->gradients = grad;
}

static void nsvg__parseGradientStop(NSVGparser* p, NSVGattrValue* attr, int nattr)
{
	NSVGattrib* curAttr = nsvg__getAttr(p);
	NSVGgradientData* grad;
	NSVGgradientStop* stop;
	int i, idx, nstops;

	curAttr->stopOffset = 0;
	curAttr->stopColor = 0;
	curAttr->stopOpacity = 1.0f;

	for (i = 0; i < nattr; i++) {
		nsvg__parseAttr(p, attr[i].name, attr[i].nameLen, attr[i].value, attr[i].valueLen);
	}

	// Add stop to the last gradient.
	grad = p->gradients;
	if (grad == NULL) return;

	nstops = grad->nstops + 1;
	grad->stops = (NSVGgradientStop*)nsvg__realloc(p->image, grad->stops, sizeof(NSVGgradientStop)*nstops, sizeof(NSVGgradientStop)*grad->nstops);
	grad->nstops = nstops;
	if (grad->stops == NULL) return;

	// Insert
	idx = grad->nstops-1;
	for (i = 0; i < grad->nstops-1; i++) {
		if (curAttr->stopOffset < grad->stops[i].offset) {
			idx = i;
			break;
		}
	}
	if (idx != grad->nstops-1) {
		for (i = grad->nstops-1; i > idx; i--)
			grad->stops[i] = grad->stops[i-1];
	}

	stop = &grad->stops[idx];
	stop->color = curAttr->stopColor;
	stop->color |= (unsigned int)(curAttr->stopOpacity*255) << 24;
	stop->offset = curAttr->stopOffset;
}

static const char* nsvg__parseAnimateTime(const char* str, int strLen, long* millis)
{
	const char* ptr;
	const char* ptr2;
	char it[64];
	float value = 0;
	char hasHours = 0;
	char hasMinutes = 0;
	char hasNext = 0;
	int len;

	*millis = 0;

	ptr = str;
	while (*ptr && strLen > 0) {
		if (nsvg__isdigit(*ptr)) {
			len = strLen+1 < 64 ? strLen+1 : 64;
			ptr2 = nsvg__parseNumber(ptr, it, len);
			strLen -= ptr2 - ptr;
			ptr = ptr2;
			value = (float)nsvg__atof(it);
		} else {
			if (*ptr == ':') {
				ptr++;
				strLen--;
				if (!hasHours) {
					*millis += (long)value * 60 * 60 * 1000;
					hasHours = 1;
					continue;
				} else if (!hasMinutes) {
					*millis += (long)value * 60 * 1000;
					hasMinutes = 1;
					continue;
				}
			} else if (strLen >= 1 && strncmp(ptr, "h", 1) == 0) {
				*millis = (long)(value * 60 * 60 * 1000);
				ptr += 1;
				strLen -= 1;
			} else if (strLen >= 3 && strncmp(ptr, "min", 3) == 0) {
				*millis = (long)(value * 60 * 1000);
				ptr += 3;
				strLen -= 3;
			} else if (strLen >= 1 && strncmp(ptr, "s", 1) == 0) {
				*millis = (long)(value * 1000);
				ptr += 1;
				strLen -= 1;
			} else if (strLen >= 2 && strncmp(ptr, "ms", 2) == 0) {
				*millis = (long)(value);
				ptr += 2;
				strLen -= 2;
			} else {
				// break before resetting value.
				break;
			}
			value = 0;
			break;
		}
	}

	// Add seconds (default) in case of any leftover values.
	if (value > 0) {
		*millis += (long)(value * 1000);
	}

	// Move to next value.
	for (; *ptr && strLen > 0 && *ptr != ';'; ptr++, strLen--);
	if (!*ptr || strLen <= 0) {
		return ptr;
	}
	// Skip the semicolon.
	ptr++;

	return ptr;
}

static const char* nsvg__parseAnimateValue(NSVGparser* p, float* args, const char* str, int strLen, char type, int* na)
{
	float xform[6];
	const char* ptr = str;
	unsigned int color;
	int dashCount;
	int len;

	// Clear number of args copied.
	*na = 0;

	// Skip whitespaces.
	for (; *ptr && strLen > 0 && nsvg__isspace(*ptr); ptr++, strLen--);
	if (!*ptr || strLen <= 0) {
		return ptr;
	}

	// Parse the values according to type.
	switch (type) {
		case NSVG_ANIMATE_TYPE_TRANSFORM_TRANSLATE:
			len = nsvg__parseTransformArgs(ptr, strLen, args, 2, na, 0);
			if (*na == 1) args[1] = 0.0;
			*na = 2;
			break;
		case NSVG_ANIMATE_TYPE_TRANSFORM_SCALE:
			nsvg__parseTransformArgs(ptr, strLen, args, 2, na, 0);
			if (*na == 1) args[1] = args[0];
			*na = 2;
			break;
		case NSVG_ANIMATE_TYPE_TRANSFORM_ROTATE:
			nsvg__parseTransformArgs(ptr, strLen, args, 3, na, 0);
			break;
		case NSVG_ANIMATE_TYPE_TRANSFORM_SKEWX:
		case NSVG_ANIMATE_TYPE_TRANSFORM_SKEWY:
			nsvg__parseTransformArgs(ptr, strLen, args, 1, na, 0);
			break;
		case NSVG_ANIMATE_TYPE_OPACITY:
		case NSVG_ANIMATE_TYPE_FILL_OPACITY:
		case NSVG_ANIMATE_TYPE_STROKE_OPACITY:
			args[0] = nsvg__parseOpacity(ptr);
			*na = 1;
			break;
		case NSVG_ANIMATE_TYPE_FILL:
		case NSVG_ANIMATE_TYPE_STROKE:
			color = nsvg__parseColor(ptr, strLen);
			args[0] = (float)(color & 0xFF);
			args[1] = (float)((color >> 8) & 0xFF);
			args[2] = (float)((color >> 16) & 0xFF);
			*na = 3;
			break;
		case NSVG_ANIMATE_TYPE_STROKE_WIDTH:
		case NSVG_ANIMATE_TYPE_STROKE_DASHOFFSET:
			args[0] = nsvg__parseCoordinate(p, ptr, strLen, 0.0f, nsvg__actualLength(p));
			*na = 1;
			break;
		case NSVG_ANIMATE_TYPE_STROKE_DASHARRAY:
			dashCount = nsvg__parseStrokeDashArray(p, ptr, strLen, args);
			args[dashCount] = (float)dashCount;
			*na = dashCount+1;
			break;
		case NSVG_ANIMATE_TYPE_SPLINE:
			nsvg__parseTransformArgs(ptr, strLen, args, 4, na, 0);
			if (*na != 4) {
				args[0] = args[1] = args[2] = args[3] = 0;
				*na = 4;
			}
			break;
		case NSVG_ANIMATE_TYPE_NUMBER:
			nsvg__parseTransformArgs(ptr, strLen, args, 1, na, 0);
			break;
	}

	// Move to next value.
	for (; *ptr && strLen > 0 && *ptr != ';'; ptr++, strLen--);
	if (!*ptr || strLen <= 0) {
		return ptr;
	}
	// Skip the semicolon.
	ptr++;

	return ptr;
}

static int nsvg__parseAnimateValuesCount(const char* str, int strLen)
{
	const char* ptr;
	const char* ptr2;
	char it[64];
	int count = 0;
	char hasValue = 1;
	int len;

	ptr = str;
	while (*ptr && strLen > 0) {

		// Skip whitespaces.
		for (; *ptr && strLen > 0 && nsvg__isspace(*ptr); ptr++, strLen--);
		if (!*ptr || strLen <= 0) {
			break;
		}

		count++;

		// Move to next value.
		for (; *ptr && strLen > 0 && *ptr != ';'; ptr++, strLen--);
		if (!*ptr || strLen <= 0) {
			break;
		}
		// Skip the semicolon.
		ptr++;
		strLen--;
	}

	return count;
}

static void nsvg__parseAnimate(NSVGparser* p, const char* tagName, int tagNameLen, NSVGattrValue* attr, int nattr)
{
	char it[64];
	float args[10] = {0};
	long unset = 0x80000000;
	long begin = 0;
	long end = unset;
	long dur = unset;
	long repeatDur = unset;
	float keyTimeBegin;
	float keyTimeEnd;
	int repeatCount = (int)unset;
	NSVGattrValue* attrName = NULL;
	NSVGattrValue* id = NULL;
	NSVGattrValue* type = NULL;
	NSVGattrValue* from = NULL;
	NSVGattrValue* to = NULL;
	NSVGattrValue* values = NULL;
	NSVGattrValue* keyTimes = NULL;
	NSVGattrValue* keySplines = NULL;
	int valuesCount = 0;
	int keyTimesCount = 0;
	int keySplinesCount = 0;
	NSVGanimate* animateList = NULL;
	NSVGanimate* animateTail = NULL;
	NSVGanimate* animate;
	NSVGshapeNode* shapeNode;
	char animateType;
	char calcMode = NSVG_ANIMATE_CALC_MODE_LINEAR;
	char additive = NSVG_ANIMATE_ADDITIVE_REPLACE;
	char fill = NSVG_ANIMATE_FILL_REMOVE;
	const char* s;
	int na, argsNa;
	int i, len;

	// Parse all received attributes.
	for (i = 0; i < nattr; i++) {
		if (strncmp(attr[i].name, "attributeName", attr[i].nameLen) == 0) {
			attrName = &attr[i];
		} else if (strncmp(attr[i].name, "id", attr[i].nameLen) == 0) {
			id = &attr[i];
		} else if (strncmp(attr[i].name, "type", attr[i].nameLen) == 0) {
			type = &attr[i];
		} else if (strncmp(attr[i].name, "from", attr[i].nameLen) == 0) {
			from = &attr[i];
		} else if (strncmp(attr[i].name, "to", attr[i].nameLen) == 0) {
			to = &attr[i];
		} else if (strncmp(attr[i].name, "values", attr[i].nameLen) == 0) {
			values = &attr[i];
			valuesCount = nsvg__parseAnimateValuesCount(values->value, attr[i].valueLen);
		} else if (strncmp(attr[i].name, "keyTimes", attr[i].nameLen) == 0) {
			keyTimes = &attr[i];
			keyTimesCount = nsvg__parseAnimateValuesCount(keyTimes->value, attr[i].valueLen);
		} else if (strncmp(attr[i].name, "keySplines", attr[i].nameLen) == 0) {
			keySplines = &attr[i];
			keySplinesCount = nsvg__parseAnimateValuesCount(keySplines->value, attr[i].valueLen);
		} else if (strncmp(attr[i].name, "begin", attr[i].nameLen) == 0) {
			nsvg__parseAnimateTime(attr[i].value, attr[i].valueLen, &begin);
		} else if (strncmp(attr[i].name, "end", attr[i].nameLen) == 0) {
			nsvg__parseAnimateTime(attr[i].value, attr[i].valueLen, &end);
		} else if (strncmp(attr[i].name, "dur", attr[i].nameLen) == 0) {
			nsvg__parseAnimateTime(attr[i].value, attr[i].valueLen, &dur);
		} else if (strncmp(attr[i].name, "repeatDur", attr[i].nameLen) == 0) {
			if (strncmp(attr[i].value, "indefinite", attr[i].valueLen) == 0) {
				repeatDur = -1;
			} else {
				nsvg__parseAnimateTime(attr[i].value, attr[i].valueLen, &repeatDur);
			}
		} else if ((strncmp(attr[i].name, "additive", attr[i].nameLen) == 0) && 
				   (strncmp(attr[i].value, "sum", attr[i].valueLen) == 0)) {
			additive = NSVG_ANIMATE_ADDITIVE_SUM;
		} else if ((strncmp(attr[i].name, "fill", attr[i].nameLen) == 0) && 
				   (strncmp(attr[i].value, "freeze", attr[i].valueLen) == 0)) {
			fill = NSVG_ANIMATE_FILL_FREEZE;
		} else if (strncmp(attr[i].name, "repeatCount", attr[i].nameLen) == 0) {
			if (strncmp(attr[i].value, "indefinite", attr[i].valueLen) == 0) {
				repeatCount = -1;
			} else {
				repeatCount = (int)nsvg__atof(attr[i].value);
			}
		} else if (strncmp(attr[i].name, "calcMode", attr[i].nameLen) == 0) {
			if (strncmp(attr[i].value, "linear", attr[i].valueLen) == 0) {
				calcMode = NSVG_ANIMATE_CALC_MODE_LINEAR;
			} else if (strncmp(attr[i].value, "discrete", attr[i].valueLen) == 0) {
				calcMode = NSVG_ANIMATE_CALC_MODE_DISCRETE;
			} else if (strncmp(attr[i].value, "paced", attr[i].valueLen) == 0) {
				calcMode = NSVG_ANIMATE_CALC_MODE_PACED;
			} else if (strncmp(attr[i].value, "spline", attr[i].valueLen) == 0) {
				calcMode = NSVG_ANIMATE_CALC_MODE_SPLINE;
			}
		}
	}

	// Check for validity.
	if (dur == unset) return;
	if (!values && (!from || !to)) return;
	if (keyTimesCount > 0 && valuesCount > 0 && keyTimesCount != valuesCount) return;
	if (keySplinesCount > 0 && valuesCount > 0 && keySplinesCount != valuesCount - 1) return;

	if (repeatDur != unset) {
		if (repeatCount != unset) {
			repeatCount = -1;
		}
		end = (end > 0 && repeatDur < 0) ? end :
			  (end < 0 && repeatDur > 0) ? repeatDur :
			  (end > 0 && repeatDur > 0) ? ((end < repeatDur) ? end : repeatDur) : end;
	}

	// Set type of animation.
	if (tagNameLen == 16 && strncmp(tagName, "animateTransform", tagNameLen) == 0) {
		if (strncmp(attrName->value, "transform", attrName->valueLen) == 0) {
			if (type == NULL) return;
			else if (strncmp(type->value, "translate", type->valueLen) == 0)
				animateType = NSVG_ANIMATE_TYPE_TRANSFORM_TRANSLATE;
			else if (strncmp(type->value, "scale", type->valueLen) == 0)
				animateType = NSVG_ANIMATE_TYPE_TRANSFORM_SCALE;
			else if (strncmp(type->value, "rotate", type->valueLen) == 0)
				animateType = NSVG_ANIMATE_TYPE_TRANSFORM_ROTATE;
			else if (strncmp(type->value, "skewX", type->valueLen) == 0)
				animateType = NSVG_ANIMATE_TYPE_TRANSFORM_SKEWX;
			else if (strncmp(type->value, "skewY", type->valueLen) == 0)
				animateType = NSVG_ANIMATE_TYPE_TRANSFORM_SKEWY;
			else return;
		} else return;
	} else if (tagNameLen == 7 && strncmp(tagName, "animate", tagNameLen) == 0) {
		if (strncmp(attrName->value, "opacity", attrName->valueLen) == 0) {
			animateType = NSVG_ANIMATE_TYPE_OPACITY;
		} else if (strncmp(attrName->value, "fill", attrName->valueLen) == 0) {
			animateType = NSVG_ANIMATE_TYPE_FILL;
		} else if (strncmp(attrName->value, "fill-opacity", attrName->valueLen) == 0) {
			animateType = NSVG_ANIMATE_TYPE_FILL_OPACITY;
		} else if (strncmp(attrName->value, "stroke", attrName->valueLen) == 0) {
			animateType = NSVG_ANIMATE_TYPE_STROKE;
		} else if (strncmp(attrName->value, "stroke-opacity", attrName->valueLen) == 0) {
			animateType = NSVG_ANIMATE_TYPE_STROKE_OPACITY;
		} else if (strncmp(attrName->value, "stroke-width", attrName->valueLen) == 0) {
			animateType = NSVG_ANIMATE_TYPE_STROKE_WIDTH;
		} else if (strncmp(attrName->value, "stroke-dashoffset", attrName->valueLen) == 0) {
			animateType = NSVG_ANIMATE_TYPE_STROKE_DASHOFFSET;
		} else if (strncmp(attrName->value, "stroke-dasharray", attrName->valueLen) == 0) {
			animateType = NSVG_ANIMATE_TYPE_STROKE_DASHARRAY;
		} else return;
	} else return;

	// Check if this is a simple animation (only to and from.)
	if (!values || valuesCount < 2) {
		animate = (NSVGanimate*)malloc(sizeof(NSVGanimate));
		if (animate == NULL) goto error;
		memset(animate, 0, sizeof(NSVGanimate));

		animate->type = animateType;
		animate->begin = begin;
		animate->end = end;
		animate->dur = dur;
		animate->groupDur = dur;
		animate->repeatCount = repeatCount;

		if (!values) {
			nsvg__parseAnimateValue(p, animate->src, from->value, from->valueLen, animateType, &animate->srcNa);
			nsvg__parseAnimateValue(p, animate->dst, to->value, to->valueLen, animateType, &animate->dstNa);
		} else {
			s = nsvg__parseAnimateValue(p, animate->src, values->value, values->valueLen, animateType, &animate->srcNa);
			values->valueLen -= s - values->value;
			values->value = s;
			memcpy(animate->dst, animate->src, sizeof(animate->dst));
			animate->dstNa = animate->srcNa;
		}

		animate->calcMode = calcMode;
		animate->additive = additive;
		animate->fill = fill;

		animateList = animate;
		animateTail = animate;
	} else {
		// Set initial values.
		if (keyTimes != NULL) {
			s = nsvg__parseAnimateValue(p, &keyTimeEnd, keyTimes->value, keyTimes->valueLen, NSVG_ANIMATE_TYPE_NUMBER, &na);
			keyTimes->valueLen -= s - keyTimes->value;
			keyTimes->value = s;
			if (na == 0) keyTimeEnd = 0;
		} else {
			keyTimeEnd = 0;
		}
		s = nsvg__parseAnimateValue(p, args, values->value, values->valueLen, animateType, &argsNa);
		values->valueLen -= s - values->value;
		values->value = s;

		// Parse the values in pairs, to create animatiton segments.
		for (i = 0; i < valuesCount - 1; i++) {
			animate = (NSVGanimate*)malloc(sizeof(NSVGanimate));
			if (animate == NULL) goto error;
			memset(animate, 0, sizeof(NSVGanimate));

			keyTimeBegin = keyTimeEnd;
			if (keyTimes != NULL) {
				s = nsvg__parseAnimateValue(p, &keyTimeEnd, keyTimes->value, keyTimes->valueLen, NSVG_ANIMATE_TYPE_NUMBER, &na);
				keyTimes->valueLen -= s - keyTimes->value;
				keyTimes->value = s;
			} else if (i < valuesCount - 2) {
				keyTimeEnd = (i + 1) / (float)(valuesCount - 1);
			} else {
				keyTimeEnd = 1;
			}

			if (keySplines != NULL) {
				s = nsvg__parseAnimateValue(p, animate->spline, keySplines->value, keySplines->valueLen, NSVG_ANIMATE_TYPE_SPLINE, &na);
				keySplines->valueLen -= s - keySplines->value;
				keySplines->value = s;
				}

			animate->type = animateType;
			animate->begin = begin + dur * keyTimeBegin;
			animate->end = end;
			animate->dur = dur * (keyTimeEnd - keyTimeBegin);
			animate->groupDur = dur;
			animate->repeatCount = repeatCount;

			animate->src[0] = args[0]; animate->src[1] = args[1]; animate->src[2] = args[2];
			animate->srcNa = argsNa;
			s = nsvg__parseAnimateValue(p, args, values->value, values->valueLen, animateType, &argsNa);
			values->valueLen -= s - values->value;
			values->value = s;
			animate->dst[0] = args[0]; animate->dst[1] = args[1]; animate->dst[2] = args[2];
			animate->dstNa = argsNa;

			animate->calcMode = calcMode;
			animate->additive = additive;
			animate->fill = fill;

			if (animateList == NULL) {
				animateList = animate;
				animateTail = animate;
			} else {
				animateTail->next = animate;
				animateTail = animate;
			}
		}
	}

	// Set the flags.
	animateList->flags |= NSVG_ANIMATE_FLAG_GROUP_FIRST;
	animateTail->flags |= NSVG_ANIMATE_FLAG_GROUP_LAST;

	// Find which shape this animate refers to.
	for (shapeNode = p->shapesTail; shapeNode != NULL && shapeNode->shapeDepth >= p->shapeDepth; shapeNode = shapeNode->prev);

	// Add the animate list to the last shape.
	if (shapeNode != NULL) {
		if (shapeNode->animatesTail != NULL) {
			shapeNode->animatesTail->next = animateList;
			shapeNode->animatesTail = animateTail;
		} else {
			shapeNode->animates = animateList;
			shapeNode->animatesTail = animateTail;
		}
	}

	return;

error:
	while (animateList != NULL) {
		animate = animateList->next;
		free(animateList);
		animateList = animate;
	}
}

static void nsvg__startElement(void* userData, const char* elName, int elNameLen, NSVGattrValue* attr, int nattr)
{
	NSVGparser* p = (NSVGparser*)userData;

	p->shapeDepth++;

	if (p->defsFlag) {
		// Skip everything but gradients in defs
		if (strncmp(elName, "linearGradient", elNameLen) == 0) {
			nsvg__parseGradient(p, attr, nattr, NSVG_PAINT_LINEAR_GRADIENT);
		} else if (strncmp(elName, "radialGradient", elNameLen) == 0) {
			nsvg__parseGradient(p, attr, nattr, NSVG_PAINT_RADIAL_GRADIENT);
		} else if (strncmp(elName, "stop", elNameLen) == 0) {
			nsvg__parseGradientStop(p, attr, nattr);
		}
		return;
	}

	if (strncmp(elName, "g", elNameLen) == 0) {
		nsvg__pushAttr(p);
		nsvg__parseGroup(p, attr, nattr);
	} else if (strncmp(elName, "path", elNameLen) == 0) {
		if (p->pathFlag)	// Do not allow nested paths.
			return;
		nsvg__pushAttr(p);
		nsvg__parsePath(p, attr, nattr);
		nsvg__popAttr(p);
	} else if (strncmp(elName, "rect", elNameLen) == 0) {
		nsvg__pushAttr(p);
		nsvg__parseRect(p, attr, nattr);
		nsvg__popAttr(p);
	} else if (strncmp(elName, "circle", elNameLen) == 0) {
		nsvg__pushAttr(p);
		nsvg__parseCircle(p, attr, nattr);
		nsvg__popAttr(p);
	} else if (strncmp(elName, "ellipse", elNameLen) == 0) {
		nsvg__pushAttr(p);
		nsvg__parseEllipse(p, attr, nattr);
		nsvg__popAttr(p);
	} else if (strncmp(elName, "line", elNameLen) == 0)  {
		nsvg__pushAttr(p);
		nsvg__parseLine(p, attr, nattr);
		nsvg__popAttr(p);
	} else if (strncmp(elName, "polyline", elNameLen) == 0)  {
		nsvg__pushAttr(p);
		nsvg__parsePoly(p, attr, nattr, 0);
		nsvg__popAttr(p);
	} else if (strncmp(elName, "polygon", elNameLen) == 0)  {
		nsvg__pushAttr(p);
		nsvg__parsePoly(p, attr, nattr, 1);
		nsvg__popAttr(p);
	} else  if (strncmp(elName, "linearGradient", elNameLen) == 0) {
		nsvg__parseGradient(p, attr, nattr, NSVG_PAINT_LINEAR_GRADIENT);
	} else if (strncmp(elName, "radialGradient", elNameLen) == 0) {
		nsvg__parseGradient(p, attr, nattr, NSVG_PAINT_RADIAL_GRADIENT);
	} else if (strncmp(elName, "stop", elNameLen) == 0) {
		nsvg__parseGradientStop(p, attr, nattr);
	} else if (strncmp(elName, "defs", elNameLen) == 0) {
		p->defsFlag = 1;
	} else if ((strncmp(elName, "animate", elNameLen) == 0) ||
			   (strncmp(elName, "animateTransform", elNameLen) == 0)) {
		nsvg__pushAttr(p);
		nsvg__parseAnimate(p, elName, elNameLen, attr, nattr);
		nsvg__popAttr(p);
	} else if (strncmp(elName, "svg", elNameLen) == 0) {
		nsvg__parseSVG(p, attr, nattr);
	}
}

static void nsvg__endElement(void* userData, const char* elName, int elNameLen)
{
	NSVGparser* p = (NSVGparser*)userData;

	if (strncmp(elName, "g", elNameLen) == 0) {
		nsvg__popAttr(p);
	} else if (strncmp(elName, "path", elNameLen) == 0) {
		p->pathFlag = 0;
	} else if (strncmp(elName, "defs", elNameLen) == 0) {
		p->defsFlag = 0;
	}

	p->shapeDepth--;
}

static void nsvg__content(void* userData, const char* content, int contentLen)
{
	NSVG_NOTUSED(userData);
	NSVG_NOTUSED(content);
	NSVG_NOTUSED(contentLen);
	// empty
}

static void nsvg__imageBounds(NSVGimage* image, float* bounds)
{
	NSVGshapeNode* shapeNode;
	shapeNode = image->shapes;
	for (; shapeNode != NULL && shapeNode->shape == NULL; shapeNode = shapeNode->next);
	if (shapeNode == NULL) {
		bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0;
		return;
	}
	bounds[0] = shapeNode->shape->bounds[0];
	bounds[1] = shapeNode->shape->bounds[1];
	bounds[2] = shapeNode->shape->bounds[2];
	bounds[3] = shapeNode->shape->bounds[3];
	for (shapeNode = shapeNode->next; shapeNode != NULL; shapeNode = shapeNode->next) {
		if (shapeNode->shape == NULL) continue;
		bounds[0] = nsvg__minf(bounds[0], shapeNode->shape->bounds[0]);
		bounds[1] = nsvg__minf(bounds[1], shapeNode->shape->bounds[1]);
		bounds[2] = nsvg__maxf(bounds[2], shapeNode->shape->bounds[2]);
		bounds[3] = nsvg__maxf(bounds[3], shapeNode->shape->bounds[3]);
	}
}

static float nsvg__viewAlign(float content, float container, int type)
{
	if (type == NSVG_ALIGN_MIN)
		return 0;
	else if (type == NSVG_ALIGN_MAX)
		return container - content;
	// mid
	return (container - content) * 0.5f;
}

static void nsvg__scaleGradient(NSVGgradient* grad, float tx, float ty, float sx, float sy)
{
	float t[6];
	nsvg__xformSetTranslation(t, tx, ty);
	memcpy(grad->xform, grad->orig.xform, sizeof(float)*6);
	nsvg__xformMultiply (grad->xform, t);

	nsvg__xformSetScale(t, sx, sy);
	nsvg__xformMultiply (grad->xform, t);
}

static void nsvg__scaleToViewbox(NSVGimage* image)
{
	NSVGshapeNode* shapeNode;
	NSVGshape* shape;
	NSVGpath* path;
	float tx, ty, sx, sy, us, bounds[4], t[6], avgs;
	int i;
	float* pt;

	// Guess image size if not set completely.
	nsvg__imageBounds(image, bounds);

	if (image->viewWidth == 0) {
		if (image->width > 0) {
			image->viewWidth = image->width;
		} else {
			image->viewMinx = bounds[0];
			image->viewWidth = bounds[2] - bounds[0];
		}
	}
	if (image->viewHeight == 0) {
		if (image->height > 0) {
			image->viewHeight = image->height;
		} else {
			image->viewMiny = bounds[1];
			image->viewHeight = bounds[3] - bounds[1];
		}
	}
	if (image->width == 0)
		image->width = image->viewWidth;
	if (image->height == 0)
		image->height = image->viewHeight;

	tx = -image->viewMinx;
	ty = -image->viewMiny;
	sx = image->viewWidth > 0 ? image->width / image->viewWidth : 0;
	sy = image->viewHeight > 0 ? image->height / image->viewHeight : 0;
	// Unit scaling
	us = 1.0f / nsvg__convertToPixels(image, nsvg__coord(1.0f, nsvg__parseUnits(image->units)), 0.0f, 1.0f);

	// Fix aspect ratio
	if (image->alignType == NSVG_ALIGN_MEET) {
		// fit whole image into viewbox
		sx = sy = nsvg__minf(sx, sy);
		tx += nsvg__viewAlign(image->viewWidth*sx, image->width, image->alignX) / sx;
		ty += nsvg__viewAlign(image->viewHeight*sy, image->height, image->alignY) / sy;
	} else if (image->alignType == NSVG_ALIGN_SLICE) {
		// fill whole viewbox with image
		sx = sy = nsvg__maxf(sx, sy);
		tx += nsvg__viewAlign(image->viewWidth*sx, image->width, image->alignX) / sx;
		ty += nsvg__viewAlign(image->viewHeight*sy, image->height, image->alignY) / sy;
	}

	// Transform
	sx *= us;
	sy *= us;
	avgs = (sx+sy) / 2.0f;
	for (shapeNode = image->shapes; shapeNode != NULL; shapeNode = shapeNode->next) {
		shape = shapeNode->shape;
		if (shape == NULL) continue;

		shape->bounds[0] = (shape->bounds[0] + tx) * sx;
		shape->bounds[1] = (shape->bounds[1] + ty) * sy;
		shape->bounds[2] = (shape->bounds[2] + tx) * sx;
		shape->bounds[3] = (shape->bounds[3] + ty) * sy;
		for (path = shape->paths; path != NULL; path = path->next) {
			path->bounds[0] = (path->bounds[0] + tx) * sx;
			path->bounds[1] = (path->bounds[1] + ty) * sy;
			path->bounds[2] = (path->bounds[2] + tx) * sx;
			path->bounds[3] = (path->bounds[3] + ty) * sy;
			if (!path->scaled) {
				for (i =0; i < path->npts; i++) {
					pt = &path->pts[i*2];
					pt[0] = (pt[0] + tx) * sx;
					pt[1] = (pt[1] + ty) * sy;
				}
				path->scaled = 1;
			}
		}

		if (shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT || shape->fill.type == NSVG_PAINT_RADIAL_GRADIENT) {
			nsvg__scaleGradient(shape->fill.gradient, tx,ty, sx,sy);
			memcpy(t, shape->fill.gradient->xform, sizeof(float)*6);
			nsvg__xformInverse(shape->fill.gradient->xform, t);
		}
		if (shape->stroke.type == NSVG_PAINT_LINEAR_GRADIENT || shape->stroke.type == NSVG_PAINT_RADIAL_GRADIENT) {
			nsvg__scaleGradient(shape->stroke.gradient, tx,ty, sx,sy);
			memcpy(t, shape->stroke.gradient->xform, sizeof(float)*6);
			nsvg__xformInverse(shape->stroke.gradient->xform, t);
		}

		if (!shape->strokeScaled)
		{
			shape->strokeWidth *= avgs;
			shape->strokeDashOffset *= avgs;
			for (i = 0; i < shape->strokeDashCount; i++)
				shape->strokeDashArray[i] *= avgs;
			shape->strokeScaled = 1;
		}
	}
}

static void nsvg__createGradients(NSVGparser* p)
{
	NSVGshapeNode* shapeNode;
	NSVGshape* shape;

	for (shapeNode = p->image->shapes; shapeNode != NULL; shapeNode = shapeNode->next) {
		shape = shapeNode->shape;
		if (shape == NULL) continue;

		if (shape->fill.type == NSVG_PAINT_UNDEF) {
			if ((shape->fillGradient != NULL) && (shape->fillGradient->id[0] != '\0')) {
				float inv[6], localBounds[4];
				nsvg__xformInverse(inv, shape->xform);
				nsvg__getLocalBounds(localBounds, shape, inv);
				shape->fill.gradient = nsvg__createGradient(p, shape->fillGradient->id, localBounds, shape->xform, &shape->fill.type);
			}
			if (shape->fill.type == NSVG_PAINT_UNDEF) {
				shape->fill.type = NSVG_PAINT_NONE;
			}
			memcpy(&shape->orig.fill, &shape->fill, sizeof(shape->orig.fill));
		}
		if (shape->stroke.type == NSVG_PAINT_UNDEF) {
			if ((shape->strokeGradient != NULL) && (shape->strokeGradient->id[0] != '\0')) {
				float inv[6], localBounds[4];
				nsvg__xformInverse(inv, shape->xform);
				nsvg__getLocalBounds(localBounds, shape, inv);
				shape->stroke.gradient = nsvg__createGradient(p, shape->strokeGradient->id, localBounds, shape->xform, &shape->stroke.type);
			}
			if (shape->stroke.type == NSVG_PAINT_UNDEF) {
				shape->stroke.type = NSVG_PAINT_NONE;
			}
			memcpy(&shape->orig.stroke, &shape->stroke, sizeof(shape->orig.stroke));
		}
	}
}

static void nsvg__findShapeParents(NSVGparser* p)
{
	NSVGshapeNode* shape;
	NSVGshapeNode* parent;

	// Find the parent of the shape by their depth (next shape with lower depth).
	for (shape = p->shapesTail; shape != NULL; shape = shape->prev) {
		for (parent = shape->prev; parent != NULL && parent->shapeDepth >= shape->shapeDepth; parent = parent->prev);
		shape->parent = parent;
	}
}

NSVGimage* nsvgParse(char* input, const char* units, float dpi)
{
	NSVGparser* p;
	NSVGimage* ret = 0;

	p = nsvg__createParser();
	if (p == NULL) {
		return NULL;
	}
	p->image->dpi = dpi;
	strncpy(p->image->units, units, 3);

	nsvg__parseXML(input, nsvg__startElement, nsvg__endElement, nsvg__content, p);

	// Create gradients after all definitions have been parsed
	nsvg__createGradients(p);

	// Find the shape parents.
	nsvg__findShapeParents(p);

	// Scale to viewBox
	nsvg__scaleToViewbox(p->image);

	ret = p->image;

	nsvg__deleteParser(p);

	return ret;
}

NSVGimage* nsvgParseFromFile(const char* filename, const char* units, float dpi)
{
	FILE* fp = NULL;
	size_t size;
	char* data = NULL;
	NSVGimage* image = NULL;

	fp = fopen(filename, "rb");
	if (!fp) goto error;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)malloc(size+1);
	if (data == NULL) goto error;
	if (fread(data, 1, size, fp) != size) goto error;
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	image = nsvgParse(data, units, dpi);
	free(data);

	return image;

error:
	if (fp) fclose(fp);
	if (data) free(data);
	if (image) nsvgDelete(image);
	return NULL;
}

NSVGpath* nsvgDuplicatePath(NSVGpath* p)
{
    NSVGpath* res = NULL;

    if (p == NULL)
        return NULL;

    res = (NSVGpath*)malloc(sizeof(NSVGpath));
    if (res == NULL) goto error;
    memset(res, 0, sizeof(NSVGpath));

    res->pts = (float*)malloc(p->npts*2*sizeof(float));
    if (res->pts == NULL) goto error;
    memcpy(res->pts, p->pts, p->npts * sizeof(float) * 2);
    res->npts = p->npts;

    memcpy(res->bounds, p->bounds, sizeof(p->bounds));

    res->closed = p->closed;

    return res;

error:
    if (res != NULL) {
        free(res->pts);
        free(res);
    }
    return NULL;
}

void nsvgDelete(NSVGimage* image)
{
	NSVGshapeNode *next, *shapeNode;
	NSVGshape *shape;
	if (image == NULL) return;
	shapeNode = image->shapes;
	while (shapeNode != NULL) {
		next = shapeNode->next;
		shape =  shapeNode->shape;
		if (shape != NULL) {
			nsvg__deletePaths(image, shape->paths);
			nsvg__deletePaint(image, &shape->fill);
			nsvg__deletePaint(image, &shape->stroke);
			nsvg__free(image, shape->id, sizeof(NSVGid));
			nsvg__free(image, shape->fillGradient, sizeof(NSVGid));
			nsvg__free(image, shape->strokeGradient, sizeof(NSVGid));
			nsvg__free(image, shape, sizeof(NSVGshape));
		}
		while (shapeNode->animates != NULL) {
			NSVGanimate* next = shapeNode->animates->next;
			nsvg__free(image, shapeNode->animates, sizeof(NSVGanimate));
			shapeNode->animates = next;
		}
		nsvg__free(image, shapeNode, sizeof(NSVGshapeNode));
		shapeNode = next;
	}
	free(image);
}

void nsvg__animateApplyTransform(float* xform, float* args, int na, char type, char additive)
{
	float xform2[6];

	nsvg__xformIdentity(xform2);

	if (type == NSVG_ANIMATE_TYPE_TRANSFORM_TRANSLATE) {
		nsvg__xformSetTranslation(xform2, args[0], args[1]);
	} else if (type == NSVG_ANIMATE_TYPE_TRANSFORM_SCALE) {
		nsvg__xformSetScale(xform2, args[0], args[1]);
	} else if (type == NSVG_ANIMATE_TYPE_TRANSFORM_ROTATE) {
		if (na > 1) {
			nsvg__xformSetNonCenterRotation(xform2, args[0], args[1], args[2]);
		} else {
			nsvg__xformSetRotation(xform2, args[0]);
		}
	} else if (type == NSVG_ANIMATE_TYPE_TRANSFORM_SKEWX) {
		nsvg__xformSetSkewX(xform2, args[0]);
	} else if (type == NSVG_ANIMATE_TYPE_TRANSFORM_SKEWY) {
		nsvg__xformSetSkewY(xform2, args[0]);
	}

	if (additive == NSVG_ANIMATE_ADDITIVE_REPLACE) {
		nsvg__xformIdentity(xform);
	}
	nsvg__xformPremultiply(xform, xform2);
}

void nsvg__animateApplyPaint(NSVGpaint* paint, float* args, char additive)
{
	int r = (int)args[0] & 0xFF;
	int g = (int)args[1] & 0xFF;
	int b = (int)args[2] & 0xFF;

	if (paint->type != NSVG_PAINT_COLOR) return;

	if (additive == NSVG_ANIMATE_ADDITIVE_SUM) {
		r += paint->color & 0xFF;
		g += (paint->color >> 8) & 0xFF;
		b += (paint->color >> 16) & 0xFF;
		r = r < 0xFF ? r : 0xFF;
		g = g < 0xFF ? g : 0xFF;
		b = b < 0xFF ? b : 0xFF;
	}
	paint->color = paint->color & 0xFF000000 | NSVG_RGB(r,g,b);
}

void nsvg__animateApplyOpacity(NSVGpaint* paint, float* args, char additive)
{
	unsigned int a = (int)(args[0] * 255) & 0xFF;

	if (paint->type != NSVG_PAINT_COLOR) return;

	if (additive == NSVG_ANIMATE_ADDITIVE_SUM) {
		a += (paint->color >> 24) & 0xFF;
		a = a < 0xFF ? a : 0xFF;
	}
	paint->color = paint->color & 0x00FFFFFF | (a << 24);
}

void nsvg__animateApplyValue(float* value, float* args, char additive)
{
	if (additive == NSVG_ANIMATE_ADDITIVE_SUM) {
		*value += args[0];
	} else {
		*value = args[0];
	}
}

char nsvg__animateApplyGroup(NSVGshape* shape, NSVGanimate* animate, long timeMs)
{
	NSVGpath* path;
	long relativeTime;
	float progression;
	float args[10];
	float splineValue;
	char animateApplied;
	char groupHasAnimate;
	char scaleStroke;
	char ended;
	int count;
	int na;
	int i;

	animateApplied = 0;
	groupHasAnimate = 0;
	for (; animate != NULL; animate = animate->next) {

		if (animate->flags & NSVG_ANIMATE_FLAG_GROUP_FIRST) groupHasAnimate = 0;
		ended = 0;
		scaleStroke = 0;

		// Check if already applied another animation from same group.
		if (groupHasAnimate) continue;

		// Verify animation time.
		relativeTime = (timeMs - animate->begin) % animate->groupDur + animate->begin;
		if (relativeTime < animate->begin) continue;
		if (relativeTime >= (animate->begin + animate->dur)) ended = 1;
		if (animate->end > 0 && timeMs >= animate->end) ended = 1;
		if (animate->repeatCount >= 0) {
			count = (timeMs - animate->begin) / animate->groupDur;
			if (count + 1 > animate->repeatCount) ended = 1;
		}

		// If ended, apply this only if last and fill is freeze.
		if (ended && !(animate->flags & NSVG_ANIMATE_FLAG_GROUP_LAST && animate->fill == NSVG_ANIMATE_FILL_FREEZE)) continue;

		groupHasAnimate = 1;

		// Calculate relative progression.
		progression = 1;
		if (!ended) {
			if (animate->calcMode != NSVG_ANIMATE_CALC_MODE_DISCRETE) {
				progression = (float)(relativeTime - animate->begin) / (float)animate->dur;
			}

			// Handle spline calculation.
			if (animate->calcMode == NSVG_ANIMATE_CALC_MODE_SPLINE) {
				splineValue = nsvg__evalBezier(progression, 0, animate->spline[0], animate->spline[2], 1); // time (input is linear progression)
				progression = nsvg__evalBezier(splineValue, 0, animate->spline[1], animate->spline[3], 1); // value (input is spline time)
			}
		}

		// Apply the value interpolation.
		for (i = 0; i < 10; i++) {
			args[i] = animate->src[i] + (animate->dst[i] - animate->src[i]) * progression;
		}

		// Check if this is a transform.
		if (animate->type == NSVG_ANIMATE_TYPE_TRANSFORM_TRANSLATE ||
			animate->type == NSVG_ANIMATE_TYPE_TRANSFORM_SCALE ||
			animate->type == NSVG_ANIMATE_TYPE_TRANSFORM_ROTATE ||
			animate->type == NSVG_ANIMATE_TYPE_TRANSFORM_SKEWX ||
			animate->type == NSVG_ANIMATE_TYPE_TRANSFORM_SKEWY) {
			// Transform the strokes.
			na = (animate->srcNa > animate->dstNa) ? animate->srcNa : animate->dstNa;
			nsvg__animateApplyTransform(shape->xform, args, na, animate->type, animate->additive);
			scaleStroke = 1;

			// Transform the paths.
			for (path = shape->paths; path != NULL; path = path->next) {
				nsvg__animateApplyTransform(path->xform, args, na, animate->type, animate->additive);
				nsvg__transformPath(path, path->xform);
				path->scaled = 0;
			}
		} else if (animate->type == NSVG_ANIMATE_TYPE_FILL) {
			nsvg__animateApplyPaint(&shape->fill, args, animate->additive);
		} else if (animate->type == NSVG_ANIMATE_TYPE_STROKE) {
			nsvg__animateApplyPaint(&shape->stroke, args, animate->additive);
		} else if (animate->type == NSVG_ANIMATE_TYPE_OPACITY) {
			if (animate->additive == NSVG_ANIMATE_ADDITIVE_SUM) {
				shape->opacity += args[0];
			}
			shape->opacity = args[0];
			shape->opacity = (shape->opacity > 1) ? 1 : shape->opacity;
		} else if (animate->type == NSVG_ANIMATE_TYPE_FILL_OPACITY) {
			nsvg__animateApplyOpacity(&shape->fill, args, animate->additive);
		} else if (animate->type == NSVG_ANIMATE_TYPE_STROKE_OPACITY) {
			nsvg__animateApplyOpacity(&shape->stroke, args, animate->additive);
		} else if (animate->type == NSVG_ANIMATE_TYPE_STROKE_WIDTH) {
			nsvg__animateApplyValue(&shape->strokeWidth, args, animate->additive);
		} else if (animate->type == NSVG_ANIMATE_TYPE_STROKE_DASHOFFSET) {
			nsvg__animateApplyValue(&shape->strokeDashOffset, args, animate->additive);
			scaleStroke = 1;
		} else if (animate->type == NSVG_ANIMATE_TYPE_STROKE_DASHARRAY) {
			if (animate->srcNa != animate->dstNa) {
				memcpy(shape->strokeDashArray, animate->dst, sizeof(float)*(animate->dstNa-1));
			} else {
				memcpy(shape->strokeDashArray, args, sizeof(float)*(animate->dstNa-1));
			}
			shape->strokeDashCount = (char)args[animate->dstNa-1];
		}

		if (scaleStroke) nsvg__scaleShapeStroke(shape, shape->xform);

		animateApplied = 1;
	}

	return animateApplied;
}

char nsvg__animateApplyGroupRecursive(NSVGshape* shape, NSVGshapeNode* shapeNode, long timeMs)
{
	char animateApplied = 0;

	if (shapeNode->parent != NULL) {
		animateApplied |= nsvg__animateApplyGroupRecursive(shape, shapeNode->parent, timeMs);
	}
	
	animateApplied |= nsvg__animateApplyGroup(shape, shapeNode->animates, timeMs);

	return animateApplied;
}

void nsvg__animateReset(NSVGshape* shape)
{
	NSVGpath* path;

	// Reset paint.
	shape->opacity = shape->orig.opacity;
	memcpy(&shape->fill, &shape->orig.fill, sizeof(shape->fill));
	memcpy(&shape->stroke, &shape->orig.stroke, sizeof(shape->stroke));
	shape->strokeWidth = shape->orig.strokeWidth;
	shape->strokeDashOffset = shape->orig.strokeDashOffset;
	shape->strokeDashCount = shape->orig.strokeDashCount;
	memcpy(shape->strokeDashArray, shape->orig.strokeDashArray, sizeof(shape->strokeDashArray));

	// Reset the shape transforms.
	memcpy(shape->xform, shape->orig.xform, sizeof(shape->xform));

	// Reset all path transform and points.
	for (path = shape->paths; path != NULL; path = path->next) {
		memcpy(path->xform, path->orig.xform, sizeof(path->xform));
		nsvg__transformPath(path, path->xform);
	}
}

char nsvgAnimate(NSVGimage* image, long timeMs)
{
	NSVGshapeNode* shapeNode = image->shapes;
	NSVGshape* shape;
	char retVal = 0;
	int count;

	for (shapeNode = image->shapes; shapeNode != NULL; shapeNode = shapeNode->next) {
		shape = shapeNode->shape;
		if (shape == NULL) continue;

		// Reset the shape transforms.
		nsvg__animateReset(shape);

		// Apply the shape transformations recursively (including parents).
		nsvg__animateApplyGroupRecursive(shape, shapeNode, timeMs);

		// Update shape bounds.
		nsvg__updateShapeBounds(shape);

		if (shapeNode->animates != NULL) {
			retVal |= 1;
		}
	}

	nsvg__scaleToViewbox(image);

	return retVal;
}

#endif // NANOSVG_IMPLEMENTATION

#endif // NANOSVG_H
