#include "ugl/fb.h"
#include "ugl/input.h"

#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if HAVE_D3D9
#include <d3d9.h>
#elif HAVE_DDRAW
#include <ddraw.h>
#endif

#ifdef NDEBUG
#define RETURN_IF(rc) if (rc) return -1
#define RETURN_IF_FAILED(rc) if (rc < 0) return -1
#else
#include <stdio.h>
#define RETURN_IF(rc) if (rc) {\
	printf("%s:%d: %s(): internal error\n", __FILE__, __LINE__,__func__);\
	return -1;\
}
#define RETURN_IF_FAILED(rc) if (rc < 0) {\
	printf("%s:%d: %s(): error code: %hu\n", __FILE__, __LINE__,__func__, (unsigned short)rc);\
	return -1;\
}
#endif

struct hw {
	HWND win;
#if HAVE_D3D9
	LPDIRECT3D9 d3d;
	IDirect3DDevice9 *device;
	IDirect3DSurface9 *backbuf;
	D3DPRESENT_PARAMETERS presentation;
#elif HAVE_DDRAW
	LPDIRECTDRAW ddraw;
	LPDIRECTDRAWSURFACE surface;
	DDSURFACEDESC surfacedesc;
#else
	HDC hdc;
	HGDIOBJ gdiobj;
	HBITMAP bitmap;
#endif
};

static LRESULT CALLBACK winproc(HWND win, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_SETCURSOR:
		SetCursor(0);
		break;
	case WM_DESTROY:
		ExitProcess(0);
		break;
	}
	return DefWindowProc(win, message, wParam, lParam);
}

#if HAVE_RAWINPUT
static inline int riopen(struct fb *fb, int page, int usage)
{
	BOOL rc;
	RAWINPUTDEVICE rid;
	struct hw *hw = fb->hw;

	rid.usUsagePage = page;
	rid.usUsage = usage;
	rid.dwFlags = 0; /* RIDEV_NOLEGACY */
	rid.hwndTarget = hw->win;
	rc = RegisterRawInputDevices(&rid, 1, sizeof(rid));
	RETURN_IF(rc == FALSE);
	return 0;
}
#endif

int hwopen(struct fb *fb)
{
	struct hw *hw;

	hw = fb->hw = malloc(sizeof(*hw));
	RETURN_IF(!hw);
	memset(hw, 0, sizeof(*hw));

	if (!fb->w)
		fb->w = GetSystemMetrics(SM_CXSCREEN);
	if (!fb->h)
		fb->h = GetSystemMetrics(SM_CYSCREEN);

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = winproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = LoadIcon(wc.hInstance, IDI_APPLICATION);
	wc.hCursor = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName = 0;
	wc.lpszClassName = "ugl";
	RegisterClass(&wc);

	hw->win = CreateWindowEx(0, "ugl", "", WS_POPUP, 0, 0, fb->w, fb->h, 0, 0, wc.hInstance, 0);
	RETURN_IF(!hw->win);

	ShowWindow(hw->win, SW_NORMAL);
	UpdateWindow(hw->win);
	PostMessage(hw->win, WM_SETCURSOR, 0, 0);

#if HAVE_D3D9
	HRESULT rc;

	hw->d3d = Direct3DCreate9(D3D_SDK_VERSION);
	RETURN_IF(!hw->d3d);

	hw->presentation.BackBufferWidth = fb->w;
	hw->presentation.BackBufferHeight = fb->h;
	hw->presentation.BackBufferFormat = D3DFMT_X8R8G8B8;
	hw->presentation.BackBufferCount = 1;
	hw->presentation.SwapEffect = D3DSWAPEFFECT_DISCARD;
	hw->presentation.hDeviceWindow = hw->win;
	hw->presentation.Windowed = FALSE;
	hw->presentation.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
#if VSYNC
	hw->presentation.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
#else
	hw->presentation.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#endif
	DWORD flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	rc = IDirect3D9_CreateDevice(hw->d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hw->win, flags, &hw->presentation, &hw->device);
	RETURN_IF_FAILED(rc);

#elif HAVE_DDRAW
	HRESULT rc;

	rc = DirectDrawCreate(0, &hw->ddraw, 0);
	RETURN_IF_FAILED(rc);

	rc = IDirectDraw_SetCooperativeLevel(hw->ddraw, hw->win, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE);
	RETURN_IF_FAILED(rc);

	rc = IDirectDraw_SetDisplayMode(hw->ddraw, fb->w, fb->h, 32);
	RETURN_IF_FAILED(rc);

	hw->surfacedesc.dwSize = sizeof(hw->surfacedesc);
	hw->surfacedesc.dwFlags = DDSD_CAPS;
	hw->surfacedesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	rc = IDirectDraw_CreateSurface(hw->ddraw, &hw->surfacedesc, &hw->surface, 0);
	RETURN_IF_FAILED(rc);

	DDPIXELFORMAT fmt = {0};
	fmt.dwSize = sizeof(fmt);
	rc = IDirectDrawSurface_GetPixelFormat(hw->surface, &fmt);
	RETURN_IF_FAILED(rc);
	RETURN_IF(!(fmt.dwFlags & DDPF_RGB));
	RETURN_IF(fmt.dwRGBBitCount != 32);
	RETURN_IF(fmt.dwRBitMask != 0xff0000 || fmt.dwGBitMask != 0xff00 || fmt.dwBBitMask != 0xff);
#else
	hw->hdc = CreateCompatibleDC(NULL);
	RETURN_IF(!hw->hdc);

	BITMAPINFO bitmapinfo = {0};
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biWidth = fb->w;
	bitmapinfo.bmiHeader.biHeight = -fb->h;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 32;
	bitmapinfo.bmiHeader.biCompression = BI_RGB;
	bitmapinfo.bmiHeader.biSizeImage = 0;
	bitmapinfo.bmiHeader.biClrUsed = 256;
	bitmapinfo.bmiHeader.biClrImportant = 256;

	hw->bitmap = CreateDIBSection(hw->hdc, &bitmapinfo, DIB_RGB_COLORS, (void **)&fb->pixbuf, NULL, 0);
	RETURN_IF(!hw->bitmap || !fb->pixbuf);

	hw->gdiobj = SelectObject(hw->hdc, hw->bitmap);
	RETURN_IF(!hw->gdiobj || hw->gdiobj == HGDI_ERROR);

	fb->pitch = fb->w;
#endif

#if HAVE_RAWINPUT
	riopen(fb, 0x01, 0x06); /* RAWKEYBOARD */
	riopen(fb, 0x01, 0x02); /* RAWMOUSE */
#endif
	return 0;
}

