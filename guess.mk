#!/bin/make -f

library = libugl.a
sources = fb.c

CFLAGS := -Wall -MMD -Ofast
LDFLAGS := -lm -lugl

machine = $(shell $(CC) -dumpmachine)
ifneq (,$(findstring linux, $(machine)))
  platform := unix
else ifneq (,$(findstring mingw, $(machine)))
  platform := windows
  cflags += -D"aligned_alloc(a, n)=_aligned_malloc(n, a)"
else ifneq (,$(findstring cygwin, $(machine)))
  platform := windows
else
  $(error Cannot guess target platform, use: make platform=<unix|windows>)
endif

video := none
input := none

ifeq ($(platform), unix)
  sources += xw.c
  ifneq (,$(wildcard /usr/include/X11/Xlib.h))
    video := xlib
  else
    $(warning Please install libx11-dev)
  endif
  ifneq (,$(wildcard /usr/include/X11/extensions/XShm.h))
    video := xshm
  else
    $(warning Please install libxext-dev)
  endif
  ifneq (,$(wildcard /usr/include/X11/extensions/XInput2.h))
    input := xi
  else
    $(warning Please install libxi-dev)
  endif
endif

ifeq ($(platform), windows)
  exe=.exe
  sources += dx.c
  CFLAGS += -mwindows
  ifneq (,$(wildcard /usr/include/w32api/wingdi.h))
    video := gdi
  else
    $(warning Please install w32api-runtime)
  endif
  ifneq (,$(wildcard /usr/include/w32api/ddraw.h))
    video := dx5
  endif
  ifneq (,$(wildcard /usr/include/w32api/d3d9.h))
    video := dx9
  endif
  ifneq (,$(wildcard /usr/include/w32api/winuser.h))
    input := ri
  endif
endif

ifeq ($(video), xlib)
  ldflags += -lX11
else ifeq ($(video), xshm)
  cflags += -DHAVE_XSHM=1
  ldflags += -lX11 -lXext
else ifeq ($(video), gdi)
  ldflags += -lgdi32
else ifeq ($(video), dx5)
  cflags += -DHAVE_DDRAW=1
  ldflags += -lddraw
else ifeq ($(video), dx9)
  cflags += -DHAVE_D3D9=1
  ldflags += -ld3d9
else
  $(error Cannot guess video device, use: make video=<xlib|xshm|gdi|dx5|dx9>)
endif

ifeq ($(input), xi)
  cflags += -DHAVE_XINPUT2=1
  ldflags += -lXi
else ifeq ($(input), ri)
  cflags += -DHAVE_RAWINPUT=1
else
  $(warning Cannot guess input device, use: make input=<xi|ri>)
endif

objects = $(sources:.c=.o)
