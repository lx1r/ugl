#include <stdlib.h>

#include "ugl/fb.h"

int hwopen(struct fb *fb);
int hwclose(struct fb *fb);
int hwlock(struct fb *fb);
int hwunlock(struct fb *fb);
int hwread(struct fb *fb);

struct fb *fbopen(int w, int h)
{
	int rc;
	struct fb *fb;

	fb = malloc(sizeof(*fb));
	if (!fb)
		return 0;
	fb->w = w;
	fb->h = h;
	fb->pitch = 0;
	fb->pixbuf = NULL;
	rc = hwopen(fb);
	if (rc) {
		fbclose(fb);
		return 0;
	}
	fb->ev = 0;
	return fb;
}

int fbclose(struct fb *fb)
{
	if (!fb)
		return -1;
	if (fb->hw)
		hwclose(fb);
	free(fb);
	return 0;
}

int fblock(struct fb *fb)
{
	int rc;

	if (!fb->hw)
		return 0;
	rc = hwlock(fb);
	return rc;
}

int fbunlock(struct fb *fb)
{
	int rc;

	if (!fb->hw)
		return 0;
	rc = hwunlock(fb);
	hwread(fb);
	return rc;
}

#define EVQSZ 1024

struct evq {
	unsigned int rd, wr;
	int buf[EVQSZ];
};

#define evqnext(idx) (((idx) + 1) & (EVQSZ - 1))

int fbread(struct fb *fb)
{
	int ev = 0;
	struct evq *evq = fb->ev;

	if (!evq)
		return 0;
	if (fb->hw)
		hwread(fb);

	if (evq->rd != evq->wr) {
		ev = evq->buf[evq->rd];
		evq->rd = evqnext(evq->rd);
	}
	return ev;
}

int fbwrite(struct fb *fb, int ev)
{
	struct evq *evq = fb->ev;

	if (!evq) {
		evq = fb->ev = malloc(sizeof(*evq));
		if (!evq)
			return -1;
		evq->rd = evq->wr = 0;
		/* ignore inherited key event */
		if (!(ev & 0xff00) && (ev >> 16))
			return 0;
	}

	if (evqnext(evq->wr) != evq->rd) {
		evq->buf[evq->wr] = ev;
		evq->wr = evqnext(evq->wr);
		return 0;
	}
	return -1;
}

struct fb *vfballoc(int w, int h)
{
	struct fb *fb;

	fb = malloc(sizeof(*fb));
	if (!fb)
		return 0;
	fb->w = w;
	fb->h = h;
	fb->pitch = (w + 0xf) & ~0xf;
	fb->pixbuf = aligned_alloc(16, fb->pitch*fb->h*sizeof(*fb->pixbuf));
	if (!fb->pixbuf) {
		free(fb);
		return 0;
	}
	fb->hw = 0;
	fb->ev = 0;
	return fb;
}

void vfbfree(struct fb *fb)
{
	if (fb) {
		free(fb->pixbuf);
		free(fb);
	}
}