int hwclose(struct fb *fb)
{
	struct hw *hw = fb->hw;
#if HAVE_D3D9
	if (hw->backbuf)
		IDirect3DSurface9_Release(hw->backbuf);
	if (hw->device)
		IDirect3DDevice9_Release(hw->device);
	if (hw->d3d)
		IDirect3D9_Release(hw->d3d);
#elif HAVE_DDRAW
	if (hw->surface)
		IDirectDrawSurface_Release(hw->surface);
	if (hw->ddraw)
		IDirectDraw_Release(hw->ddraw);
#else
	if (hw->gdiobj && hw->gdiobj != HGDI_ERROR)
		SelectObject(hw->hdc, hw->gdiobj);
	if (hw->bitmap)
		DeleteObject(hw->bitmap);
	if (hw->hdc)
		DeleteDC(hw->hdc);
#endif
	free(hw);
	fb->hw = 0;
	return 0;
}

int hwlock(struct fb *fb)
{
#if HAVE_D3D9
	HRESULT rc;
	struct hw *hw = fb->hw;

	rc = IDirect3DDevice9_TestCooperativeLevel(hw->device);
	if (FAILED(rc)) {
		if (rc == D3DERR_DEVICENOTRESET) {
			if (hw->backbuf) {
				IDirect3DSurface9_Release(hw->backbuf);
				hw->backbuf = NULL;
			}
			rc = IDirect3DDevice9_Reset(hw->device, &hw->presentation);
			RETURN_IF_FAILED(rc);
		}
		else
			return -1;
	}
	if (!hw->backbuf) {
		IDirect3DDevice9_GetBackBuffer(hw->device, 0, 0, D3DBACKBUFFER_TYPE_MONO, &hw->backbuf);
		RETURN_IF(!hw->backbuf);
	}

	D3DLOCKED_RECT lock;
	rc = IDirect3DSurface9_LockRect(hw->backbuf, &lock, NULL, D3DLOCK_NOSYSLOCK);
	RETURN_IF_FAILED(rc);
	fb->pixbuf = lock.pBits;
	fb->pitch = lock.Pitch/sizeof(*fb->pixbuf);
#elif HAVE_DDRAW
	HRESULT rc;
	struct hw *hw = fb->hw;

	if (IDirectDrawSurface_IsLost(hw->surface)) {
		rc = IDirectDrawSurface_Restore(hw->surface);
		if (FAILED(rc))
			return -1;
	}
#if VSYNC
	IDirectDraw_WaitForVerticalBlank(hw->ddraw, DDWAITVB_BLOCKBEGIN, 0);
#endif
	rc = IDirectDrawSurface_Lock(hw->surface, 0, &hw->surfacedesc, DDLOCK_NOSYSLOCK, 0);
	RETURN_IF_FAILED(rc);

	fb->pixbuf = hw->surfacedesc.lpSurface;
	if (hw->surfacedesc.dwFlags & DDSD_PITCH)
		fb->pitch = hw->surfacedesc.lPitch/sizeof(*fb->pixbuf);
	else
		fb->pitch = fb->w;
#endif
	return 0;
}

