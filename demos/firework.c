#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

struct particle
{
	vec2 pos;
	vec2 vel;
	vec2 acc;
	float sz;
	float cl;
};

#define FIREWORKS 5
#define PARTICLES 300
#define PALSZ 512

struct firework
{
	int time;
	struct particle pt[PARTICLES];
	color palette[PALSZ];
};

void init_firework(struct fb *vfb, struct firework *f)
{
	int i, cx, cy;
	float r, a;
	color g[] = {rand(), rand(), rand()};

	for (i = 0; i < PALSZ; i++)
		f->palette[i] = gradient(i, PALSZ, g, 3);

	cx = rand2(vfb->w/5, vfb->w - vfb->w/5);
	cy = rand2(vfb->h/5, vfb->h - vfb->h/5);

	for (i = 0; i < PARTICLES; i++) {
		f->pt[i].cl = frand2(0, PALSZ);
		f->pt[i].sz = frand2(3, 5);
		f->pt[i].pos = vec2(cx, cy);
		a = frand()*2*M_PI;
		r = frand()*10;
		f->pt[i].vel = vec2(r*cosf(a), r*sinf(a));
		f->pt[i].acc = vec2(frand2(-0.005f, 0.005f), 0.05f);
	}
}

void draw_firework(struct fb *vfb, struct firework *f)
{
	int i;
	float flag;
	vec2 mask;
	vec2 plb = vec2(5, 5);
	vec2 pub = vec2(vfb->w - 5, vfb->h - 5);

	for (i = 0; i < PARTICLES; i++) {
		f->pt[i].vel = v2add(f->pt[i].vel, f->pt[i].acc);
		f->pt[i].pos = v2clamp(v2add(f->pt[i].pos, f->pt[i].vel), plb, pub, &mask);
		if (mask.x || mask.y) { f->pt[i].sz = 0; continue; }
		f->pt[i].cl = fclamp(f->pt[i].cl - 0.2f, 0, PALSZ, &flag);
		if (flag) continue;
		f->pt[i].sz = fclamp(f->pt[i].sz - 0.03f, 1, 5, &flag);
		if (flag) continue;
		spot(vfb, (int)f->pt[i].pos.x, (int)f->pt[i].pos.y, (int)f->pt[i].sz, f->palette[(int)f->pt[i].cl]);
	}
}

int main()
{
	int i;
	struct fb *fb, *vfb;
	struct firework f[FIREWORKS];

	fb = fbopen(0, 0);
	vfb = vfballoc(fb->w, fb->h);

	f[0].time = 0;
	for (i = 1; i < FIREWORKS; i++)
		f[i].time = rand2(-70, 0);

	while (!fbread(fb)) {
		cls(vfb);
		for (i = 0; i < FIREWORKS; i++) {
			if (f[i].time == 0)
				init_firework(vfb, &f[i]);
			if (f[i].time >= 0)
				draw_firework(vfb, &f[i]);
			f[i].time++;
			if (f[i].time >= 128)
				f[i].time = rand2(-128, 0);
		}
		blur(vfb, 1, 1, vfb);
		blit(fb, 0, 0, vfb, pmov);
	}
	fbclose(fb);
	return 0;
}
