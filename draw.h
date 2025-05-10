#ifndef UGL_DRAW_H
#define UGL_DRAW_H

#include "ugl/fb.h"
#include "ugl/math.h"

static inline void cls(struct fb *dst)
{
	int i;

	fblock(dst);
	for (i = 0; i < dst->pitch*dst->h; i++)
		dst->pixbuf[i] = 0;
	fbunlock(dst);
}

static inline color getpixel(struct fb *dst, int x0, int y0)
{
	return *(dst->pixbuf + y0*dst->pitch + x0);
}

static inline void putpixel(struct fb *dst, int x0, int y0, color cl)
{
	*(dst->pixbuf + y0*dst->pitch + x0) = cl;
}

static inline void hline(struct fb *dst, int x0, int y0, int w, color cl)
{
	int x;
	pixel *p;

	p = dst->pixbuf + y0*dst->pitch + x0;
	for (x = 0; x < w; x++)
		*p++ = cl;
}

static inline void vline(struct fb *dst, int x0, int y0, int h, color cl)
{
	int y;
	pixel *p;

	p = dst->pixbuf + y0*dst->pitch + x0;
	for (y = 0; y < h; y++, p += dst->pitch)
		*p = cl;
}

static inline void line(struct fb *dst, int x0, int y0, int x1, int y1, color cl)
{
	fixed x = itofix(x0);
	fixed y = itofix(y0);
	fixed dx = 0, dy = 0;

	int w = x1 - x0;
	int h = y1 - y0;
	int len = max(abs(w), abs(h));

	if (len) {
		dx = frac(w, len);
		dy = frac(h, len);
	}

	for (; len >= 0; len--) {
		putpixel(dst, fixtoi(x), fixtoi(y), cl);
		x += dx;
		y += dy;
	}
}

static inline void spot(struct fb* dst, int x0, int y0, int r, color cl)
{
	int x, y, d;

	for (x = 0, y = r, d = 3 - (r << 1); x <= y; x++) {
		hline(dst, -x + x0,  y + y0, (x << 1) + 1, cl);
		hline(dst, -x + x0, -y + y0, (x << 1) + 1, cl);
		hline(dst, -y + x0,  x + y0, (y << 1) + 1, cl);
		hline(dst, -y + x0, -x + y0, (y << 1) + 1, cl);
		if (d < 0)
			d = d + (x << 2) + 6;
		else
			d = d + ((x - (y--)) << 2) + 10;
	}
}

static inline void rect(struct fb* dst, int x0, int y0, int w, int h, color cl)
{
	int y;

	for (y = 0; y < h; y++)
		hline(dst, x0, y0 + y, w, cl);
}

#endif
