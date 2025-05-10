#include "ugl/fb.h"
#include "ugl/input.h"

#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#ifdef HAVE_XSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#ifdef HAVE_XINPUT2
#include <X11/extensions/XInput2.h>
#endif

struct hw {
	Display *display;
	Window window;
	XImage *image;
	GC gc;
#ifdef HAVE_XSHM
	XShmSegmentInfo shminfo;
#endif
	int depth;
	int min_keycode;
	int max_keycode;
};

static int xwchkbpp(struct fb* fb, int depth)
{
	int i;
	int count;
	XPixmapFormatValues *formats;
	struct hw *hw = fb->hw;

	formats = XListPixmapFormats(hw->display, &count);
	for (i = 0; i < count; i++) {
		if (depth == formats[i].depth) {
			if (formats[i].bits_per_pixel == 32) {
				XFree(formats);
				return 0;
			}
		}
	}
	XFree(formats);
	printf("Only 32bpp TrueColor mode is supported\n");
	return -1;
}

static int xwfullscreen(struct fb *fb)
{
	struct hw *hw = fb->hw;
	Atom atoms[2] = {XInternAtom(hw->display, "_NET_WM_STATE_FULLSCREEN", False), None};

	XChangeProperty(hw->display, hw->window,
			XInternAtom(hw->display, "_NET_WM_STATE", False),
			XA_ATOM, 32, PropModeReplace, (void *)atoms, 1);
	return 0;
}

static int xwhidecursor(struct fb *fb)
{
	XColor color = {0};
	const char data[] = {0};
	struct hw *hw = fb->hw;

	Pixmap pixmap = XCreateBitmapFromData(hw->display, hw->window, data, 1, 1);
	Cursor cursor = XCreatePixmapCursor(hw->display, pixmap, pixmap, &color, &color, 0, 0);

	XDefineCursor(hw->display, hw->window, cursor);
	XFreeCursor(hw->display, cursor);
	XFreePixmap(hw->display, pixmap);
	return 0;
}

static int xwopenbackbuf(struct fb *fb)
{
	struct hw *hw = fb->hw;

#ifdef HAVE_XSHM
	hw->shminfo.shmid = shmget(IPC_PRIVATE, fb->pitch*fb->h*sizeof(*fb->pixbuf), IPC_CREAT | 0777);
	if (hw->shminfo.shmid < 0)
		return -1;

	hw->shminfo.shmaddr = shmat(hw->shminfo.shmid, 0, 0);
	if (hw->shminfo.shmaddr == (char *)-1)
		return -1;

	hw->shminfo.readOnly = False;
	XShmAttach(hw->display, &hw->shminfo);

	fb->pixbuf = (void *)hw->shminfo.shmaddr;
	hw->image = XShmCreateImage(hw->display, CopyFromParent, hw->depth, ZPixmap,
				    hw->shminfo.shmaddr, &hw->shminfo, fb->w, fb->h);
#else
	fb->pixbuf = aligned_alloc(16, fb->pitch*fb->h*sizeof(*fb->pixbuf));
	hw->image = XCreateImage(hw->display, CopyFromParent, hw->depth, ZPixmap,
				 0, (char *)fb->pixbuf, fb->w, fb->h, 32, fb->pitch*sizeof(*fb->pixbuf));
#endif
	return 0;
}

static int xwclosebackbuf(struct fb *fb)
{
	struct hw *hw = fb->hw;

#ifdef HAVE_XSHM
	XShmDetach(hw->display, &hw->shminfo);
	shmdt(hw->shminfo.shmaddr);
#else
	XDestroyImage(hw->image);
#endif
	return 0;
}

#ifdef HAVE_XINPUT2
static int xiopen(struct fb *fb)
{
	int major = 2;
	int minor = 2;
	int xi_opcode, event, error;
	XIEventMask evmask[1];
	unsigned char mask[(XI_LASTEVENT + 7)/8] = {0};
	struct hw *hw = fb->hw;

	if (!XQueryExtension(hw->display, "XInputExtension", &xi_opcode, &event, &error)) {
		printf("XInputExtension is not available\n");
		return -1;
	}
	if (XIQueryVersion(hw->display, &major, &minor) == BadRequest) {
		printf("XI2 is not supported, server supports version %d.%d only\n", major, minor);
		return -1;
	}
	XISetMask(mask, XI_RawMotion);
	evmask[0].deviceid = XIAllMasterDevices;
	evmask[0].mask_len = sizeof(mask);
	evmask[0].mask = mask;
	XISelectEvents(hw->display, DefaultRootWindow(hw->display), evmask, 1);
	XFlush(hw->display);
	return 0;
}
#endif

