#if (_WIN32_WINNT < 0x0500)
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>

#include "ebframe.h"

BOOL InitEBFrame(HINSTANCE hInstance)
{
	WNDCLASSEX WCEX = {};
	WCEX.cbSize = sizeof(WCEX);
	WCEX.style = CS_HREDRAW | CS_VREDRAW;
	WCEX.hInstance = hInstance;
	WCEX.hCursor = LoadCursor(0, (LPCTSTR)IDC_ARROW);
	WCEX.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WCEX.lpfnWndProc = EBFrameProc;
	WCEX.lpszClassName = EBFRAME_CLASS;
	if (!RegisterClassEx(&WCEX)) return FALSE;
	return TRUE;
}

LRESULT CALLBACK EBFrameProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPEBFPROPERTIES pEBFP = NULL;
	if (uMsg == WM_NCCREATE)
	{
		pEBFP = new EBFPROPERTIES;
		ZeroMemory(pEBFP, sizeof(EBFPROPERTIES));
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pEBFP);
	}
	else
	{
		pEBFP = (LPEBFPROPERTIES)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	switch (uMsg)
	{
		case WM_CREATE:
			//Инициализация
			//----------------------------------------------------------
			pEBFP->hDC = GetDC(hWnd);
			pEBFP->crBrColorOne = RGB(0, 0, 0);
			pEBFP->crBrColorTwo = RGB(255, 255, 255);
			pEBFP->lMode = EBF_MODE_NORMAL;
			break;
		case WM_SIZE:
		{
			DWORD dwCRgnOffset;
			HRGN hWRgn = 0, hCRgn = 0;
			GetClientRect(hWnd, &pEBFP->CRC);
			hWRgn = CreateRectRgn(pEBFP->CRC.left, pEBFP->CRC.top, pEBFP->CRC.right,
				pEBFP->CRC.bottom);
			if (pEBFP->lMode == EBF_MODE_SINGLELINE)
				dwCRgnOffset = 1; else dwCRgnOffset = 2;
			hCRgn = CreateRectRgn(pEBFP->CRC.left + dwCRgnOffset, pEBFP->CRC.top + dwCRgnOffset,
				pEBFP->CRC.right - dwCRgnOffset, pEBFP->CRC.bottom - dwCRgnOffset);
			CombineRgn(hWRgn, hWRgn, hCRgn, RGN_XOR);
			SetWindowRgn(hWnd, hWRgn, TRUE);
			DeleteObject(hWRgn);
			DeleteObject(hCRgn);
			if (pEBFP->hDBDC)
			{
				SelectObject(pEBFP->hDBDC, pEBFP->hOldDBBitmap);
				DeleteObject(pEBFP->hDBBitmap);
			}
			else
			{
				pEBFP->hDBDC = CreateCompatibleDC(pEBFP->hDC);
			}
			pEBFP->hDBBitmap = CreateCompatibleBitmap(pEBFP->hDC, pEBFP->CRC.right, pEBFP->CRC.bottom);
			pEBFP->hOldDBBitmap = (HBITMAP)SelectObject(pEBFP->hDBDC, pEBFP->hDBBitmap);
			PostMessage(hWnd, EBFM_DRAWCONTROL, 0, 0);
			break;
		}
		case EBFM_SETBRCOLORS:
			pEBFP->crBrColorOne = (COLORREF)wParam;
			pEBFP->crBrColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBFM_DRAWCONTROL, 0, 0);
			break;
		case EBFM_GETBRCOLORS:
			*(PLONG)wParam = pEBFP->crBrColorOne;
			*(PLONG)lParam = pEBFP->crBrColorTwo;
			break;
		case EBFM_SETMODE:
			pEBFP->lMode = (LONG)lParam;
			PostMessage(hWnd, WM_SIZE, 0, 0);
			break;
		case EBFM_GETMODE:
			return pEBFP->lMode;
		case WM_SYSCOLORCHANGE:
			PostMessage(hWnd, EBFM_DRAWCONTROL, 0, 0);
			break;
		case WM_XBUTTONUP:
			PostMessage(GetParent(hWnd), uMsg, wParam, lParam);
			break;
		case WM_ENABLE:
			PostMessage(hWnd, EBFM_DRAWCONTROL, 0, 0);
			break;
		case EBFM_DRAWCONTROL:
		{
			if (pEBFP->hDBDC)
			{
				if (IsWindowEnabled(hWnd))
				{
					HBRUSH hBr1Brush = CreateSolidBrush(pEBFP->crBrColorOne);
					FrameRect(pEBFP->hDBDC, &pEBFP->CRC, hBr1Brush);
					if (pEBFP->lMode == EBF_MODE_NORMAL)
					{
						HBRUSH hBr2Brush = CreateSolidBrush(pEBFP->crBrColorTwo);
						InflateRect(&pEBFP->CRC, -1, -1);
						FrameRect(pEBFP->hDBDC, &pEBFP->CRC, hBr2Brush);
						InflateRect(&pEBFP->CRC, 1, 1);
						DeleteObject(hBr2Brush);
					}
					DeleteObject(hBr1Brush);
				}
				else
				{
					FrameRect(pEBFP->hDBDC, &pEBFP->CRC, GetSysColorBrush(COLOR_3DDKSHADOW));
					if (pEBFP->lMode == EBF_MODE_NORMAL)
					{
						InflateRect(&pEBFP->CRC, -1, -1);
						FrameRect(pEBFP->hDBDC, &pEBFP->CRC, GetSysColorBrush(COLOR_3DHIGHLIGHT));
						InflateRect(&pEBFP->CRC, 1, 1);
					}
				}
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
			BitBlt(hDC, 0, 0, pEBFP->CRC.right, pEBFP->CRC.bottom, pEBFP->hDBDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &PS);
			break;
		}
		case WM_DESTROY:
			SelectObject(pEBFP->hDBDC, pEBFP->hOldDBBitmap);
			DeleteObject(pEBFP->hDBBitmap);
			DeleteDC(pEBFP->hDBDC);
			ReleaseDC(hWnd, pEBFP->hDC);
			delete pEBFP;
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
