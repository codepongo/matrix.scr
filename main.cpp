#include <windows.h> 
#include <scrnsave.h>
#include <time.h>
#include "resource.h"
#if defined(NDEBUG)
#pragma comment(lib, "scrnsavw.lib")
#endif
#pragma comment(lib, "comctl32.lib")

#define ID_TIMER1 100
#define ID_TIMER2 101

typedef struct _tagPlace{
	BOOL bStart;
	INT nX;
	INT nY;
	INT nPict1;
	INT nPict2;
}PLACE;

const INT MAX_NUM = 5000;

LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HDC hdc_wnd;
	PAINTSTRUCT ps;
	RECT rc;
	static INT w, h, ch;
	static HDC hdc_main, hdc_mem, hdc_char[36][3];
	static HBITMAP hBmp_main, hBmp_mem, hBmp_char[36][3];
	static PLACE Place[MAX_NUM];
	HINSTANCE hInst;
	INT n = 0, m = 0;

	switch (msg) {
	case WM_CREATE:		
		GetWindowRect(hWnd, &rc);
		
		w = rc.right - rc.left;
		h = rc.bottom - rc.top;
		ch = (INT)(w / 15);

		srand((unsigned)time(NULL));
		hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		hBmp_mem = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FONT));
		hdc_mem = CreateCompatibleDC(NULL);
		SelectObject(hdc_mem, hBmp_mem);

		hdc_wnd = GetDC(hWnd);
		hBmp_main = CreateCompatibleBitmap(hdc_wnd, w, h);
		hdc_main = CreateCompatibleDC(NULL);
		SelectObject(hdc_main, hBmp_main);
		ReleaseDC(hWnd, hdc_wnd);

		for(n = 0; n < 36; n++) {
			for(m = 0; m < 3; m++) {
				hBmp_char[n][m] = CreateCompatibleBitmap(hdc_main, 10, 15);
				hdc_char[n][m] = CreateCompatibleDC(NULL);
				SelectObject(hdc_char[n][m], hBmp_char[n][m]);
				BitBlt(hdc_char[n][m], 0, 0, 10, 15, hdc_mem, n*10, m*15, SRCCOPY);
		
			}
		}
		DeleteObject(hBmp_mem);
		DeleteDC(hdc_mem);

		SetTimer(hWnd, ID_TIMER1, 70, NULL);
		SetTimer(hWnd, ID_TIMER2, 35, NULL);
		break;

	case WM_TIMER:
		switch(wp){
		case ID_TIMER1:
			PatBlt(hdc_main, 0, 0, w, h, BLACKNESS);
			for(n = 0; n < MAX_NUM; n ++) {
				if(Place[n].bStart) {	
					Place[n].nY += 15;
					for(m = 0; m < 3; m++) {
						BitBlt(hdc_main, Place[n].nX, Place[n].nY + m * 15, 10, 15,
							hdc_char[rand()%36][0], 0, 0, SRCCOPY);
					}
					for(m = 3; m < 16; m++) {
						BitBlt(hdc_main, Place[n].nX, Place[n].nY + m * 15, 10, 15,
							hdc_char[rand()%36][1], 0, 0, SRCCOPY);
					}
					BitBlt(hdc_main, Place[n].nX, Place[n].nY + 16 * 15, 10, 15,
						hdc_char[rand()%36][2], 0, 0, SRCCOPY);
					if(Place[n].nY > h) {
						Place[n].bStart = FALSE;
					}
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
			
		case ID_TIMER2:
			while(Place[n].bStart) {
				n++;
				if(n > MAX_NUM) {
					break;
				}
			}
			if(n >= MAX_NUM) {
				break;
			}
			Place[n].bStart = TRUE;
			Place[n].nX = (rand() % ch) * 20;
			Place[n].nY = -15 * 16;
			break;
		}
		break;
	case WM_ERASEBKGND:
		return 0;
	case WM_PAINT:
		hdc_wnd = BeginPaint(hWnd, &ps);
		BitBlt(hdc_wnd, 0, 0, w, h, hdc_main, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		KillTimer(hWnd, ID_TIMER1);
		KillTimer(hWnd, ID_TIMER2);

		DeleteObject(hBmp_main);
		DeleteDC(hdc_main);
		for(n = 0; n < 3; n++){
			for(m = 0; m < 36; m++){
				DeleteObject(hBmp_char[n][m]);
				DeleteDC(hdc_char[n][m]);

			}
		}
		PostQuitMessage(0);
        break;
    
	default:
        break;
	}
#if defined(NDEBUG)
	return DefScreenSaverProc(hWnd, msg, wp, lp);
#else
	return DefWindowProc(hWnd, msg, wp, lp);
#endif
}

#if !defined(NDEBUG)
int CALLBACK WinMain(
	_In_ HINSTANCE hinstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {

	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return -1;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		CloseHandle(hToken);
		return -1;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		CloseHandle(hToken);
		return -1;
	}

	MSG msg;

	WNDCLASSEX wcx;

	// Fill in the window class structure with parameters 
	// that describe the main window. 

	wcx.cbSize = sizeof(wcx);          // size of structure 
	wcx.style = CS_HREDRAW |
		CS_VREDRAW;                    // redraw if size changes 
	wcx.lpfnWndProc = ScreenSaverProc;     // points to window procedure 
	wcx.cbClsExtra = 0;                // no extra class memory 
	wcx.cbWndExtra = 0;                // no extra window memory 
	wcx.hInstance = hinstance;         // handle to instance 
	wcx.hIcon = LoadIcon(NULL,
		IDI_APPLICATION);              // predefined app. icon 
	wcx.hCursor = LoadCursor(NULL,
		IDC_ARROW);                    // predefined arrow 
	wcx.hbrBackground = (HBRUSH)GetStockObject(
		WHITE_BRUSH);                  // white background brush 
	wcx.lpszMenuName = L"MainMenu";    // name of menu resource 
	wcx.lpszClassName = L"MainWClass";  // name of window class 
	wcx.hIconSm = (HICON)LoadImage(hinstance, // small class icon 
		MAKEINTRESOURCE(5),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR);


	if (!RegisterClassEx(&wcx))
	{
		return -1;
	}

	RECT rc;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

	int w = rc.right - rc.left; //GetSystemMetrics(SM_CXSCREEN);
	int h = rc.bottom - rc.top;// GetSystemMetrics(SM_CYSCREEN);
	HWND hwnd;


	// Create the main window. 

	hwnd = CreateWindow(
		L"MainWClass",        // name of window class 
		L"Sample",            // title-bar string 
		WS_OVERLAPPEDWINDOW, // top-level window 
		0,       // default horizontal position 
		0,       // default vertical position 
		w,       // default width 
		h,       // default height 
		(HWND)NULL,         // no owner window 
		(HMENU)NULL,        // use class menu 
		hinstance,           // handle to application instance 
		(LPVOID)NULL);      // no window-creation data 

	if (!hwnd)
		return -1;

	// Show the window and send a WM_PAINT message to the window 
	// procedure. 
	LONG l = GetWindowLong(hwnd, GWL_STYLE);
	SetWindowLong(hwnd, GWL_STYLE, l & ~WS_MAXIMIZEBOX);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	

	BOOL fGotMessage;
	while ((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(hPrevInstance);
}
#endif 