#pragma once
#include "stdafx.h" 
struct cords {
	int x = 0;
	int y = 0;
};
struct Position {
	cords LeftTopCorner;
	cords RightBottomCorner;
	float PercentMatch = 0;
	wchar_t File[MAX_PATH];
};

extern "C" __declspec(dllexport) ULONG_PTR Init();
extern "C" __declspec(dllexport) void Shutdown(ULONG_PTR InitValue);

extern "C" __declspec(dllexport) Gdiplus::Bitmap *HWNDToPBitmap(HWND hwnd, int PixelFormat);
extern "C" __declspec(dllexport) int PBitmapToFile(Gdiplus::Bitmap *pbmp, wchar_t *File);

extern "C" __declspec(dllexport) Position *SeekImgOnImg(Gdiplus::Bitmap *pbmp1,
													Gdiplus::Bitmap *pbmp2,
													int Quality,
													int OccurCount,
													float MinPercentMatch);
extern "C" __declspec(dllexport) Gdiplus::Bitmap *FileToPBitmap(wchar_t *File);