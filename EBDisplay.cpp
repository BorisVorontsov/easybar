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

#include "ebdisplay.h"

#pragma comment (lib, "msimg32.lib")

BOOL InitEBDisplay(HINSTANCE hInstance)
{
	WNDCLASSEX WCEX = {};
	WCEX.cbSize = sizeof(WCEX);
	WCEX.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	WCEX.hInstance = hInstance;
	WCEX.hCursor = LoadCursor(0, (LPCTSTR)IDC_ARROW);
	WCEX.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WCEX.lpfnWndProc = EBDisplayProc;
	WCEX.lpszClassName = EBDISPLAY_CLASS;
	if (!RegisterClassEx(&WCEX)) return FALSE;
	return TRUE;
}

LRESULT CALLBACK EBDisplayProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPEBDPROPERTIES pEBDP = NULL;
	if (uMsg == WM_NCCREATE)
	{
		pEBDP = new EBDPROPERTIES;
		ZeroMemory(pEBDP, sizeof(EBDPROPERTIES));
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pEBDP);
	}
	else
	{
		pEBDP = (LPEBDPROPERTIES)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	switch (uMsg)
	{
		case WM_CREATE:
			//Инициализация
			//----------------------------------------------------------
			pEBDP->hDC = GetDC(hWnd);
			pEBDP->hFont = CreateFont(-MulDiv(8, GetDeviceCaps(pEBDP->hDC, LOGPIXELSY), 72)
				, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Tahoma"));
			pEBDP->crFontColorOne = RGB(255, 255, 255);
			pEBDP->crFontColorTwo = RGB(128, 128, 128);
			pEBDP->crBkColorOne = RGB(255, 0, 0);
			pEBDP->crBkColorTwo = RGB(0, 0, 255);
			pEBDP->crBrColorOne = RGB(0, 0, 0);
			pEBDP->crBrColorTwo = RGB(255, 255, 255);
			ZeroMemory(pEBDP->lpCurrentText, sizeof(pEBDP->lpCurrentText));
			break;
		case WM_SIZE:
			GetClientRect(hWnd, &pEBDP->CRC);
			if (pEBDP->hDBDC)
			{
				SelectObject(pEBDP->hDBDC, pEBDP->hOldDBBitmap);
				DeleteObject(pEBDP->hDBBitmap);
			}
			else
			{
				pEBDP->hDBDC = CreateCompatibleDC(pEBDP->hDC);
			}
			pEBDP->hDBBitmap = CreateCompatibleBitmap(pEBDP->hDC, pEBDP->CRC.right, pEBDP->CRC.bottom);
			pEBDP->hOldDBBitmap = (HBITMAP)SelectObject(pEBDP->hDBDC, pEBDP->hDBBitmap);
			PostMessage(hWnd, EBDM_DRAWCONTROL, 0, 0);
			break;
		case EBDM_SETFONT:
			pEBDP->hFont = (HFONT)wParam;
			if (LOWORD(lParam) == TRUE)
				PostMessage(hWnd, EBDM_DRAWCONTROL, 0, 0);
			break;
		case EBDM_GETFONT:
			return (LRESULT)pEBDP->hFont;
		case EBDM_SETFONTCOLORS:
			pEBDP->crFontColorOne = (COLORREF)wParam;
			pEBDP->crFontColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBDM_DRAWCONTROL, 0, 0);
			break;
		case EBDM_GETFONTCOLORS:
			*(PLONG)wParam = pEBDP->crFontColorOne;
			*(PLONG)lParam = pEBDP->crFontColorTwo;
			break;
		case EBDM_SETBKCOLORS:
			pEBDP->crBkColorOne = (COLORREF)wParam;
			pEBDP->crBkColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBDM_DRAWCONTROL, 0, 0);
			break;
		case EBDM_GETBKCOLORS:
			*(PLONG)wParam = pEBDP->crBkColorOne;
			*(PLONG)lParam = pEBDP->crBkColorTwo;
			break;
		case EBDM_SETBRCOLORS:
			pEBDP->crBrColorOne = (COLORREF)wParam;
			pEBDP->crBrColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBDM_DRAWCONTROL, 0, 0);
			break;
		case EBDM_GETBRCOLORS:
			*(PLONG)wParam = pEBDP->crBrColorOne;
			*(PLONG)lParam = pEBDP->crBrColorTwo;
			break;
		case WM_SETTEXT:
			_tcscpy(pEBDP->lpCurrentText, (PTCHAR)lParam);
			PostMessage(hWnd, EBDM_DRAWCONTROL, 0, 0);
			return TRUE;
		case WM_GETTEXT:
			_tcsncpy((PTCHAR)lParam, pEBDP->lpCurrentText, wParam);
			return _tcslen((PTCHAR)lParam);
		case WM_SYSCOLORCHANGE:
			PostMessage(hWnd, EBDM_DRAWCONTROL, 0, 0);
			break;
		case WM_XBUTTONUP:
			PostMessage(GetParent(hWnd), uMsg, wParam, lParam);
			break;
		case WM_LBUTTONUP:
			ZeroMemory(&pEBDP->NM, sizeof(pEBDP->NM));
			pEBDP->NM.code = EBDN_CLICKED;
			pEBDP->NM.hwndFrom = hWnd;
			pEBDP->NM.idFrom = GetDlgCtrlID(hWnd);
			PostMessage(GetParent(hWnd), WM_NOTIFY, pEBDP->NM.idFrom, (LPARAM)&pEBDP->NM);
			break;
		case WM_LBUTTONDBLCLK:
			ZeroMemory(&pEBDP->NM, sizeof(pEBDP->NM));
			pEBDP->NM.code = EBDN_DBLCLK;
			pEBDP->NM.hwndFrom = hWnd;
			pEBDP->NM.idFrom = GetDlgCtrlID(hWnd);
			PostMessage(GetParent(hWnd), WM_NOTIFY, pEBDP->NM.idFrom, (LPARAM)&pEBDP->NM);
			break;
		case WM_ENABLE:
			PostMessage(hWnd, EBDM_DRAWCONTROL, 0, 0);
			break;
		case EBDM_DRAWCONTROL:
		{
			if (pEBDP->hDBDC)
			{
				TRIVERTEX TV[2] = {};
				GRADIENT_RECT GR = {};
				RECT RCB = {}, RCT = {};
				HFONT hOldFont;
				SetBkMode(pEBDP->hDBDC, TRANSPARENT);
				hOldFont = (HFONT)SelectObject(pEBDP->hDBDC, pEBDP->hFont);
				//Заливаем часть без градиента
				//--------------------------------------------------------------------
				CopyRect(&RCB, &pEBDP->CRC);
				RCB.bottom -= ((pEBDP->CRC.bottom / 3) * 2);
				if (IsWindowEnabled(hWnd))
				{
					HBRUSH hBkBrush = CreateSolidBrush(pEBDP->crBkColorOne);
					FillRect(pEBDP->hDBDC, &RCB, hBkBrush);
					DeleteObject(hBkBrush);
				}
				else
				{
					FillRect(pEBDP->hDBDC, &RCB, GetSysColorBrush(COLOR_3DFACE));
				}
				//--------------------------------------------------------------------
				TV[0].x = pEBDP->CRC.left;
				TV[0].y = pEBDP->CRC.top + (RCB.bottom - RCB.top);
				if (IsWindowEnabled(hWnd))
				{
					TV[0].Red = GetRValue(pEBDP->crBkColorOne) << 8;
					TV[0].Green = GetGValue(pEBDP->crBkColorOne) << 8;
					TV[0].Blue = GetBValue(pEBDP->crBkColorOne) << 8;
				}
				else
				{
					TV[0].Red = GetRValue(GetSysColor(COLOR_3DFACE)) << 8;
					TV[0].Green = GetGValue(GetSysColor(COLOR_3DFACE)) << 8;
					TV[0].Blue = GetBValue(GetSysColor(COLOR_3DFACE)) << 8;
				}
				TV[0].Alpha = 0;
				TV[1].x = pEBDP->CRC.right;
				TV[1].y = pEBDP->CRC.bottom;
				if (IsWindowEnabled(hWnd))
				{
					TV[1].Red =  GetRValue(pEBDP->crBkColorTwo) << 8;
					TV[1].Green = GetGValue(pEBDP->crBkColorTwo) << 8;
					TV[1].Blue =  GetBValue(pEBDP->crBkColorTwo) << 8;
				}
				else
				{
					TV[1].Red = GetRValue(GetSysColor(COLOR_3DFACE)) << 8;
					TV[1].Green = GetGValue(GetSysColor(COLOR_3DFACE)) << 8;
					TV[1].Blue = GetBValue(GetSysColor(COLOR_3DFACE)) << 8;
				}
				TV[1].Alpha = 0;
				GR.UpperLeft = 0;
				GR.LowerRight = 1;
				GradientFill(pEBDP->hDBDC, &TV[0], 2, (PVOID)&GR, 1, GRADIENT_FILL_RECT_V);
				if (_tcslen(pEBDP->lpCurrentText))
				{
					DrawText(pEBDP->hDBDC, pEBDP->lpCurrentText, -1, &RCT, DT_SINGLELINE/* | DT_CALCRECT*/ | DT_END_ELLIPSIS);
					/*if (RCT.right >= (pEBDP->CRC.right - 2))
					{
						SIZE_T szTextSize = _tcslen(pEBDP->lpCurrentText);
						if (szTextSize > 3)
						{
							LPTSTR lpTmp = new TCHAR[szTextSize + 1];
							ZeroMemory(lpTmp, (szTextSize + 1) * sizeof(TCHAR));
							for (SIZE_T i = 1; i <= szTextSize; i++)
							{
								_tcsncpy(lpTmp, pEBDP->lpCurrentText, i);
								DrawText(pEBDP->hDBDC, lpTmp, -1, &RCT, DT_SINGLELINE | DT_CALCRECT);
								if (RCT.right >= (pEBDP->CRC.right - 2))
								{
									lpTmp[(i - 1) - 2] = '\0';
									_tcscat(lpTmp, TEXT("..."));
									break;
								}
							}
							_tcscpy(pEBDP->lpCurrentText, lpTmp);
							delete[] lpTmp;
						}
					}*/
					if (IsWindowEnabled(hWnd))
					{
						SetTextColor(pEBDP->hDBDC, pEBDP->crFontColorTwo);
					}
					else
					{
						SetTextColor(pEBDP->hDBDC, GetSysColor(COLOR_3DHIGHLIGHT));
					}
					OffsetRect(&pEBDP->CRC, 1, 1);
					DrawText(pEBDP->hDBDC, pEBDP->lpCurrentText, -1, &pEBDP->CRC, DT_VCENTER | DT_CENTER |
						DT_SINGLELINE | DT_NOPREFIX);
					if (IsWindowEnabled(hWnd))
					{
						SetTextColor(pEBDP->hDBDC, pEBDP->crFontColorOne);
					}
					else
					{
						SetTextColor(pEBDP->hDBDC, GetSysColor(COLOR_3DSHADOW));
					}
					OffsetRect(&pEBDP->CRC, -1, -1);
					DrawText(pEBDP->hDBDC, pEBDP->lpCurrentText, -1, &pEBDP->CRC, DT_VCENTER | DT_CENTER |
						DT_SINGLELINE | DT_NOPREFIX);
				}
				if (IsWindowEnabled(hWnd))
				{
					HBRUSH hBr1Brush, hBr2Brush;
					hBr1Brush = CreateSolidBrush(pEBDP->crBrColorOne);
					FrameRect(pEBDP->hDBDC, &pEBDP->CRC, hBr1Brush);
					hBr2Brush = CreateSolidBrush(pEBDP->crBrColorTwo);
					InflateRect(&pEBDP->CRC, -1, -1);
					FrameRect(pEBDP->hDBDC, &pEBDP->CRC, hBr2Brush);
					InflateRect(&pEBDP->CRC, 1, 1);
					DeleteObject(hBr1Brush);
					DeleteObject(hBr2Brush);
				}
				else
				{
					FrameRect(pEBDP->hDBDC, &pEBDP->CRC, GetSysColorBrush(COLOR_3DDKSHADOW));
					InflateRect(&pEBDP->CRC, -1, -1);
					FrameRect(pEBDP->hDBDC, &pEBDP->CRC, GetSysColorBrush(COLOR_3DHIGHLIGHT));
					InflateRect(&pEBDP->CRC, 1, 1);
				}
				//Эффект стекла
				//--------------------------------------------------------------------
				if (IsWindowEnabled(hWnd))
				{
					HDC hLDC;
					HBRUSH hGlBrush;
					HBITMAP hTmpBitmap, hOldBitmap;
					RECT RCGE = {};
					BLENDFUNCTION BF = {};
					CopyRect(&RCGE, &pEBDP->CRC);
					RCGE.right -= RCGE.left;
					RCGE.bottom -= RCGE.top;
					RCGE.bottom >>= 1;
					RCGE.left = 0;
					RCGE.top = 0;
					hLDC = CreateCompatibleDC(pEBDP->hDBDC);
					hTmpBitmap = CreateCompatibleBitmap(pEBDP->hDBDC, RCGE.right, RCGE.bottom);
					hOldBitmap = (HBITMAP)SelectObject(hLDC, hTmpBitmap);
					hGlBrush = CreateSolidBrush(RGB(255, 255, 255));
					FillRect(hLDC, &RCGE, hGlBrush);
					DeleteObject(hGlBrush);
					BF.SourceConstantAlpha = 80;
					BF.BlendOp = AC_SRC_OVER;
					AlphaBlend(pEBDP->hDBDC, pEBDP->CRC.left, pEBDP->CRC.top, RCGE.right, RCGE.bottom, hLDC, 0, 0, RCGE.right, RCGE.bottom, BF);
					SelectObject(hLDC, hOldBitmap);
					DeleteObject(hTmpBitmap);
					DeleteDC(hLDC);
				}
				//--------------------------------------------------------------------
				SelectObject(pEBDP->hDBDC, hOldFont);
			}
			RedrawWindow(hWnd, 0, 0, RDW_ERASE | RDW_INVALIDATE);
			break;
		}
		case WM_ERASEBKGND:
			return 1;
		case WM_PAINT:
		{
			PAINTSTRUCT PS = {};
			HDC hDC = BeginPaint(hWnd, &PS);
			BitBlt(hDC, 0, 0, pEBDP->CRC.right, pEBDP->CRC.bottom, pEBDP->hDBDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &PS);
			break;
		}
		case WM_DESTROY:
			SelectObject(pEBDP->hDBDC, pEBDP->hOldDBBitmap);
			DeleteObject(pEBDP->hDBBitmap);
			DeleteDC(pEBDP->hDBDC);
			ReleaseDC(hWnd, pEBDP->hDC);
			DeleteObject(pEBDP->hFont);
			delete pEBDP;
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}