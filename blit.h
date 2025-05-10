#ifndef UGL_BLIT_H
#define UGL_BLIT_H

#include "ugl/fb.h"
#include "ugl/math.h"

#define RENDER(dst, p0, x0, y0, w0, h0, src, p1, x1, y1, w1, h1, ROP)\
do {\
	int x, y, w, h;\
	\
	if (x0 < 0) { x1 -= x0; w1 += x0; x0 = 0; }\
	if (y0 < 0) { y1 -= y0; h1 += y0; y0 = 0; }\
	\
	w = min(w0, w1);\
	h = min(h0, h1);\
	\
	if (fblock(dst)) break;\
	p0 = (dst)->pixbuf + y0*(dst)->pitch + x0;\
	p1 = (src)->pixbuf + y1*(src)->pitch + x1;\
	for (y = 0; y < h; y++) {\
		for (x = 0; x < w; x++) {\
			*p0 = ROP;\
			p0++; p1++;\
		}\
		p0 += (dst)->pitch - w;\
		p1 += (src)->pitch - w;\
	}\
	fbunlock(dst);\
} while (0)

static inline void blit(struct fb *dst, int x0, int y0, struct fb *src,
			pixel (*rop)(pixel, pixel))
{
	int w0 = min(dst->w, dst->w - x0);
	int h0 = min(dst->h, dst->h - y0);
	int x1 = 0, y1 = 0;
	int w1 = src->w, h1 = src->h;
	pixel *p0, *p1;

	if (!rop)
		rop = pmov;

	RENDER(dst, p0, x0, y0, w0, h0, src, p1, x1, y1, w1, h1, rop(*p0, *p1));
}

static inline void blur(struct fb *dst, int x0, int y0, struct fb *src)
{
	int w0 = min(dst->w, dst->w - x0);
	int h0 = min(dst->h, dst->h - y0);
	int x1 = 1, y1 = 1;
	int w1 = src->w - 2, h1 = src->h - 2;
	pixel *p0, *p1;

	RENDER(dst, p0, x0, y0, w0, h0, src, p1, x1, y1, w1, h1,
	       pavg4(*(p1 - 1), *(p1 + 1), *(p1 - src->pitch), *(p1 + src->pitch)));
}

#undef RENDER

#endif
