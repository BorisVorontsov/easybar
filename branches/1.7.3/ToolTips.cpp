//–абота со всплывающими подсказками

#include <windows.h>
#include <commctrl.h>

#include "tooltips.h"

#pragma comment (lib, "comctl32.lib")

CToolTips::CToolTips()
{
	//
}

CToolTips::~CToolTips()
{
	//
}

int CToolTips::Initialize()
{
	if ((m_hInstance == 0) || (m_hOwner == 0)) return 0;
	INITCOMMONCONTROLSEX ICC = {};
	InitCommonControlsEx(&ICC);
	m_hToolTipWnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, 0, WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		m_hOwner, 0, m_hInstance, 0);
	if (!m_hToolTipWnd) return 0;
	SendMessage(m_hToolTipWnd, TTM_SETDELAYTIME, TTDT_INITIAL, 1000);
	SendMessage(m_hToolTipWnd, TTM_SETDELAYTIME, TTDT_AUTOPOP, 6000);
	return 1;
}

int CToolTips::AddToolTip(HWND hCtrl, LPCWSTR lpText)
{
	if (!m_hToolTipWnd) return 0;
	TOOLINFO TI = {};
	TI.cbSize = sizeof(TI);
	TI.uFlags = TTF_SUBCLASS | TTF_IDISHWND; 
	TI.hwnd = hCtrl; 
	TI.uId = (UINT_PTR)hCtrl; 
	TI.lpszText = new WCHAR[wcslen(lpText) + 1];
	wcscpy(TI.lpszText, lpText);
	GetClientRect(hCtrl, &TI.rect);
	return (int)SendMessage(m_hToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&TI);
}

void CToolTips::GetToolTip(HWND hCtrl, LPWSTR lpText)
{
	if (!m_hToolTipWnd) return;
	TOOLINFO TI = {};
	TI.cbSize = sizeof(TI);
	TI.hwnd = hCtrl;
	TI.uId = (UINT_PTR)hCtrl; 
	TI.lpszText = lpText;
	SendMessage(m_hToolTipWnd, TTM_GETTEXT, 0, (LPARAM)&TI);
}

int CToolTips::RemoveToolTip(HWND hCtrl)
{
	if (!m_hToolTipWnd) return 0;
	TOOLINFO TI = {};
	ULONG i, lTTCount;
	lTTCount = (ULONG)SendMessage(m_hToolTipWnd, TTM_GETTOOLCOUNT, 0, 0);
	for (i = 0; i < lTTCount; i++)
	{
		SendMessage(m_hToolTipWnd, TTM_ENUMTOOLS, i, (LPARAM)&TI);
		if (TI.hwnd == hCtrl)
		{
			delete[] TI.lpszText;
			SendMessage(m_hToolTipWnd, TTM_DELTOOL, 0, (LPARAM)&TI);
			break;
		}
		else
		{
			if (i == (lTTCount - 1))
				return 0; else continue;
		}
	}
	return 1;
}

int CToolTips::Destroy()
{
	if (!m_hToolTipWnd) return 0;
	DestroyWindow(m_hToolTipWnd);
	return 1;
}