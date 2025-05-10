#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

#define N 2000
#define RMAX 2
#define ZMAX 15
#define SPEED 0.1f

vec3 star(int w, int h, int zmin)
{
	float a = frand()*2*M_PI;
	float rx = frand2(w/10, w*4);
	float ry = frand2(h/10, h*4);

	return vec3(rx*cosf(a), ry*sinf(a), frand2(zmin, ZMAX));
}

int main()
{
	int i, x, y, c, r;
	struct fb *fb = fbopen(0, 0);
	struct fb *vfb = vfballoc(fb->w, fb->h);
	int nr = fb->w;
	vec3 stars[nr];

	for (i = 0; i < N; i++)
		stars[i] = star(fb->w, fb->h, 2);

	while (!fbread(fb)) {
		for (i = 0; i < N; i++) {
			x = (int)(stars[i].x/stars[i].z + 0.5f) + vfb->w/2;
			y = (int)(stars[i].y/stars[i].z + 0.5f) + vfb->h/2;
			if ((x < RMAX) || (x >= (vfb->w - RMAX)) ||
			    (y < RMAX) || (y >= (vfb->h - RMAX)) || (stars[i].z < 0)) {
				stars[i] = star(fb->w, fb->h, ZMAX);
				continue;
			}
			r = lerp(stars[i].z, 0, ZMAX, RMAX, 1);
			c = (stars[i].z >= ZMAX/3) ?
				lerp(stars[i].z, ZMAX/3, ZMAX, 255, 0) : 255;
			spot(vfb, x, y, r, rgb(c, c, c));
			stars[i].z = stars[i].z - SPEED;
		}
		blit(fb, 0, 0, vfb, pmov);
		cls(vfb);
	}
	fbclose(fb);
	return 0;
}
