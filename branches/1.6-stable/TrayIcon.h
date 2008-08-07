#ifndef TRAYICON_H
#define TRAYICON_H

#define IDM_TRAY_HIDESHOW		0x00001004

#define TRAY_ICON_ID			0x00000C94

#define TRAY_CB_WND_CLASS		L"TRAYCB_"

#define WM_TASKBARCREATED		RegisterWindowMessage(L"TaskbarCreated")
#define WM_TRAY_NOTIFY			WM_USER + 0x226

typedef enum _BALLOONICON
{
	BI_NONE = 0,
	BI_INFORMATION = 1,
	BI_WARNING = 2,
	BI_ERROR = 3
} BALLOONICON;

void InitTrayCBWnd(BOOL bCreate = TRUE);
static LRESULT CALLBACK TrayCBWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UpdateTrayMenuColors();
BOOL InitTrayIcon();
BOOL UpdateTrayIcon(LPCWSTR lpwTip);
BOOL ShowBalloon(LPCWSTR lpwTitle, LPCWSTR lpwText,
				 BALLOONICON bIcon = BI_INFORMATION, DWORD dwTimeout = 10000);
BOOL RemoveTrayIcon();

extern HWND hTrayCBWnd;

#endif