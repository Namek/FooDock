#include "foo_dock.h"


HBITMAP LoadFoobarIcon(HMODULE hModule)
{
	HBITMAP bmp = LoadBitmap(hModule, MAKEINTRESOURCE(IDB_ICON));

	if (bmp == NULL)
		MessageBox(0, L"Foobar icon cannot be loaded", L"Error", MB_OK | MB_ICONEXCLAMATION);

	return bmp;
}

HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbmMask, hbmOld, hbmOld2;
	BITMAP bm;

	GetObject(hbmColour, sizeof(BITMAP), &bm);
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	hdcMem = CreateCompatibleDC(NULL);
	hdcMem2 = CreateCompatibleDC(NULL);

	hbmOld = (HBITMAP)SelectObject(hdcMem, hbmColour);
	hbmOld2 = (HBITMAP)SelectObject(hdcMem2, hbmMask);

	SetBkColor(hdcMem, crTransparent);

	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

	SelectObject(hdcMem, hbmOld);
	SelectObject(hdcMem2, hbmOld2);
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

	return hbmMask;
}
