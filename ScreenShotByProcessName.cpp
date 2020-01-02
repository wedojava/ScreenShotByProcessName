#include "stdafx.h"
#include "resource.h"
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <comdef.h>
#include <gdiplus.h>
#include <io.h>
#include <stdio.h>
#include <iostream>
#include <direct.h>
#include "sal.h"
#include <string>
#include <stdlib.h>  // for strtol
#include <tlhelp32.h>


using namespace Gdiplus;
using namespace std;
#pragma comment(lib,"gdiplus.lib")

#define R_OK 4 /* Test for read permission. */
#define W_OK 2 /* Test for write permission. */
#define X_OK 1 /* Test for execute permission. */
#define F_OK 0 /* Test for existence. */

//  Forward declarations:
BOOL				GetProcessList();
int					CaptureImage(HWND hWnd, CHAR* dirPath, CHAR* filename);
INT					GetEncoderClsid(const WCHAR* format, CLSID* pClsid);  // helper function
INT					Convert2png(CHAR* dirPath, CHAR* filename);
int					ScreenShot(CHAR* dirPath, CHAR* filename);
int					DirCreate(string path);
int					JustDoIt(char* savePath, char* keyword, int capTimes);
int					main(int argc, char* argv[]);
string				processNames = "";


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	int argc = __argc;
	char** argv = __argv;
	main(argc, argv);
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		return 0;
	}
	int captureTimes = 200;
	char* p;
	if (argc == 3)
	{
		captureTimes = strtol(argv[2], &p, 10);
	}
	// if set folder to save pictures may occur error in some system, but don't know why,
	// by stat returned 2 means InvalidParameter, I guess set save path to "" may fix the bug unperfectly. 
	// all the stat num's meaning can refer:
	// https://docs.microsoft.com/en-us/windows/win32/api/gdiplustypes/ne-gdiplustypes-status
	// JustDoIt((char*)".\\PrtSc\\", argv[1], captureTimes);  // Save png to PrtSc folder
	JustDoIt((char*)".\\", argv[1], captureTimes);
	return 0;
}

int JustDoIt(char* savePath, char* keyword, int capTimes)
{
	// create save path
	DirCreate(savePath);

	// filename = keyword
	char filename[100] = { 0 };
	strcpy(filename, keyword);
	char filenameMerged[100] = { 0 };
	int i = 1;
	while (true)
	{
		processNames = "";  // init windows processes names
		if(!GetProcessList())
		{
			return -1;
		}
		if (processNames.find(keyword) != -1)  // winNames.find(keyword) == -1 means found none.
		{
			sprintf(filenameMerged, "%s%d", filename, i);  // filenameMerged = filename + i
			ScreenShot(savePath, filenameMerged);
			i++;
		}
		if (i > capTimes)
		{
			return 0;
		}
		Sleep(2000);
	}
}

int DirCreate(string path)
{
	if (_access(path.c_str(), R_OK))  // -1: not exist, 0: exist!
	{
		return _mkdir(path.c_str());
	}
	return 0;
}

BOOL GetProcessList()
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		// printf(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Now walk the snapshot of processes, and
    // set exe file name about each process in turn to winNames
	do
	{
		// printf( TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile );
		processNames = processNames + " | " + pe32.szExeFile;
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return(TRUE);
}

int ScreenShot(CHAR* dirPath, CHAR* filename)
{
	CaptureImage(GetDesktopWindow(), dirPath, filename);  // capture and save as diaPath + filename.bmp
	if (!Convert2png(dirPath, filename))  // if convert to png success return 0
	{
		char filepath[256] = { 0 };
		sprintf(filepath, "%s%s%s", dirPath, filename, ".bmp");
		if (!_access(filepath, R_OK))
		{
			remove(filepath);
		}
		return 0;
	}
	printf("[-] Failure: convert bmp to png false.");
	return 1;
}

// By reference:
// https://docs.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-converting-a-bmp-image-to-a-png-image-use
INT Convert2png(CHAR* dirPath, CHAR* filename)
{
	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	Status  stat;
	char filepath[100] = { 0 };
	wchar_t* wfilepath = new wchar_t[50];
	sprintf(filepath, "%s%s%s", dirPath, filename, ".bmp");
	if (_access(filepath, R_OK))  // if file is not exist!
	{
		printf("[-] Failure: %d not exist, convert action stoped!\n", filepath);
		return -1;
	}
	swprintf(wfilepath, L"%hs", filepath);
	Image* image = new Image(wfilepath);

	// Get the CLSID of the PNG encoder.
	GetEncoderClsid(L"image/png", &encoderClsid);
	//GetEncoderClsid(L"image/jpeg", &encoderClsid);

	sprintf(filepath, "%s%s%s", dirPath, filename, ".png");
	swprintf(wfilepath, L"%hs", filepath);
	stat = image->Save(wfilepath, &encoderClsid, NULL);
	delete image;
	GdiplusShutdown(gdiplusToken);
	if (stat != Ok)
	{
		// stat meaning reference:
		// https://docs.microsoft.com/en-us/windows/win32/api/gdiplustypes/ne-gdiplustypes-status
		printf("[-] Failure: stat = %d\n", stat);
		return -1;
	}
	return 0;
}


