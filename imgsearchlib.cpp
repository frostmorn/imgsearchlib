#include "imgsearchlib.h"
//#define _LogS
#ifdef _LogS
#include <iostream>
#endif
#include <ctime>
#include <gl\GL.h>
//#include "stdafx.h"
DEFINE_GUID(EncoderColorDepth, 0x66087055, 0xad66, 0x4c7c, 0x9a, 0x18, 0x38, 0xa2, 0x31, 0x0b, 0x83, 0x37);

/*
	Костыль из GdiPlusImaging.h
	Ни в каком из порядков инклудов не собирается, поэтому я задефайнил так
*/


float GetPicMatch(Gdiplus::BitmapData *BitmapData1, Gdiplus::BitmapData *BitmapData2, int xpos, int ypos, float MinMatch, int width, int height) {

	int NonMatch = 0;
	
	int MaxMatch = width *height;
	int MinNonMatch = ((((100 - MinMatch) / 100)*MaxMatch));
	UINT* pixels1 = (UINT*)BitmapData1->Scan0;
	UINT* pixels2 = (UINT*)BitmapData2->Scan0;
	
	for(int x = 0; x < width; x++)

		for (int y = 0; y < height; y++) {
			if ((pixels1[y*BitmapData1->Stride /4 +x] != pixels2[(y+ypos)*BitmapData2->Stride /4 +(x+xpos)]))
			 NonMatch++;
			if (NonMatch > MinNonMatch) {
					return 0;
			}
		}

	return (100.0 - (NonMatch*100.0f / MaxMatch*1.0f));	//In percents
}



int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}
	free(pImageCodecInfo);
	return -1;  // Failure
}

