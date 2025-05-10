#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

#define ALB -3
#define AUB  3
#define VLB -15
#define VUB  15
#define CLB  0
#define CUB  511

#define NX 5
#define NY 3

color palette[CUB + 1];

struct {
	float cl;
	float vel;
	float acc;
} grid[NX][NY];

void plasma(struct fb *dst, int x0, int y0, int x1, int y1)
{
	if (((x1 - x0) <= 1) && ((y1 - y0) <= 1))
		return;

	putpixel(dst, (x0 + x1)/2, y0,          pavg(getpixel(dst, x0, y0), getpixel(dst, x1, y0)));
	putpixel(dst, (x0 + x1)/2, y1,          pavg(getpixel(dst, x0, y1), getpixel(dst, x1, y1)));
	putpixel(dst, x0,          (y0 + y1)/2, pavg(getpixel(dst, x0, y0), getpixel(dst, x0, y1)));
	putpixel(dst, x1,          (y0 + y1)/2, pavg(getpixel(dst, x1, y0), getpixel(dst, x1, y1)));
	putpixel(dst, (x0 + x1)/2, (y0 + y1)/2, pavg4(getpixel(dst, x0, y0), getpixel(dst, x1, y1),
						      getpixel(dst, x1, y0), getpixel(dst, x0, y1)));

	plasma(dst, x0,          y0,          (x0 + x1)/2, (y0 + y1)/2);
	plasma(dst, (x0 + x1)/2, y0,          x1,          (y0 + y1)/2);
	plasma(dst, x0,          (y0 + y1)/2, (x0 + x1)/2, y1);
	plasma(dst, (x0 + x1)/2, (y0 + y1)/2, x1,          y1);
}

int main()
{
	int i, j;
	int x[NX], y[NY];
	struct fb *fb, *vfb;

	fb = fbopen(0, 0);
	vfb = vfballoc(fb->w, fb->h);

	x[0] = 0;
	for (i = 1; i < NX; i++)
		x[i] = x[i - 1] + vfb->w/(NX - 1);
	x[NX - 1]--;

	y[0] = 0;
	for (i = 1; i < NY; i++)
		y[i] = y[i - 1] + vfb->h/(NY - 1) - 1;
	y[NY - 1]--;

	for (i = 0; i < CUB + 1; i++)
		palette[i] = gradient(i, CUB + 1, (color[]){0xffe010, 0xff2020, 0x20ef10, 0x2020ff}, 4);

	for (j = 0; j < NY; j++) {
		for (i = 0; i < NX; i++) {
			grid[i][j].cl = frand2(CLB, CUB);
			grid[i][j].vel = frand2(VLB, VUB);
			grid[i][j].acc = frand2(ALB, AUB);
			putpixel(vfb, x[i], y[j], palette[(int)grid[i][j].cl]);
		}
	}

	while (!fbread(fb)) {

		for (j = 0; j < NY - 1; j++)
			for (i = 0; i < NX - 1; i++)
				plasma(vfb, x[i], y[j], x[i + 1], y[j + 1]);

		blit(fb, 0, 0, vfb, pmov);

		for (j = 0; j < NY; j++) {
			for (i = 0; i < NX; i++) {
				float clamped;
				grid[i][j].vel = fclamp(grid[i][j].vel + grid[i][j].acc, VLB, VUB, &clamped);
				if (clamped) grid[i][j].acc = frand2(ALB, AUB);
				grid[i][j].cl = fclamp(grid[i][j].cl + grid[i][j].vel, CLB, CUB, &clamped);
				if (clamped) grid[i][j].vel = frand2(VLB, VUB);
				putpixel(vfb, x[i], y[j], palette[(int)grid[i][j].cl]);
			}
		}
	}
	fbclose(fb);
	return 0;
}
