#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

#define BLOBS 32

struct blob {
	vec2 pos;
	vec2 vel;
	vec2 acc;
} b[BLOBS];

int main()
{
	int i;
	struct fb *fb = fbopen(0, 0);
	int rad = fb->w/12;
	struct fb *vfb = vfballoc(fb->w, fb->h);
	struct fb *blob = vfballoc(2*rad, 2*rad);
	vec2 plb = vec2(0, 0);
	vec2 pub = vec2(vfb->w - 1, vfb->h - 1);
	vec2 vlb = vec2(-20, -20);
	vec2 vub = vec2(20, 20);
	vec2 alb = vec2(-1, -1);
	vec2 aub = vec2(1, 1);
	vec2 mask;

	cls(blob);
	for (i = 0; i < rad; i++)
		spot(blob, rad, rad, rad - i,
		     gradient(4*i, 4*rad, (color[]){0x000000, 0xa00000, 0xffff00}, 3));

	for (i = 0; i < BLOBS; i++) {
		b[i].pos = v2rand(plb, pub);
		b[i].vel = v2rand(vlb, vub);
		b[i].acc = v2rand(alb, aub);
	}

	while (!fbread(fb)) {
		cls(vfb);
		for (i = 0; i < BLOBS; i++) {
			b[i].vel = v2clamp(v2add(b[i].vel, b[i].acc), vlb, vub, &mask);
			b[i].acc = v2mask(b[i].acc, v2rand(alb, aub), mask);
			b[i].pos = v2clamp(v2add(b[i].pos, b[i].vel), plb, pub, &mask);
			b[i].vel = v2mask(b[i].vel, v2rand(vlb, vub), mask);
			blit(vfb, (int)b[i].pos.x - rad, (int)b[i].pos.y - rad, blob, padd);
		}
		blit(fb, 0, 0, vfb, pmov);
	}
	fbclose(fb);
	return 0;
}