// By reference:
// https://docs.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-retrieving-the-class-identifier-for-an-encoder-use
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

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

/**
 * GDI Capture indicated window
 *
 * param hwnd		handle of windows should be catch
 * param dirPath    path to save the screenshot
 * param filename	name of screenshot without path
 */
int CaptureImage(HWND hwnd, CHAR* dirPath, CHAR* filename)
{
	HDC mdc;
	HBITMAP hbmp;
	CHAR FilePath[MAX_PATH];
	HDC hdcScreen;
	HDC hdcWindow;
	HDC hdcMemDC = NULL;
	HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;
	RECT rcClient;
	BITMAPFILEHEADER   bmfHeader;
	BITMAPINFOHEADER   bi;
	DWORD dwBmpSize;
	HANDLE hDIB;
	CHAR* lpbitmap;
	HANDLE hFile;
	DWORD dwSizeofDIB;
	DWORD dwBytesWritten;

	hdcScreen = GetDC(NULL); // Full Screen DC
	hdcWindow = GetDC(hwnd); // Target window DC

	// Compatible memory DC init
	hdcMemDC = CreateCompatibleDC(hdcWindow);

	if (!hdcMemDC)
	{
		goto done;
	}

	// Get client area for calculating size
	GetClientRect(hwnd, &rcClient);

	// Set stretch mode
	SetStretchBltMode(hdcWindow, HALFTONE);

	// Source DC is the entire screen, target DC is the current window (HWND)
	if (!StretchBlt(hdcWindow,
		0, 0,
		rcClient.right, rcClient.bottom,
		hdcScreen,
		0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		SRCCOPY))
	{
		goto done;
	}

	// Create a compatible bitmap from the window's DC
	hbmScreen = CreateCompatibleBitmap(
		hdcWindow,
		rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top
	);

	if (!hbmScreen)
	{
		goto done;
	}

	// Pass bitmap blocks to our compatible memory DC
	SelectObject(hdcMemDC, hbmScreen);
	if (!BitBlt(
		hdcMemDC,   // Target DC
		0, 0,        // x, y coordinates of target DC
		rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, // width and heigh of target DC
		hdcWindow,  // Source DC
		0, 0,        // x, y coordinates of source DC
		SRCCOPY))   // How to Paste
	{
		goto done;
	}

	// Get bitmap infor and pass it in bmpScreen
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	// GlobalAlloc and LocalAlloc are encapsulated by HeapAlloc on 32-bit windows
	// Due to handle points to the process's default stack, the overhead is expensive than HeapAlloc
	hDIB = GlobalAlloc(GHND, dwBmpSize);
	lpbitmap = (char*)GlobalLock(hDIB);

	// Get the bits of compatible bitmap and copy into an lpbitmap.
	GetDIBits(
		hdcWindow,  // Device context handle
		hbmScreen,  // Bitmap Handle
		0,          // Specify the first scan line to retrieve
		(UINT)bmpScreen.bmHeight, // Specify the number of scan lines to retrieve
		lpbitmap,   // Pointer to the buffer used to retrieve the bitmap data
		(BITMAPINFO*)& bi, // This structure holds the bitmap data format
		DIB_RGB_COLORS // The color table consists of three direct values of red, green, and blue (RGB)
	);


	wsprintf(FilePath, "%s%s.bmp", dirPath, filename);

	// Create a file to save the file screenshot
	hFile = CreateFile(
		FilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	// Get entire file size via sum the size of picture headers and bitmap
	dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Set Offset to where the bitmap bits actually start
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	// File size
	bmfHeader.bfSize = dwSizeofDIB;

	// Bitmap's bfType must be the string "BM"
	bmfHeader.bfType = 0x4D42; //BM

	dwBytesWritten = 0;
	if (!WriteFile(hFile, (LPSTR)& bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL)){
		printf("[-] Failure: WriteFile() 1st step error!");
	}
	if (WriteFile(hFile, (LPSTR)& bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL)){
		printf("[-] Failure: WriteFile() 2nd step error!");
	}
	if (WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL)){
		printf("[-] Failure: WriteFile() 3rn step error!");
	}

	// Unlock stack memory and release
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	// Close file handle
	CloseHandle(hFile);

	// Clean up resources
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);
	ReleaseDC(hwnd, hdcWindow);

	return 0;
}