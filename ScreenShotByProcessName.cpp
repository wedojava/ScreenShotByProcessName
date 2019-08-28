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
string				winNames = "";


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
	int captureTimes = 200;
	char* p;
	if (argc == 3)
	{
		captureTimes = strtol(argv[2], &p, 10);
	}
	JustDoIt((char*)".\\ScrS\\", argv[1], captureTimes);
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
		winNames = "";  // init windows processes names
		if(!GetProcessList())
		{
			return -1;
		}
		int ifHasKeyword = winNames.find(keyword);
		if (ifHasKeyword != -1)
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
	if (_access(path.c_str(), R_OK) == -1)  // if _access(path.c_str(), R_OK) == 0 the folder exist!
	{
		int flag = _mkdir(path.c_str());
		return flag;
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
		winNames = winNames + " | " + pe32.szExeFile;
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return(TRUE);
}

int ScreenShot(CHAR* dirPath, CHAR* filename)
{
	CaptureImage(GetDesktopWindow(), dirPath, filename); // ����Ϊ E:hello.bmp
	INT flag = Convert2png(dirPath, filename);
	if (flag == 0)
	{
		char filepath[256] = { 0 };
		sprintf(filepath, "%s%s%s", dirPath, filename, ".bmp");
		if (!_access(filepath, R_OK))
		{
			remove(filepath);
		}
	}
	return 0;
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
	swprintf(wfilepath, L"%hs", filepath);
	Image* image = new Image(wfilepath);

	// Get the CLSID of the PNG encoder.
	GetEncoderClsid(L"image/png", &encoderClsid);
	//GetEncoderClsid(L"image/jpeg", &encoderClsid);

	sprintf(filepath, "%s%s%s", dirPath, filename, ".png");
	swprintf(wfilepath, L"%hs", filepath);
	stat = image->Save(wfilepath, &encoderClsid, NULL);
	delete image;
	sprintf(filepath, "%s%s%s", dirPath, filename, ".bmp");
	if (!_access(filepath, R_OK))
	{
		remove(filepath);
	}
	GdiplusShutdown(gdiplusToken);
	if (stat != Ok)
	{
		printf("Failure: stat = %d\n", stat);
		return 1;
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
 * GDI ��ȡָ������
 *
 * ���� hwnd   Ҫ�����Ĵ��ھ��
 * ���� dirPath    ��ͼ���Ŀ¼
 * ���� filename ��ͼ����
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

	hdcScreen = GetDC(NULL); // ȫ��ĻDC
	hdcWindow = GetDC(hwnd); // ��ͼĿ�괰��DC

	// ���������ڴ�DC
	hdcMemDC = CreateCompatibleDC(hdcWindow);

	if (!hdcMemDC)
	{
		goto done;
	}

	// ��ȡ�ͻ����������ڼ����С
	GetClientRect(hwnd, &rcClient);

	// ������չģʽ
	SetStretchBltMode(hdcWindow, HALFTONE);

	// ��Դ DC ��������Ļ��Ŀ�� DC �ǵ�ǰ�Ĵ��� (HWND)
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

	// ͨ������DC ����һ������λͼ
	hbmScreen = CreateCompatibleBitmap(
		hdcWindow,
		rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top
	);

	if (!hbmScreen)
	{
		goto done;
	}

	// ��λͼ�鴫�͵����Ǽ��ݵ��ڴ�DC��
	SelectObject(hdcMemDC, hbmScreen);
	if (!BitBlt(
		hdcMemDC,   // Ŀ��DC
		0, 0,        // Ŀ��DC�� x,y ����
		rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, // Ŀ�� DC �Ŀ��
		hdcWindow,  // ��ԴDC
		0, 0,        // ��ԴDC�� x,y ����
		SRCCOPY))   // ճ����ʽ
	{
		goto done;
	}

	// ��ȡλͼ��Ϣ������� bmpScreen ��
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

	// �� 32-bit Windows ϵͳ��, GlobalAlloc �� LocalAlloc ���� HeapAlloc ��װ����
	// handle ָ�����Ĭ�ϵĶ�. ���Կ����� HeapAlloc Ҫ��
	hDIB = GlobalAlloc(GHND, dwBmpSize);
	lpbitmap = (char*)GlobalLock(hDIB);

	// ��ȡ����λͼ��λ���ҿ��������һ�� lpbitmap ��.
	GetDIBits(
		hdcWindow,  // �豸�������
		hbmScreen,  // λͼ���
		0,          // ָ�������ĵ�һ��ɨ����
		(UINT)bmpScreen.bmHeight, // ָ��������ɨ������
		lpbitmap,   // ָ����������λͼ���ݵĻ�������ָ��
		(BITMAPINFO*)& bi, // �ýṹ�屣��λͼ�����ݸ�ʽ
		DIB_RGB_COLORS // ��ɫ���ɺ졢�̡�����RGB������ֱ��ֵ����
	);


	wsprintf(FilePath, "%s\%s.bmp", dirPath, filename);

	// ����һ���ļ��������ļ���ͼ
	hFile = CreateFile(
		FilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	// �� ͼƬͷ(headers)�Ĵ�С, ����λͼ�Ĵ�С����������ļ��Ĵ�С
	dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// ���� Offset ƫ����λͼ��λ(bitmap bits)ʵ�ʿ�ʼ�ĵط�
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	// �ļ���С
	bmfHeader.bfSize = dwSizeofDIB;

	// λͼ�� bfType �������ַ��� "BM"
	bmfHeader.bfType = 0x4D42; //BM

	dwBytesWritten = 0;
	WriteFile(hFile, (LPSTR)& bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)& bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

	// �������ڴ沢�ͷ�
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	// �ر��ļ����
	CloseHandle(hFile);

	// ������Դ
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);
	ReleaseDC(hwnd, hdcWindow);

	return 0;
}