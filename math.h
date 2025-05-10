#ifndef UGL_MATH_H
#define UGL_MATH_H

#include <math.h>
#include <stdlib.h>

static inline int min(int a, int b)
{
	return b + ((a - b) & ((a - b) >> 31));
}

static inline int max(int a, int b)
{
	return a - ((a - b) & ((a - b) >> 31));
}

/* restrict a value to within a range */

static inline int clamp(int a, int lower, int upper)
{
	return max(lower, min(a, upper));
}

static inline float fclamp(float a, float lower, float upper, float *clamped)
{
	float c = fmaxf(lower, fminf(a, upper));
	if (clamped) *(clamped) = (a != c);
	return c;
}

/* pseudo-random number helpers */

static inline int rand2(int lower, int upper)
{
	return lower + rand()%(upper - lower + 1); /* [lower, upper] */
}

static inline float frand()
{
	return rand()/(float)RAND_MAX; /* [0.0, 1.0] */
}

static inline float frand2(float lower, float upper)
{
	return lower + frand()*(upper - lower); /* [lower, upper] */
}

/* fixed-point arithmetic */

typedef int fixed;

#define FIXBASE 16
#define FIXMASK 0xffff
#define FIXHALF 0x7fff

static inline fixed itofix(int a)
{
	return (a << FIXBASE);
}

static inline int fixtoi(fixed a)
{
	return (a + FIXHALF) >> FIXBASE;
}

static inline fixed ftofix(float a)
{
	return (int)(a*FIXMASK);
}

static inline float fixtof(fixed a)
{
	return (float)a/FIXMASK;
}

static inline fixed frac(int a, int b)
{
	return (a << FIXBASE)/b;
}

static inline fixed fixmul(fixed a, fixed b)
{
	return ((long long)a*b) >> FIXBASE;
}

static inline fixed fixdiv(fixed a, fixed b)
{
	return ((long long)a << FIXBASE)/b;
}

/* packed four 8-bit bytes arithmetic */

#define PLSB 0x01010101
#define PL2B 0x03030303
#define PMSB 0x80808080

typedef unsigned int pixel;

static inline pixel padd(pixel a, pixel b)
{
	pixel s = (a & ~PMSB) + (b & ~PMSB);
	pixel abs = (a | b) & PMSB;
	pixel c = abs & (s | (a & b));

	return s | abs | (abs - (c >> 7));
}

static inline pixel psub(pixel a, pixel b)
{
	pixel s = (a | PMSB) - (b & ~PMSB);
	pixel anb = a & ~b;
	pixel c = (anb | (s & ~(a ^ b))) & PMSB;

	return s & ((c & anb) | (c - (c >> 7)));
}

static inline pixel pavg(pixel a, pixel b)
{
	pixel ai = a & PLSB;
	pixel bi = b & PLSB;

	return ((a ^ ai) >> 1) + ((b ^ bi) >> 1);
}

static inline pixel pavg4(pixel a, pixel b, pixel c, pixel d)
{
	pixel ai = a & PL2B;
	pixel bi = b & PL2B;
	pixel ci = c & PL2B;
	pixel di = d & PL2B;

	return ((a ^ ai) >> 2) + ((b ^ bi) >> 2) + ((c ^ ci) >> 2) + ((d ^ di) >> 2);
}

static inline pixel pmov(pixel a, pixel b)
{
	return b;
}

static inline pixel pand(pixel a, pixel b)
{
	return a & b;
}

static inline pixel por(pixel a, pixel b)
{
	return a | b;
}

static inline pixel pxor(pixel a, pixel b)
{
	return a ^ b;
}

/* color value representation */

typedef unsigned int color;

static inline color rgb(unsigned char r, unsigned char g, unsigned char b)
{
	return (r << 16 | g << 8 | b << 0);
}

static inline color frgb(float r, float g, float b)
{
	return rgb(roundf(r*255.0f), roundf(g*255.0f), roundf(b*255.0f));
}

static inline unsigned char getr(color cl)
{
	return cl >> 16;
}

static inline unsigned char getg(color cl)
{
	return cl >> 8;
}

static inline unsigned char getb(color cl)
{
	return cl >> 0;
}

/* linear interpolation */
static inline float lerp(float x, float x0, float x1, float y0, float y1)
{
	return y0 + (x - x0)*(y1 - y0)/(x1 - x0);
}

/* cubic hermite spline */
static inline float hermite(float t, float p0, float p1, float m0, float m1)
{
	return p0*(2*t*t*t - 3*t*t + 1) + m0*(t*t*t - 2*t*t + t) + p1*(-2*t*t*t + 3*t*t) + m1*(t*t*t - t*t);
}

/* color interpolation */
static inline color gradient(int x, int size, color *knots, int n)
{
	float t;
	int i, step = size/(n - 1);

	i = x/step;
	x = x - step*i;
	t = lerp(x, 0, step, 0.0f, 1.0f);
	return rgb(roundf(hermite(t, getr(knots[i]), getr(knots[i + 1]), 0, 0)),
		   roundf(hermite(t, getg(knots[i]), getg(knots[i + 1]), 0, 0)),
		   roundf(hermite(t, getb(knots[i]), getb(knots[i + 1]), 0, 0)));
}

/* vector operations */

typedef struct { float x, y; } vec2;

#define vec2(x, y) (vec2){x, y}

#define v2add(a, b) (typeof((a))){(a).x + (b).x, (a).y + (b).y}
#define v2sub(a, b) (typeof((a))){(a).x - (b).x, (a).y - (b).y}
#define v2len(a) (sqrtf((a).x*(a).x + (a).y*(a).y))
#define v2dot(a, b) ((a).x*(b).x + (a).y*(b).y)

#define v2mask(a, b, mask) (typeof((a))){(mask).x ? (b).x : (a).x, (mask).y ? (b).y : (a).y}
#define v2clamp(a, min, max, clamped) (typeof((a))){fclamp((a).x, (min).x, (max).x, (&(clamped)->x)), fclamp((a).y, (min).y, (max).y, &((clamped)->y))}
#define v2rand(a, b) (vec2){frand2(a.x, b.x), frand2(a.y, b.y)}

typedef struct { float x, y, z; } vec3;

#define vec3(x, y, z) (vec3){x, y, z}

#endif