extern "C" __declspec(dllexport) Gdiplus::Bitmap *HWNDToPBitmap(HWND hwnd, int PixelFormat) {

		RECT rc;
		HDC hdcScreen;
		HDC hdc;
		HBITMAP hbmp;
		try {
			
#ifdef _LogS
			std::cout << "I'm in function" << std::endl;
#endif
			if ((hwnd == 0)||(!IsWindow(hwnd)) || (IsIconic(hwnd)))
			{
#ifdef _LogS
				std::cout << "Is not a Window, or Handle invalid" << std::endl;
#endif
				return 0;
			}
			int PixForm;
			switch (PixelFormat) {
			case 1: {PixForm = PixelFormat1bppIndexed;
				break; }	// Чёрный/Белый
			case 2: {PixForm = PixelFormat4bppIndexed;
				break; }		// 4 бит серые тона
			case 3: {PixForm = PixelFormat8bppIndexed;
				break; }	// 8 бит серые тона
			case 4: {PixForm = PixelFormat24bppRGB;
				break; }	// 2^24 полноцветная
			}
					Gdiplus::Bitmap *pbmp;
		
#ifdef _LogS
			std::cout << "GetWindowRect" << std::endl;
#endif
			if (!GetWindowRect(hwnd, &rc))
				return 0;
			BITMAPINFO* bmpInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255);
			ZeroMemory(bmpInfo, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255);
			
			bmpInfo->bmiHeader.biWidth = rc.right - rc.left;
			bmpInfo->bmiHeader.biHeight = rc.bottom - rc.top;
			bmpInfo->bmiHeader.biPlanes = 1;
			bmpInfo->bmiHeader.biBitCount = Gdiplus::GetPixelFormatSize(PixForm) ;
			if (bmpInfo->bmiHeader.biBitCount == 1)
			{
				bmpInfo->bmiColors[0].rgbBlue = 255;
				bmpInfo->bmiColors[0].rgbGreen = 255;
				bmpInfo->bmiColors[0].rgbRed = 255;
				bmpInfo->bmiColors[1].rgbBlue = 0;
				bmpInfo->bmiColors[1].rgbGreen = 0;
				bmpInfo->bmiColors[1].rgbRed = 0;
			}
			else if (bmpInfo->bmiHeader.biBitCount == 4) {
				bmpInfo->bmiHeader.biClrUsed = 16;
				bmpInfo->bmiHeader.biClrImportant = 16;
				for (int i = 0; i < 16; i++) {
					bmpInfo->bmiColors[i].rgbBlue = 8 + 16* i;
					bmpInfo->bmiColors[i].rgbGreen = 8 + 16 * i;
					bmpInfo->bmiColors[i].rgbRed = 8 + 16 * i;
				}
			}
			else if (bmpInfo->bmiHeader.biBitCount == 8) {
				for (int i = 0; i < 255; i++)
				{
					bmpInfo->bmiColors[i].rgbBlue = i;
					bmpInfo->bmiColors[i].rgbGreen = i;
					bmpInfo->bmiColors[i].rgbRed = i;
				}
			}
			bmpInfo->bmiHeader.biCompression = BI_RGB;
			bmpInfo->bmiHeader.biSizeImage = 0;
			bmpInfo->bmiHeader.biXPelsPerMeter = 0;
			bmpInfo->bmiHeader.biYPelsPerMeter = 0;
			bmpInfo->bmiHeader.biClrUsed = 0;
			bmpInfo->bmiHeader.biClrImportant = 0;
			bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			COLORREF* pixel = new COLORREF[bmpInfo->bmiHeader.biWidth * bmpInfo->bmiHeader.biHeight];
			hdcScreen = GetDC(GetDesktopWindow());
			hdc = CreateCompatibleDC(hdcScreen);

			
			hbmp = CreateCompatibleBitmap(hdcScreen,
				rc.right - rc.left, rc.bottom - rc.top);

			hbmp = CreateDIBSection(hdc, bmpInfo, DIB_RGB_COLORS, (void **)pixel, 0, 0);
		
#ifdef _LogS
			std::cout << "Select Object" << std::endl;
#endif
			
			
			SelectObject(hdc, hbmp);
			
#ifdef _LogS
				std::cout << "BitBlt" << std::endl;
#endif
			BitBlt(hdc, 0, 0, rc.right-rc.left,  rc.bottom-rc.top, hdcScreen,rc.left,rc.top, SRCCOPY | CAPTUREBLT);
#ifdef _LogS
			std::cout << "GetObject" << std::endl;
#endif
			GetObject(hbmp, (rc.right - rc.left)*(rc.bottom - rc.top), pixel);
			pbmp = new Gdiplus::Bitmap(hbmp, 0);
		

			delete[] pixel;

	#ifdef _LogS
			std::cout << "Gdiplus::Bitmap.Clone()" << std::endl;
#endif
		
			Gdiplus::Bitmap *newpbmp = pbmp;
			free(bmpInfo);

#ifdef _LogS
		std::cout << "Last status pbmp = " << pbmp->GetLastStatus() << std::endl;
		std::cout << "Last status newpbmp = " << newpbmp->GetLastStatus() << std::endl;
		std::cout << &pbmp << std::endl;
		std::cout << &newpbmp << std::endl;
#endif

		//release
		DeleteDC(hdc);
		DeleteObject(hbmp);
		ReleaseDC(hwnd, hdcScreen);



		return newpbmp;
	}
	catch (...) {
#ifdef _LogS

		std::cout << "Exception catch" << std::endl;
		std::cout << "Last Error = " << GetLastError() << std::endl;
#endif
		
		DeleteDC(hdc);
		DeleteObject(hbmp);
		ReleaseDC(hwnd, hdcScreen);

		return 0;
	}

	
}

