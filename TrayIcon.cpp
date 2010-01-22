//Дополнительный модуль для 'EasyBar.cpp'
//Работа с иконкой в области уведомлений (трей)

#define _WIN32_WINNT	0x0500

#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "common.h"
#include "ebmenu.h"
#include "easybar.h"
#include "trayicon.h"

extern WCHAR lpwMutexName[128];
extern HANDLE hMutex;
extern HINSTANCE hAppInstance;
extern HWND hMainWnd;

static CEBMenu *pEBMenuTray = 0;

HWND hTrayCBWnd = 0;

static WNDCLASSEX WCEX = { 0 };
static HMENU hTrayMenu = 0;
static NOTIFYICONDATA NID = { 0 };

void InitTrayCBWnd(BOOL bCreate)
{
	LONG i, lTMItemCnt;
	HMENU hMainMenu = GetMenu(hMainWnd);
	if (bCreate)
	{
		LONG lMMItemCnt;
		WCHAR lpwText[MAX_PATH] = { 0 };
		HMENU hTraySubMenu;
		hTrayMenu = CreatePopupMenu();
		AppendMenu(hTrayMenu, MF_STRING, IDM_TRAY_HIDESHOW, L"&Hide/Show Player");
		AppendMenu(hTrayMenu, MF_SEPARATOR, 0, 0);
		lMMItemCnt = GetMenuItemCount(hMainMenu);
		for (i = 0; i < lMMItemCnt; i++)
		{
			GetMenuString(hMainMenu, i, lpwText, MAX_PATH, MF_BYPOSITION);
			hTraySubMenu = GetSubMenu(hMainMenu, i);
			AppendMenu(hTrayMenu, MF_STRING | MF_POPUP, (UINT_PTR)hTraySubMenu, lpwText);
		}
		AppendMenu(hTrayMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hTrayMenu, MF_STRING, IDM_TRAY_EXIT, L"&Exit");
		SetMenuDefaultItem(hTrayMenu, IDM_TRAY_HIDESHOW, FALSE);
		WCEX.cbSize = sizeof(WNDCLASSEX); 
		WCEX.lpfnWndProc = (WNDPROC)TrayCBWndProc;
		WCEX.hInstance = hAppInstance;
		WCEX.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		WCEX.lpszClassName = TRAY_CB_WND_CLASS;
		RegisterClassEx(&WCEX);
		hTrayCBWnd = CreateWindow(WCEX.lpszClassName, 0, WS_POPUP,
			16, 16, 32, 32, 0, hTrayMenu, hAppInstance, 0);
		if (!dwNoOwnerDrawMenu)
		{
			pEBMenuTray->hBmpCheck = (HBITMAP)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_CHECKMARK),
				IMAGE_BITMAP, 16, 16, 0);
			pEBMenuTray->hBmpRadioCheck = (HBITMAP)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_RADIOCHECKMARK),
				IMAGE_BITMAP, 16, 16, 0);
			pEBMenuTray->InitEBMenu(hTrayCBWnd, TRUE);
		}
	}
	else
	{
		if (!dwNoOwnerDrawMenu) pEBMenuTray->InitEBMenu(0);
		lTMItemCnt = GetMenuItemCount(hTrayMenu);
		for (i = (lTMItemCnt - 1); i >= 0; i--)
			RemoveMenu(hTrayMenu, i, MF_BYPOSITION);

		DestroyMenu(hTrayMenu);
		hTrayMenu = 0;
		DestroyWindow(hTrayCBWnd);
		hTrayCBWnd = 0;
	}
}