int hwunlock(struct fb *fb)
{
	struct hw *hw = fb->hw;
#if HAVE_D3D9
	HRESULT rc;

	IDirect3DSurface9_UnlockRect(hw->backbuf);

	rc = IDirect3DDevice9_Present(hw->device, NULL, NULL, hw->win, NULL);
	RETURN_IF_FAILED(rc);
#elif HAVE_DDRAW
	IDirectDrawSurface_Unlock(hw->surface, 0);
#else
	PAINTSTRUCT ps;
	InvalidateRect(hw->win, NULL, FALSE);
	HDC hdc = BeginPaint(hw->win, &ps);
	BitBlt(hdc, 0, 0, fb->w, fb->h, hw->hdc, 0, 0, SRCCOPY);
	EndPaint(hw->win, &ps);
#endif
	return 0;
}

int hwread(struct fb *fb)
{
	MSG msg;
	struct hw *hw = fb->hw;

	while (PeekMessage(&msg, hw->win, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		switch (msg.message) {
#if HAVE_RAWINPUT
		case WM_INPUT:
			RAWINPUT *raw;
			unsigned int size;

			GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
			raw = alloca(size);
			GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

			if (raw->header.dwType == RIM_TYPEKEYBOARD) {
				if (raw->data.keyboard.Flags & RI_KEY_BREAK)
					fbwrite(fb, ev(raw->data.keyboard.MakeCode, KEY_RELEASED));
				else
					fbwrite(fb, ev(raw->data.keyboard.MakeCode, KEY_PRESSED));
			}
			else if (raw->header.dwType == RIM_TYPEMOUSE) {
				if (raw->data.mouse.lLastX != 0)
					fbwrite(fb, ev(MICE_DX, raw->data.mouse.lLastX));
				if (raw->data.mouse.lLastY != 0)
					fbwrite(fb, ev(MICE_DY, raw->data.mouse.lLastY));
				if (raw->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
					fbwrite(fb, ev(MICE_LEFT, KEY_PRESSED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_UP)
					fbwrite(fb, ev(MICE_LEFT, KEY_RELEASED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
					fbwrite(fb, ev(MICE_RIGHT, KEY_PRESSED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_UP)
					fbwrite(fb, ev(MICE_RIGHT, KEY_RELEASED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
					fbwrite(fb, ev(MICE_MIDDLE, KEY_PRESSED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_MIDDLE_BUTTON_UP)
					fbwrite(fb, ev(MICE_MIDDLE, KEY_RELEASED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
					fbwrite(fb, ev(MICE_FOUR, KEY_PRESSED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_BUTTON_4_UP)
					fbwrite(fb, ev(MICE_FOUR, KEY_RELEASED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
					fbwrite(fb, ev(MICE_FIVE, KEY_PRESSED));
				if (raw->data.mouse.ulButtons & RI_MOUSE_BUTTON_5_UP)
					fbwrite(fb, ev(MICE_FIVE, KEY_RELEASED));
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
					fbwrite(fb, ev(MICE_WHEEL, (short)raw->data.mouse.usButtonData/WHEEL_DELTA));
			}
			break;
#else
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			if (HIWORD(msg.lParam) & KF_UP)
				fbwrite(fb, ev(LOBYTE(HIWORD(msg.lParam)), KEY_RELEASED));
			else
				fbwrite(fb, ev(LOBYTE(HIWORD(msg.lParam)), KEY_PRESSED));
			break;
#endif
		}
		DispatchMessage(&msg);
	}
	return 0;
}
