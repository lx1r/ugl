#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

int main()
{
	int x, y, u, v;
	struct fb *fb = fbopen(0, 0);
	struct fb *vfb = vfballoc(fb->w, fb->h);
	struct fb *texture = vfballoc(256, 256);
	float a = 0, da = M_PI/256;
	float r = 80, dr = 1;

	cls(texture);
	rect(texture,   8,   8, 112, 112, 0xaf0000);
	rect(texture, 136,   8, 112, 112, 0xfacff0);
	rect(texture,   8, 136, 112, 112, 0x0004c0);
	rect(texture, 136, 136, 112, 112, 0xcfcf04);

	while (!fbread(fb)) {
		pixel *p = vfb->pixbuf;
		float cose = r*cosf(a)/256;
		float sine = r*sinf(a)/256;

		for (y = 0; y < vfb->h; y++) {
			for (x = 0; x < vfb->pitch; x++) {
				u = (int)(x*cose - y*sine) & 0xff;
				v = (int)(x*sine + y*cose) & 0xff;
				*p++ = getpixel(texture, u, v);
			}
		}
		a = (a + da);
		if (rand()%300 == 0)
			da = -da;
		r -= dr;
		if (r >= 0xff || r <= 0 || rand()%80 == 0)
			dr = -dr;
		blit(fb, 0, 0, vfb, pmov);
	}
	fbclose(fb);
	return 0;
}