static LRESULT CALLBACK TrayCBWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TASKBARCREATED)
	{
		InitTrayIcon();
		return 0;
	}
	switch (uMsg)
	{
		case WM_TRAY_NOTIFY:
			switch(lParam)
			{
				case WM_LBUTTONDBLCLK:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_TRAY_HIDESHOW, 0), 0);
					break;
				case WM_RBUTTONUP:
				{
					SetForegroundWindow(hWnd);
					POINT pt = { 0 };
					GetCursorPos(&pt);
					TrackPopupMenu(hTrayMenu, 0, pt.x, pt.y, 0, hWnd, 0);
					PostMessage(hWnd, WM_NULL, 0, 0);
					break;
				}
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDM_TRAY_HIDESHOW:
					if (IsWindowVisible(hMainWnd))
					{
						//ShowWindow(hMainWnd, SW_HIDE);
						CloseWindow(hMainWnd);
					}
					else
					{
						ShowWindow(hMainWnd, SW_RESTORE);
						SetForegroundWindow(hMainWnd);
					}
					break;
				case IDM_TRAY_EXIT:
					if (dwMultipleInstances)
					{
						//Предварительно закрываем основной мьютекс
						CloseHandle(hMutex);
						hMutex = 0;
						
						HANDLE hMutex2 = CreateMutex(0, TRUE, lpwMutexName);
						if (GetLastError() == ERROR_ALREADY_EXISTS)
						{
							if (MessageBox(hWnd, L"Close all instances?", APP_NAME, MB_YESNO | MB_ICONQUESTION |
								MB_DEFBUTTON2) == IDYES)
							{
								PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_EXIT, 0), 0);
								break;
							}
						}
						CloseHandle(hMutex2);
					}
					PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSEWINDOW, 0), 0);
					break;
				default:
					PostMessage(hMainWnd, WM_COMMAND, wParam, lParam);
					break;
			}
			break;
		case WM_INITMENUPOPUP:
			SendMessage(hMainWnd, WM_INITMENUPOPUP, 0, 0);
			break;
		case WM_MEASUREITEM:
		{
			if (!dwNoOwnerDrawMenu)
			{
				LPMEASUREITEMSTRUCT pMIS = (LPMEASUREITEMSTRUCT)lParam;
				if (pMIS->CtlType == ODT_MENU)
				{
					return pEBMenuTray->MeasureItem(wParam, lParam);
				}
			}
			return FALSE;
		}
		case WM_DRAWITEM:
		{
			if (!dwNoOwnerDrawMenu)
			{
				LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
				if (pDIS->CtlType == ODT_MENU)
				{
					return pEBMenuTray->DrawItem(wParam, lParam);
				}
			}
			return FALSE;
		}
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void UpdateTrayMenuColors()
{
	if (!pEBMenuTray) return;
	if (!dwNoOwnerDrawMenu)
	{
		if (dwUseSystemColors)
		{
			pEBMenuTray->crFontColorOne = GetSysColor(COLOR_HIGHLIGHTTEXT);
			pEBMenuTray->crFontColorTwo = Blend(GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_HIGHLIGHT), 0.6);
			pEBMenuTray->crFontColorThree = GetSysColor(COLOR_MENUTEXT);
			pEBMenuTray->crBkColorOne = Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.2);
			pEBMenuTray->crBkColorTwo = GetSysColor(COLOR_MENU);
			pEBMenuTray->crBkColorThree = GetMenuBarColor();
			pEBMenuTray->crSelColorOne = GetSysColor(COLOR_HIGHLIGHT);
			pEBMenuTray->crSelColorTwo = Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1);
			pEBMenuTray->crBrColorOne = Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1);
			pEBMenuTray->crBrColorTwo = Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1);
		}
		else
		{
			pEBMenuTray->crFontColorOne = dwTextColor;
			pEBMenuTray->crFontColorTwo = dwTextShadowColor;
			pEBMenuTray->crFontColorThree = GetSysColor(COLOR_MENUTEXT);
			pEBMenuTray->crBkColorOne = dwBackgroundColor;
			pEBMenuTray->crBkColorTwo = GetSysColor(COLOR_MENU);
			pEBMenuTray->crBkColorThree = GetMenuBarColor();
			pEBMenuTray->crSelColorOne = dwGradientColor1;
			pEBMenuTray->crSelColorTwo = dwGradientColor2;
			pEBMenuTray->crBrColorOne = dwBorderColor1;
			pEBMenuTray->crBrColorTwo = dwBorderColor2;
		}
	}
}

//-------------------------------------------------------------------------------
BOOL InitTrayIcon()
{
	if (!pEBMenuTray) pEBMenuTray = new CEBMenu;
	UpdateTrayMenuColors();
	if (!hTrayCBWnd) InitTrayCBWnd();
	NID.cbSize = sizeof(NID);
	NID.hIcon = (HICON)LoadImage(hAppInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, 0);
	NID.hWnd = hTrayCBWnd;
	NID.uCallbackMessage = WM_TRAY_NOTIFY;
	NID.uID = TRAY_ICON_ID;
	NID.uFlags = NIF_ICON | NIF_MESSAGE;
	return Shell_NotifyIcon(NIM_ADD, &NID);
}

BOOL UpdateTrayIcon(LPCWSTR lpwTip)
{
	if (!NID.cbSize) return FALSE;
	wcsncpy(NID.szTip, lpwTip, (sizeof(NID.szTip) / sizeof(WCHAR)) - 1);
	NID.uFlags = NIF_TIP;
	return Shell_NotifyIcon(NIM_MODIFY, &NID);
}

BOOL ShowBalloon(LPCWSTR lpwTitle, LPCWSTR lpwText, BALLOONICON bIcon, DWORD dwTimeout)
{
	if (!NID.cbSize) return FALSE;
	wcsncpy(NID.szInfoTitle, lpwTitle, (sizeof(NID.szInfoTitle) / sizeof(WCHAR)) - 1);
	wcsncpy(NID.szInfo, lpwText, (sizeof(NID.szInfo) / sizeof(WCHAR)) - 1);
	switch (bIcon)
	{
		case BI_NONE:
			NID.dwInfoFlags = NIIF_NONE;
			break;
		case BI_INFORMATION:
			NID.dwInfoFlags = NIIF_INFO;
			break;
		case BI_WARNING:
			NID.dwInfoFlags = NIIF_WARNING;
			break;
		case BI_ERROR:
			NID.dwInfoFlags = NIIF_ERROR;
			break;
		default:
			return FALSE;
	}
	NID.uTimeout = (dwTimeout < 10000)?10000:dwTimeout;
	NID.uFlags = NIF_INFO;
	return Shell_NotifyIcon(NIM_MODIFY, &NID);
}

BOOL RemoveTrayIcon()
{
	if (hTrayCBWnd) InitTrayCBWnd(FALSE);
	SDO(pEBMenuTray);
	return Shell_NotifyIcon(NIM_DELETE, &NID);
}
//-------------------------------------------------------------------------------