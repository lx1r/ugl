#ifndef UGL_FB_H
#define UGL_FB_H

struct fb {
	int w, h;
	int pitch;
	unsigned int *pixbuf;
	void *ev, *hw;
};

struct fb *fbopen(int w, int h);
int fbclose(struct fb *fb);
int fblock(struct fb *fb);
int fbunlock(struct fb *fb);
int fbread(struct fb *fb);
int fbwrite(struct fb *fb, int ev);
struct fb *vfballoc(int w, int h);
void vfbfree(struct fb *fb);

#endif
