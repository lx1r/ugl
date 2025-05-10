#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

void init_shade(struct fb *shade)
{
	int i, c, r = shade->w/2;

	cls(shade);
	for (i = 0; i < shade->w; i += 4) {
		c = 256*i/shade->w;
		spot(shade, r, r, r - 3*i/10, rgb(c, c, c));
	}
}

void init_texture(struct fb *texture)
{
	int y;

	for (y = 0; y < texture->h; y++)
		hline(texture, 0, y, texture->w,
		      gradient(y, texture->h, (color[]){0x000000, 0x1c1191, 0xdf0b12, 0xefeb37, 0x000000}, 5));
}

void init_tunnel(struct fb *tunnel)
{
	int x, y;
	int u, v;
	float angle, radius;
	pixel *p = tunnel->pixbuf;

	for (y = -tunnel->h/2; y < tunnel->h/2; y++) {
		for (x = -tunnel->w/2; x < tunnel->w/2; x++) {
			radius = sqrtf(x*x + y*y);
			if (radius < 1)
				radius = 1;
			angle = atan2f(y, x);
			v = 100000.0f/radius;
			u = (angle + M_PI)*128.0f/M_PI;
			*p++ = (u & 0xff) + ((v & 0xff) << 8);
		}
	}
}

void render_tunnel(struct fb *vfb, struct fb *tunnel, struct fb *texture, float time)
{
	int i;
	int u = 256*cosf(time);
	int v = 512*time;

	for (i = 0; i < vfb->w*vfb->h; i++)
		vfb->pixbuf[i] = texture->pixbuf[(tunnel->pixbuf[i] + u + (v << 8)) & 0xffff];
}

int main()
{
	struct fb *fb = fbopen(0, 0);
	struct fb *vfb = vfballoc(fb->w, fb->h);
	struct fb *tunnel = vfballoc(fb->w, fb->h);
	struct fb *texture = vfballoc(256, 256);
	struct fb *shade = vfballoc(fb->h/8, fb->h/8);
	float time = 0;

	init_shade(shade);
	init_texture(texture);
	init_tunnel(tunnel);

	while (!fbread(fb)) {
		render_tunnel(vfb, tunnel, texture, time);
		blit(vfb, fb->w/2 - shade->w/2, fb->h/2 - shade->h/2, shade, psub);
		blit(fb, 0, 0, vfb, pmov);
		time += 0.02f;
	}
	fbclose(fb);
}
