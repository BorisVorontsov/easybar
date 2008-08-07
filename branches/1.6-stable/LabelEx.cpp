#if (_WIN32_WINNT < 0x0500)
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>

#ifdef UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include <tchar.h>

#include "labelex.h"

BOOL InitLabelEx(HINSTANCE hInstance)
{
	WNDCLASSEX WCEX = { 0 };
	WCEX.cbSize = sizeof(WCEX);
	WCEX.style = CS_HREDRAW | CS_VREDRAW;
	WCEX.hInstance = hInstance;
	WCEX.hCursor = LoadCursor(0, (LPCTSTR)IDC_HAND);
	WCEX.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WCEX.lpfnWndProc = LabelExProc;
	WCEX.lpszClassName = LABELEX_CLASS;
	if (!RegisterClassEx(&WCEX)) return FALSE;
	return TRUE;
}

LRESULT CALLBACK LabelExProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPLEPROPERTIES pLEP = NULL;
	if (uMsg == WM_NCCREATE)
	{
		pLEP = new LEPROPERTIES;
		ZeroMemory(pLEP, sizeof(LEPROPERTIES));
		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)pLEP);
	}
	else
	{
		pLEP = (LPLEPROPERTIES)GetWindowLongPtr(hWnd, GWL_USERDATA);
	}
	switch (uMsg)
	{
		case WM_CREATE:
		{
			//Инициализация
			//----------------------------------------------------------
			pLEP->hDC = GetDC(hWnd);
			pLEP->hNormalFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			/*pLEP->hNormalFont = CreateFont(-MulDiv(8, GetDeviceCaps(pLEP->hDC, LOGPIXELSY), 72)
				, 0, 0, 0, 0, FALSE, TRUE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, 
				TEXT("Microsoft Sans Serif"));*/
			pLEP->hHotFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			/*pLEP->hHotFont = CreateFont(-MulDiv(8, GetDeviceCaps(pLEP->hDC, LOGPIXELSY), 72)
				, 0, 0, 0, 0, FALSE, TRUE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, 
				TEXT("Microsoft Sans Serif"));*/
			pLEP->crNormalFontColor = RGB(0, 0, 255);
			pLEP->crHotFontColor = RGB(255, 0, 0);
			pLEP->crBkColor = GetSysColor(COLOR_3DFACE);
			pLEP->bAutoSize = TRUE;
			pLEP->LEMF.dwFlag = LEMF_LEAVE;
			ZeroMemory(pLEP->lpCurrentText, sizeof(pLEP->lpCurrentText));
			break;
		}
		case LEM_SETNFONT:
			pLEP->hNormalFont = (HFONT)wParam;
			if (LOWORD(lParam) == TRUE)
				PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		case LEM_GETNFONT:
			return (LRESULT)pLEP->hNormalFont;
		case LEM_SETHFONT:
			pLEP->hHotFont = (HFONT)wParam;
			if (LOWORD(lParam) == TRUE)
				PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		case LEM_GETHFONT:
			return (LRESULT)pLEP->hHotFont;
		case LEM_SETNFONTCOLOR:
			pLEP->crNormalFontColor = (COLORREF)wParam;
			PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		case LEM_GETNFONTCOLOR:
			return (LRESULT)pLEP->crNormalFontColor;
		case LEM_SETHFONTCOLOR:
			pLEP->crHotFontColor = (COLORREF)wParam;
			RedrawWindow(hWnd, 0, 0, RDW_ERASE | RDW_INVALIDATE);
			break;
		case LEM_GETHFONTCOLOR:
			return (LRESULT)pLEP->crHotFontColor;
		case LEM_SETBKCOLOR:
			pLEP->crBkColor = (COLORREF)wParam;
			PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		case LEM_GETBKCOLOR:
			return (LRESULT)pLEP->crBkColor;
		case LEM_SETAUTOSIZE:
			pLEP->bAutoSize = (BOOL)LOWORD(lParam);
			break;
		case LEM_GETAUTOSIZE:
			return (LRESULT)pLEP->bAutoSize;
		case WM_SETTEXT:
		{
			RECT RCW = { 0 }, RCT = { 0 };
			_tcscpy(pLEP->lpCurrentText, (PTCHAR)lParam);
			if (pLEP->bAutoSize)
			{
				HFONT hOldFont = (HFONT)SelectObject(pLEP->hDBDC, pLEP->hNormalFont);
				DrawText(pLEP->hDBDC, pLEP->lpCurrentText, -1, &RCT, DT_LEFT | DT_SINGLELINE |
					DT_CALCRECT);
				SelectObject(pLEP->hDBDC, hOldFont);
				SetWindowPos(hWnd, HWND_TOP, 0, 0, RCT.right, RCT.bottom, SWP_NOMOVE);
			}
			else
			{
				GetClientRect(hWnd, &RCT);
				if ((RCT.right != pLEP->CRC.right) || (RCT.bottom != pLEP->CRC.bottom))
				{
					SetWindowPos(hWnd, HWND_TOP, 0, 0, pLEP->CRC.right, pLEP->CRC.bottom, SWP_NOMOVE);
				}
				else PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			}
			return TRUE;
		}
		case WM_SYSCOLORCHANGE:
			PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT TME = { 0 };
			TME.cbSize = sizeof(TME);
			TME.dwFlags = TME_HOVER | TME_LEAVE;
			TME.dwHoverTime = 10;
			TME.hwndTrack = hWnd;
			TrackMouseEvent(&TME);
			break;
		}
		case WM_MOUSEHOVER:
			if (pLEP->LEMF.dwFlag != LEMF_HOVER)
			{
				pLEP->LEMF.dwFlag = LEMF_HOVER;
				PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			}
			break;
		case WM_MOUSELEAVE:
			if (pLEP->LEMF.dwFlag != LEMF_LEAVE)
			{
				pLEP->LEMF.dwFlag = LEMF_LEAVE;
				PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			}
			break;
		case WM_LBUTTONDOWN:
			pLEP->LEMF.dwFlag = LEMF_DOWN;
			PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		case WM_LBUTTONUP:
		{
			pLEP->LEMF.dwFlag = LEMF_HOVER;
			ZeroMemory(&pLEP->NM, sizeof(pLEP->NM));
			pLEP->NM.code = LEN_CLICKED;
			pLEP->NM.hwndFrom = hWnd;
			pLEP->NM.idFrom = GetDlgCtrlID(hWnd);
			PostMessage(GetParent(hWnd), WM_NOTIFY, pLEP->NM.idFrom, (LPARAM)&pLEP->NM);
			PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		}
		case WM_SIZE:
			GetClientRect(hWnd, &pLEP->CRC);
			if (pLEP->hDBDC)
			{
				SelectObject(pLEP->hDBDC, pLEP->hOldDBBitmap);
				DeleteObject(pLEP->hDBBitmap);
			}
			else
			{
				pLEP->hDBDC = CreateCompatibleDC(pLEP->hDC);
			}
			pLEP->hDBBitmap = CreateCompatibleBitmap(pLEP->hDC, pLEP->CRC.right, pLEP->CRC.bottom);
			pLEP->hOldDBBitmap = (HBITMAP)SelectObject(pLEP->hDBDC, pLEP->hDBBitmap);
			PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		case WM_ENABLE:
			PostMessage(hWnd, LEM_DRAWCONTROL, 0, 0);
			break;
		case LEM_DRAWCONTROL:
			HFONT hOldFont;
			if (IsWindowEnabled(hWnd))
			{
				HBRUSH hBkBrush = CreateSolidBrush(pLEP->crBkColor);
				FillRect(pLEP->hDBDC, &pLEP->CRC, hBkBrush);
				DeleteObject(hBkBrush);
			}
			else
			{
				FillRect(pLEP->hDBDC, &pLEP->CRC, GetSysColorBrush(COLOR_3DFACE));
			}
			SetBkMode(pLEP->hDBDC, TRANSPARENT);
			if (IsWindowEnabled(hWnd))
			{
				switch (pLEP->LEMF.dwFlag)
				{
					case LEMF_LEAVE:
						hOldFont = (HFONT)SelectObject(pLEP->hDBDC, pLEP->hNormalFont);
						SetTextColor(pLEP->hDBDC, pLEP->crNormalFontColor);
						break;
					case LEMF_HOVER:
					case LEMF_DOWN:
						hOldFont = (HFONT)SelectObject(pLEP->hDBDC, pLEP->hHotFont);
						SetTextColor(pLEP->hDBDC, pLEP->crHotFontColor);
						break;
				}
			}
			else
			{
				hOldFont = (HFONT)SelectObject(pLEP->hDBDC, pLEP->hNormalFont);
				SetTextColor(pLEP->hDBDC, GetSysColor(COLOR_GRAYTEXT));
			}
			DrawText(pLEP->hDBDC, pLEP->lpCurrentText, -1, &pLEP->CRC, DT_LEFT | DT_SINGLELINE);
			SelectObject(pLEP->hDBDC, hOldFont);
			RedrawWindow(hWnd, 0, 0, RDW_ERASE | RDW_INVALIDATE);
			break;
		case WM_ERASEBKGND:
			return 1;
		case WM_PAINT:
		{
			PAINTSTRUCT PS = { 0 };
			HDC hDC = BeginPaint(hWnd, &PS);
			BitBlt(hDC, 0, 0, pLEP->CRC.right, pLEP->CRC.bottom, pLEP->hDBDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &PS);
			break;
		}
		case WM_DESTROY:
			SelectObject(pLEP->hDBDC, pLEP->hOldDBBitmap);
			DeleteObject(pLEP->hDBBitmap);
			DeleteDC(pLEP->hDBDC);
			ReleaseDC(hWnd, pLEP->hDC);
			DeleteObject(pLEP->hNormalFont);
			DeleteObject(pLEP->hHotFont);
			delete pLEP;
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
