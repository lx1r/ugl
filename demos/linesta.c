#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

#define N 4
#define VLB -20
#define VUB  20

struct particle {
	vec2 pos;
	vec2 vel;
	float cl, dcl;
} p[N];

int main()
{
	int i;
	struct fb *fb, *vfb;
	vec2 plb, pub;
	vec2 vlb = vec2(VLB, VLB);
	vec2 vub = vec2(VUB, VUB);

	fb = fbopen(0, 0);
	vfb = vfballoc(fb->w, fb->h);
	plb = vec2(5, 5);
	pub = vec2(vfb->w - 5, vfb->h - 5);

	for (i = 0; i < N; i++) {
		p[i].pos = v2rand(plb, pub);
		p[i].vel = v2rand(vlb, vub);
		p[i].cl = frand2(10, 500);
		p[i].dcl = frand2(VLB, VUB);
	}

	while (!fbread(fb)) {

		for (i = 0; i < N; i++) {
			float clamped;
			vec2 mask;
			p[i].pos = v2clamp(v2add(p[i].pos, p[i].vel), plb, pub, &mask);
			p[i].vel = v2mask(p[i].vel, vec2(-p[i].vel.x, -p[i].vel.y), mask);
			p[i].cl = fclamp(p[i].cl + p[i].dcl, 0, 512, &clamped);
			if (clamped)
				p[i].dcl = -p[i].dcl;
		}
		blur(vfb, 1, 1, vfb);

		for (i = 0; i < N; i += 2) {
			line(vfb, (int)p[i].pos.x, (int)p[i].pos.y, (int)p[i + 1].pos.x, (int)p[i + 1].pos.y,
			     gradient((int)p[i].cl, 512, (color[]){0xff0000, 0x00ff00, 0x0000ff}, 3));
		}
		blit(fb, 0, 0, vfb, pmov);
	}
	fbclose(fb);
	return 0;
}
