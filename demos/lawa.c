#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

#define N 4
#define F 20

#define PALSZ 512
color palette[PALSZ];

void init_lawa(int *lawa, int *lawad, int w, int h)
{
	int i;

	for (i = 0; i < w*h; i++) {
		lawa[i] = rand2(48, PALSZ - 1);
		lawad[i] = rand2(-F, F + 1);
	}
}

void fixup_lawa(int *lawa, int *lawad)
{
	if (*lawa < 48) {
		*lawa = 48;
		*lawad = rand2(0, F + 1);
	} else if (*lawa > PALSZ - 1) {
		*lawa = PALSZ - 1;
		*lawad = rand2(-F, 0);
	}
	if (!(rand()%30))
		*lawad = rand2(-F, F + 1);
}

void next_lawa(int *lawa, int *lawad, int w, int h)
{
	int i;

	for (i = 1; i < w; i++) {
		lawa[i] = ((lawa[i - 1] + lawa[i + 1])/2) + lawad[i];
		fixup_lawa(&lawa[i], &lawad[i]);
	}
	for (i = w; i < w*(h - 1); i++) {
		lawa[i] = ((lawa[i - 1] + lawa[i + 1] + lawa[i - w] + lawa[i + w])/4) + lawad[i];
		fixup_lawa(&lawa[i], &lawad[i]);
	}
	for (i = w*(h - 1); i < h*w; i++) {
		lawa[i] = ((lawa[i - 1] + lawa[i + 1])/2) + lawad[i];
		fixup_lawa(&lawa[i], &lawad[i]);
	}
}

void render_lawa(struct fb *dst, int *lawa, int w, int h)
{
	int i, j;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++)
			rect(dst, i*N, j*N, N - 1, N - 1, palette[lawa[i]]);
		lawa += w;
	}
}

void fade_lawa(struct fb *dst, int n)
{
	int i;

	for (i = 0; i < dst->pitch*dst->h; i++)
		dst->pixbuf[i] = psub(dst->pixbuf[i], rgb(n, n, n));
}

int main()
{
	int i;
	struct fb *fb = fbopen(0, 0);
	struct fb *vfb = vfballoc(fb->w, fb->h);
	int w = vfb->w/N;
	int h = vfb->h/N;
	int lawa[w*h], lawad[w*h];

	for (i = 0; i < PALSZ; i++)
		palette[i] = gradient(i, PALSZ, (color[]){0x000000, 0xff0000, 0xffff00}, 3);

	init_lawa(lawa, lawad, w, h);
	for (i = 0; i < 32; i++)
		next_lawa(lawa, lawad, w, h);

	while (!fbread(fb)) {
		next_lawa(lawa, lawad, w, h);
		render_lawa(vfb, lawa, w, h);
		blit(fb, 0, 0, vfb, pmov);
	}
	while (!fbread(fb));

	for (i = 0; i < 256; i += 4) {
		fade_lawa(vfb, 4);
		blit(fb, 0, 0, vfb, pmov);
		if (fbread(fb))
			break;
	}
	fbclose(fb);
	return 0;
}