extern "C" __declspec(dllexport) int PBitmapToFile(Gdiplus::Bitmap *pbmp, wchar_t *File) {
	if (pbmp == 0)
		return 0;
	try {
#ifdef _LogS
		std::wcout << "File = " << File << std::endl;
#endif
		CLSID myClsId;
		int retVal = GetEncoderClsid(L"image/bmp", &myClsId);
#ifdef _LogS
		std::cout << "CLSID d1= " << myClsId.Data1 << std::endl;
		std::cout << "CLSID d2= " << myClsId.Data2 << std::endl;
		std::cout << "CLSID d3= " << myClsId.Data3 << std::endl;
		std::cout << "CLSID d4= " << myClsId.Data4 << std::endl;
#endif
		Gdiplus::EncoderParameters* pEncoderParameters = (Gdiplus::EncoderParameters*)
			malloc(sizeof(Gdiplus::EncoderParameters));
		pEncoderParameters->Parameter->Guid = EncoderColorDepth; //-V619
		pEncoderParameters->Parameter->NumberOfValues = 1; //-V619
		pEncoderParameters->Parameter->Type = PropertyTagTypeByte; //-V619
		Gdiplus::PixelFormat PxlFormat = pbmp->GetPixelFormat();
		
		pEncoderParameters->Parameter->Value = &PxlFormat; //-V619 //-V506
		pEncoderParameters->Count = 1;
		pbmp->Save(File, &myClsId, pEncoderParameters);		
		
		free(pEncoderParameters);
		return 1;
	}
	catch (...) {
#ifdef _LogS
		std::cout << "Exception catch" << std::endl;
		std::cout << "Last Error = " << GetLastError() << std::endl;
#endif
		return 0;
	}
}

extern "C" __declspec(dllexport) ULONG_PTR Init() {
	try {
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		return gdiplusToken;
	}
		catch (...)
		{
#ifdef _LogS
			std::cout << "Exception catch" << std::endl;
			std::cout << "Last Error = " << GetLastError() << std::endl;
#endif
			return 0;
		}

}
extern "C" __declspec(dllexport) void Shutdown(ULONG_PTR InitValue) {
	Gdiplus::GdiplusShutdown(InitValue);
}
extern "C" __declspec(dllexport) Gdiplus::Bitmap *FileToPBitmap(wchar_t *File) {
	Gdiplus::Bitmap *pbmp = new Gdiplus::Bitmap(File);
	return pbmp;
}
static Gdiplus::Bitmap *ScaleBitmap(Gdiplus::Bitmap *pbmp, int nWidth, int nHeight)
{
	Gdiplus::Bitmap *npbmp = new Gdiplus::Bitmap(nWidth, nHeight);
	Gdiplus::Graphics *PGraphics = new Gdiplus::Graphics(npbmp);

	
	PGraphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	Gdiplus::Status State = PGraphics->DrawImage(pbmp,
		Gdiplus::Rect(0, 0, nWidth, nHeight));
	while (State == Gdiplus::ObjectBusy)
		Sleep(100);
	return npbmp;
}

