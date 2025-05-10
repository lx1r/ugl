#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

#define PALSZ 512
color palette[PALSZ];

static inline pixel pal(pixel dst, pixel src)
{
	return palette[src];
}

void render_fire_line(struct fb *dst)
{
	int x;

	for (x = 4; x < dst->w - 4; x += 4)
		rect(dst, x, dst->h - 2, 4, 2, rand() & (PALSZ - 1));
}

void render_fire_flame(struct fb *dst)
{
	int i;
	pixel *p = dst->pixbuf;

	for (i = dst->w; i < dst->w*(dst->h - 2); i++) {
		p[i] = (p[i + dst->w - 1] + p[i + dst->w] +
				  p[i + dst->w + 1] + p[i + 2*dst->w])/4;
		if (p[i] > 1)
			p[i] -= 1;
	}
}

int main()
{
	int i;
	struct fb *fb, *fire;

	fb = fbopen(0, 0);
	fire = vfballoc(fb->w, 250);
	cls(fb);

	for (i = 0; i < PALSZ; i++)
		palette[i] = gradient(i, PALSZ, (color[]){0x000000, 0x121212, 0xff1212, 0xffff00, 0xffffff}, 5);

	for (i = 0; i < fire->h; i++) {
		render_fire_line(fire);
		render_fire_flame(fire);
	}

	while (!fbread(fb)) {
		render_fire_line(fire);
		render_fire_flame(fire);
		blit(fb, 0, fb->h - fire->h + 8, fire, pal);
	}
	fbclose(fb);
	return 0;
}
