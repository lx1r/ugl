#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

#define DCL 2
#define LINES 4

#define PALSZ 256
color palette[PALSZ];

static inline pixel pal(pixel dst, pixel src)
{
	return palette[src];
}

static inline color addm(color cl, int d)
{
	return (cl + d) & (PALSZ - 1);
}

void render_nocturne_line(struct fb *dst, int y)
{
	int x, d;
	color up, cl;

	d = rand2(-DCL, DCL);
	up = getpixel(dst, 0, y - 1);
	cl = addm(up, d);
	putpixel(dst, 0, y, cl);
	for (x = 1; x < dst->w - 1; x++) {
		d = rand2(-DCL, DCL);
		if (rand()%10)
			up = getpixel(dst, x, y - 1);
		else
			up = (getpixel(dst, x - 1, y - 1) +
			      getpixel(dst, x + 1, y - 1) +
			      rand()%2)/2;
		cl = addm(up, d);
		putpixel(dst, x, y, cl);
	}
	d = rand2(-DCL, DCL);
	up = getpixel(dst, dst->w - 1, y - 1);
	cl = addm(up, d);
	putpixel(dst, dst->w - 1, y, cl);
}

void init_nocturne(struct fb *dst, color cl)
{
	int x, y, d;

	putpixel(dst, 0, 0, cl);
	for (x = 1; x < dst->w; x++) {
		d = rand2(-4*DCL, 4*DCL);
		cl = addm(cl, d);
		putpixel(dst, x, 0, cl);
	}
	for (y = 1; y < dst->h; y++)
		render_nocturne_line(dst, y);
}

static inline void scrollup(struct fb *dst, int n)
{
	int i;
	pixel *p = dst->pixbuf;

	for (i = 0; i < dst->pitch*(dst->h - n); i++)
		p[i] = p[i + n*dst->pitch];
}

int main()
{
	int i;
	struct fb *fb, *vfb;

	fb = fbopen(0, 0);
	vfb = vfballoc(fb->w, fb->h);
	for (i = 0; i < PALSZ; i++)
		palette[i] = gradient(i, PALSZ, (color[]){0x000010, 0x0000b0, 0x000020, 0x081eec, 0x000010}, 5);

	init_nocturne(vfb, PALSZ/4);

	while (!fbread(fb)) {
		scrollup(vfb, LINES);
		for (i = LINES; i > 0; i--)
			render_nocturne_line(vfb, vfb->h - i);
		blit(fb, 0, 0, vfb, pal);
	}
	fbclose(fb);
	return 0;
}