int hwopen(struct fb *fb)
{
	int screen;
	int fullscreen = !(fb->w || fb->h);
	XSetWindowAttributes attr;
	struct hw *hw;

	hw = fb->hw = malloc(sizeof(*fb->hw));
	if (!fb->hw)
		return -1;
	memset(hw, 0, sizeof(*hw));

	hw->display = XOpenDisplay(0);
	if (!hw->display)
		return -1;
	screen = XDefaultScreen(hw->display);

	if (!fb->w)
		fb->w = XDisplayWidth(hw->display, screen);
	if (!fb->h)
		fb->h = XDisplayHeight(hw->display, screen);
	fb->pitch = fb->w;

	hw->depth = XDefaultDepth(hw->display, screen);
	if (xwchkbpp(fb, hw->depth) == -1)
		return -1;

	attr.border_pixel = 0;
	attr.background_pixel = 0;
	attr.event_mask = ExposureMask | StructureNotifyMask;

	hw->window = XCreateWindow(hw->display,
				   XRootWindow(hw->display, screen), 0, 0,
				   fb->w, fb->h, 0, 0, InputOutput, CopyFromParent,
				   CWBackPixel | CWBorderPixel | CWEventMask, &attr);
	XMapRaised(hw->display, hw->window);

	if (fullscreen)
		xwfullscreen(fb);
	xwhidecursor(fb);

	hw->gc = XDefaultGC(hw->display, screen);
	xwopenbackbuf(fb);

	XSelectInput(hw->display, hw->window,
		     KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
	XDisplayKeycodes(hw->display, &hw->min_keycode, &hw->max_keycode);
#ifdef HAVE_XINPUT2
	xiopen(fb);
#endif
	return 0;
}

int hwclose(struct fb *fb)
{
	struct hw *hw = fb->hw;

	if (hw->image)
		xwclosebackbuf(fb);
	if (hw->display)
		XCloseDisplay(hw->display);
	free(fb->hw);
	fb->hw = 0;
	return 0;
}

int hwlock(struct fb *fb)
{
	return 0;
}

int hwunlock(struct fb *fb)
{
	struct hw *hw = fb->hw;

#ifdef HAVE_XSHM
	XShmPutImage(hw->display, hw->window, hw->gc, hw->image, 0, 0, 0, 0, fb->pitch, fb->h, False);
#else
	XPutImage(hw->display, hw->window, hw->gc, hw->image, 0, 0, 0, 0, fb->pitch, fb->h);
#endif
	return 0;
}

int hwread(struct fb *fb)
{
	XEvent ev;
#ifdef HAVE_XINPUT2
	XGenericEventCookie *cookie = &ev.xcookie;
#endif
	struct hw *hw = fb->hw;

	while (XPending(hw->display)) {
		XNextEvent(hw->display, &ev);
		switch(ev.type) {
		case KeyPress:
			fbwrite(fb, ev(ev.xkey.keycode - hw->min_keycode, KEY_PRESSED));
			break;
		case KeyRelease:
			fbwrite(fb, ev(ev.xkey.keycode - hw->min_keycode, KEY_RELEASED));
			break;
		case ButtonPress:
			if (ev.xbutton.button == Button1)
				fbwrite(fb, ev(MICE_LEFT, KEY_PRESSED));
			else if (ev.xbutton.button == Button2)
				fbwrite(fb, ev(MICE_MIDDLE, KEY_PRESSED));
			else if (ev.xbutton.button == Button3)
				fbwrite(fb, ev(MICE_RIGHT, KEY_PRESSED));
			break;
		case ButtonRelease:
			if (ev.xbutton.button == Button1)
				fbwrite(fb, ev(MICE_LEFT, KEY_RELEASED));
			else if (ev.xbutton.button == Button2)
				fbwrite(fb, ev(MICE_MIDDLE, KEY_RELEASED));
			else if (ev.xbutton.button == Button3)
				fbwrite(fb, ev(MICE_RIGHT, KEY_RELEASED));
			break;
#ifdef HAVE_XINPUT2
		case GenericEvent:
			if (!XGetEventData(hw->display, cookie))
				break;
			if (cookie->evtype == XI_RawMotion) {
				XIRawEvent *re = cookie->data;
				int dx = (int)re->raw_values[0];
				int dy = (int)re->raw_values[1];

				if (dx)
					fbwrite(fb, ev(MICE_DX, dx));
				if (dy)
					fbwrite(fb, ev(MICE_DY, dy));
			}
			break;
#endif
		}
	}
	return 0;
}