extern "C" __declspec(dllexport) Position *SeekImgOnImg(Gdiplus::Bitmap *pbmp1, 
														Gdiplus::Bitmap *pbmp2, 
														int Quality, 
														int OccurCount, 
														float MinPercentMatch) {

	if (!pbmp1 || !pbmp2)
		return 0;
	if ((pbmp1->GetWidth() > pbmp2->GetWidth())||(pbmp1->GetHeight()>pbmp2->GetHeight()))	
		//высота или ширина искомого изображения больше 
		//основного
		return 0;

	float pbmp1NewWidth, pbmp2NewWidth, pbmp1NewHeight, pbmp2NewHeight;
	Position *PositionList = (Position *)malloc(sizeof(Position)* OccurCount);
	ZeroMemory(PositionList, sizeof(Position)*OccurCount);
	pbmp1NewWidth	= (float)pbmp1->GetWidth() / Quality;
	pbmp2NewHeight	= (float)pbmp2->GetHeight() / Quality;
	
	pbmp2NewWidth	= (float)pbmp2->GetWidth() / Quality;
	pbmp1NewHeight	= (float)pbmp1->GetHeight() / Quality;
	Gdiplus::Bitmap *npbmp1;
	Gdiplus::Bitmap *npbmp2;
	if (Quality != 1) {
		npbmp1 = ScaleBitmap(pbmp1, (int)pbmp1NewWidth, (int)pbmp1NewHeight);
		npbmp2 = ScaleBitmap(pbmp2, (int)pbmp2NewWidth , (int)pbmp2NewHeight);
	}
	else
	{
		npbmp1 = pbmp1;
		npbmp2 = pbmp2;
	}

	Gdiplus::Rect *rc1 = new Gdiplus::Rect(0, 0, npbmp1->GetWidth(), npbmp1->GetHeight());
	Gdiplus::Rect *rc2 = new Gdiplus::Rect(0, 0,  npbmp2->GetWidth(), npbmp2->GetHeight());
	Gdiplus::BitmapData* bitmapData1 = new Gdiplus::BitmapData;
	Gdiplus::BitmapData* bitmapData2 = new Gdiplus::BitmapData;
	
	npbmp1->LockBits(rc1, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData1);

	npbmp2->LockBits(rc2, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData2);
	delete rc1; 
	delete rc2;
	for (int y = 0; y <= (int)npbmp2->GetHeight() - npbmp1->GetHeight(); y++)
		for (int x = 0; x <= (int)npbmp2->GetWidth() - npbmp1->GetWidth(); x++)
		{
			Position *MinMatchPos = &PositionList[0];
			int k = 0;
			int Begin = GetTickCount();
			for (int i = 0; i < OccurCount; i++) {
				if (((x  > PositionList[i].LeftTopCorner.x) && 
					(x < PositionList[i].RightBottomCorner.x)) && 
					(y  > PositionList[i].LeftTopCorner.y) && 
					(y < PositionList[i].RightBottomCorner.y))
				{
					MinMatchPos = &PositionList[i];
					break;
				}
				else
				if (MinMatchPos->PercentMatch > PositionList[i].PercentMatch)
				{
					MinMatchPos = &PositionList[i];
				}
				
				
			}
			
			float CurrentMatch = GetPicMatch(bitmapData1, bitmapData2, x, y, max(MinMatchPos->PercentMatch,MinPercentMatch), npbmp1->GetWidth(), npbmp1->GetHeight());
			
			if (CurrentMatch>= max(MinMatchPos->PercentMatch, MinPercentMatch))
			{
				MinMatchPos->PercentMatch = CurrentMatch;

				MinMatchPos->LeftTopCorner.x = x;
				MinMatchPos->LeftTopCorner.y = y;
				MinMatchPos->RightBottomCorner.x = (x + npbmp1->GetWidth());
				MinMatchPos->RightBottomCorner.y = (y + npbmp1->GetHeight());
			
			}
			
			int End = GetTickCount();
	}
	for (int m = 0; m < OccurCount; m++) {

		PositionList[m].LeftTopCorner.x = PositionList[m].LeftTopCorner.x*Quality;
		PositionList[m].LeftTopCorner.y = PositionList[m].LeftTopCorner.y*Quality;
		PositionList[m].RightBottomCorner.x = PositionList[m].RightBottomCorner.x*Quality;
		PositionList[m].RightBottomCorner.y = PositionList[m].RightBottomCorner.y*Quality;

	}
#ifdef _LogS

	for (int m = 0; m < OccurCount; m++) {
		std::cout << "Founded result : x1 =" << PositionList[m].LeftTopCorner.x 
			<< " y1 = " << PositionList[m].LeftTopCorner.y<<
			" x2 = "<<PositionList[m].RightBottomCorner.x<<" y2 = "<<PositionList[m].RightBottomCorner.y
			<< " Match = " << PositionList[m].PercentMatch << "%" << std::endl;
	}
#endif
	npbmp1->UnlockBits(bitmapData1);
	npbmp2->UnlockBits(bitmapData2);
	delete bitmapData1;
	delete bitmapData2;
	return PositionList;
}

