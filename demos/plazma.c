#include "ugl/fb.h"
#include "ugl/blit.h"
#include "ugl/draw.h"
#include "ugl/math.h"

#define N 4

int main()
{
	int x, y, cl;
	int a = 3, b = 2, c = 5;
	int d = 4, e = 5, f = 3;
	int g = 1, h = 3, i = 2;
	int j = 5, k = 1, l = 4;
	int time1, time2, time3, time4;
	int angle1, angle2, angle3, angle4;
	int icostbl[256];
	color palette[256];
	struct fb *fb, *vfb;

	fb = fbopen(0, 0);
	vfb = vfballoc(fb->w, fb->h);

	for (x = 0; x < 256; x++) {
		palette[x] = gradient(x, 256,
			(color[]) {0xf002f2, 0x0000f1, 0x030303, 0xf1f0f3, 0x8791b4}, 5);
		icostbl[x] = (int)(32*cosf(2*M_PI/256*x));
	}

	time1 = c;
	time2 = f;
	time3 = i;
	time4 = l;

	while (!fbread(fb)) {
		angle3 = time3;
		angle4 = time4;
		for (y = 0; y < vfb->h/N; y++) {
			angle1 = time1;
			angle2 = time2;
			for (x = 0; x < vfb->w/N; x++) {

				cl = palette[
					32 + icostbl[(angle3 + 32 + icostbl[angle1 & 0xff]) & 0xff] +
					32 + icostbl[(angle4 + 32 + icostbl[angle2 & 0xff]) & 0xff] +
					32 + icostbl[(angle1 + 32 + icostbl[angle3 & 0xff]) & 0xff] +
					32 + icostbl[(angle2 + 32 + icostbl[angle4 & 0xff]) & 0xff]];

				rect(vfb, N*x, N*y, N - 1, N - 1, cl);

				angle1 = angle1 + a;
				angle2 = angle2 + d;
			}
			angle3 = angle3 + g;
			angle4 = angle4 + j;
		}
		time1 = time1 + b;
		time2 = time2 + e;
		time3 = time3 + h;
		time4 = time4 + k;
		blit(fb, 0, 0, vfb, pmov);
	}
	fbclose(fb);
	return 0;
}