extern "C" __declspec(dllexport) Position *SearchFromFolderOnHwnd(wchar_t *Folder, HWND hwnd, int OccurCount, float MinPercentMatch, int PixelFormat, wchar_t *ErrorFolder) {
	
	 wchar_t * nFolder = (wchar_t *)malloc(sizeof(wchar_t)*MAX_PATH);
	 wcscpy(nFolder, Folder);
	 wcscat(nFolder, L"\\*.bmp");
	 Position * Result;
	 WIN32_FIND_DATAW FileData;
	 HANDLE SearchHandle = FindFirstFileW(nFolder, &FileData);
	 try {
		 Gdiplus::Bitmap *pbmp1 = HWNDToPBitmap(hwnd, PixelFormat);
		 if (!pbmp1)
			 throw 2;
		 if (SearchHandle != INVALID_HANDLE_VALUE)
			 do
			 {
				 wchar_t * nPath = (wchar_t *)malloc(sizeof(wchar_t)*MAX_PATH);
				 wcscpy(nPath, Folder);
				 wcscat(nPath,L"\\");
				 wcscat(nPath, FileData.cFileName);
#ifdef _LogS
				 std::wcout <<  L"Searching " << FileData.cFileName<< std::endl;
#endif
				 Gdiplus::Bitmap *pbmp2 = new Gdiplus::Bitmap(nPath,0);
				
				 Position * CurrentPositionList = SeekImgOnImg(pbmp2, pbmp1, 1, OccurCount, MinPercentMatch);
				 delete pbmp2;
				 for (int i = 0; i < OccurCount; i++)
					 if ((int)CurrentPositionList[i].PercentMatch>0) {
						 Result = CurrentPositionList;
						 wcscpy(Result[0].File, nPath);
						 free(nPath);
						 delete pbmp1;
;						 throw 0;
					 }
				 free(nPath);
				// delete pbmp1;
				 free(CurrentPositionList);
			
			 } while (FindNextFileW(SearchHandle, &FileData));
		 else {
			 delete pbmp1;
			 throw 1;
		 }
	
		
		 if (ErrorFolder && wcscmp(ErrorFolder, L"")) { //-V526
			 wchar_t * ErrorPath = (wchar_t *)malloc(sizeof(wchar_t)*MAX_PATH);
			 wcscpy(ErrorPath, ErrorFolder);
			 wcscat(ErrorPath, L"\\");
			 wcscat(ErrorPath, L"error_");
			 time_t t = time(0);
			 struct tm * now = localtime(&t);
			 wchar_t *buffer = (wchar_t *)malloc(sizeof(wchar_t) * 5);
			 _itow(now->tm_year + 1900, buffer, 10);

			 wcscat(ErrorPath, buffer);
			 wcscat(ErrorPath, L".");
			 _itow(now->tm_mday, buffer, 10);
			 wcscat(ErrorPath, buffer);
			 wcscat(ErrorPath, L".");
			 _itow(now->tm_mon+1, buffer, 10);
			 wcscat(ErrorPath, buffer);
			 wcscat(ErrorPath, L"_");
			 _itow(now->tm_hour, buffer, 10);
			 wcscat(ErrorPath, buffer);
			 wcscat(ErrorPath, L"#");
			 _itow(now->tm_min, buffer, 10);
			 wcscat(ErrorPath, buffer);
			 wcscat(ErrorPath, L"#");
			 _itow(now->tm_sec, buffer, 10);
			 wcscat(ErrorPath, buffer);
			 wcscat(ErrorPath, L".bmp");
			 PBitmapToFile(pbmp1, ErrorPath);
			 free(buffer);;
		
			 free(ErrorPath);
		 }
		 delete pbmp1;
	 }
	 catch (int Error) {
		 if (Error == 0)
		 {
				 
			free(nFolder);
			FindClose(SearchHandle);
			return Result;
		 }
		 
	 }
	
	 free(nFolder);
	 FindClose(SearchHandle);
	 return 0;
}

int DeletePBitmap(Gdiplus::Bitmap *pbmp) {
	try {
		delete pbmp;
		return 1;
	}
	catch (...) {
		return 0;
	}
}

extern "C" __declspec(dllexport) int PosFree(void *Data) {
	try {
		free(Data);
		return 1;
	}
	catch (...) {
		return 0;
	}
}