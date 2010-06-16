#if (_WIN32_WINNT < 0x0500)
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <math.h>

#ifdef UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include <tchar.h>

#include "ebslider.h"

#pragma comment (lib, "msimg32.lib")

BOOL InitEBSlider(HINSTANCE hInstance)
{
	WNDCLASSEX WCEX = {};
	WCEX.cbSize = sizeof(WCEX);
	WCEX.style = CS_HREDRAW | CS_VREDRAW;
	WCEX.hInstance = hInstance;
	WCEX.hCursor = LoadCursor(0, (LPCTSTR)IDC_ARROW);
	WCEX.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WCEX.lpfnWndProc = EBSliderProc;
	WCEX.lpszClassName = EBSLIDER_CLASS;
	if (!RegisterClassEx(&WCEX)) return FALSE;
	return TRUE;
}

LRESULT CALLBACK EBSliderProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPEBSPROPERTIES pEBSP = NULL;
	if (uMsg == WM_NCCREATE)
	{
		pEBSP = new EBSPROPERTIES;
		ZeroMemory(pEBSP, sizeof(EBSPROPERTIES));
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pEBSP);
	}
	else
	{
		pEBSP = (LPEBSPROPERTIES)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	switch (uMsg)
	{
		case WM_CREATE:
			//Инициализация
			//----------------------------------------------------------
			pEBSP->hDC = GetDC(hWnd);
			pEBSP->lMin = 0;
			pEBSP->lMax = 100;
			pEBSP->lPos = 0;
			pEBSP->crLineColorOne = RGB(255, 0, 0);
			pEBSP->crLineColorTwo = RGB(0, 0, 255);
			pEBSP->crBkColor = RGB(255, 255, 255);
			pEBSP->crBrColorOne = RGB(0, 0, 0);
			pEBSP->crBrColorTwo = RGB(255, 255, 255);
			pEBSP->lMode = EBS_MODE_HORIZONTAL;
			break;
		case WM_SIZE:
			GetClientRect(hWnd, &pEBSP->CRC);
			if (pEBSP->hDBDC)
			{
				SelectObject(pEBSP->hDBDC, pEBSP->hOldDBBitmap);
				DeleteObject(pEBSP->hDBBitmap);
			}
			else
			{
				pEBSP->hDBDC = CreateCompatibleDC(pEBSP->hDC);
			}
			pEBSP->hDBBitmap = CreateCompatibleBitmap(pEBSP->hDC, pEBSP->CRC.right, pEBSP->CRC.bottom);
			pEBSP->hOldDBBitmap = (HBITMAP)SelectObject(pEBSP->hDBDC, pEBSP->hDBBitmap);
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		case EBSM_SETRANGE:
			pEBSP->lMin = (LONG)wParam;
			pEBSP->lMax = (LONG)lParam;
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		case EBSM_GETRANGE:
			*(PLONG)wParam = pEBSP->lMin;
			*(PLONG)lParam = pEBSP->lMax;
			break;
		case EBSM_SETPOS:
		{
			pEBSP->lPos = (LONG)lParam;
			if (wParam)
			{
				NMHDR NM = {};
				NM.code = EBSN_CHANGE_SP;
				NM.hwndFrom = hWnd;
				NM.idFrom = GetDlgCtrlID(hWnd);
				SendMessage(GetParent(hWnd), WM_NOTIFY, NM.idFrom, (LPARAM)&NM);
			}
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		}
		case EBSM_GETPOS:
			return pEBSP->lPos;
		case EBSM_SETLINECOLORS:
			pEBSP->crLineColorOne = (COLORREF)wParam;
			pEBSP->crLineColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		case EBSM_GETLINECOLORS:
			*(PLONG)wParam = pEBSP->crLineColorOne;
			*(PLONG)lParam = pEBSP->crLineColorTwo;
			break;
		case EBSM_SETBKCOLOR:
			pEBSP->crBkColor = (COLORREF)lParam;
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		case EBSM_GETBKCOLOR:
			return pEBSP->crBkColor;
		case EBSM_SETBRCOLORS:
			pEBSP->crBrColorOne = (COLORREF)wParam;
			pEBSP->crBrColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		case EBSM_GETBRCOLORS:
			*(PLONG)wParam = pEBSP->crBrColorOne;
			*(PLONG)lParam = pEBSP->crBrColorTwo;
			break;
		case EBSM_SETMODE:
			pEBSP->lMode = (LONG)lParam;
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		case EBSM_GETMODE:
			return pEBSP->lMode;
		case WM_SYSCOLORCHANGE:
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		case WM_XBUTTONUP:
			PostMessage(GetParent(hWnd), uMsg, wParam, lParam);
			break;
		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
		case WM_MOUSEMOVE:
		{
			if ((wParam & MK_LBUTTON) == MK_LBUTTON)
			{
				POINTS pt = MAKEPOINTS(lParam);
				if (pEBSP->lMode == EBS_MODE_HORIZONTAL)
				{
					if ((pt.x >= 0) && (pt.x <= pEBSP->CRC.right))
					{
						pEBSP->lPos = (LONG)(((double)pt.x / (double)pEBSP->CRC.right)
							* (pEBSP->lMax - pEBSP->lMin)) + pEBSP->lMin;
					}
				}
				else
				{
					if ((pt.y >= 0) && (pt.y <= pEBSP->CRC.bottom))
					{
						pEBSP->lPos = (pEBSP->lMax - pEBSP->lMin) - (LONG)(((double)pt.y /
							(double)pEBSP->CRC.bottom) * (pEBSP->lMax - pEBSP->lMin)) +
							pEBSP->lMin;
					}
				}
				ZeroMemory(&pEBSP->NM, sizeof(pEBSP->NM));
				pEBSP->NM.code = EBSN_SCROLL;
				pEBSP->NM.hwndFrom = hWnd;
				pEBSP->NM.idFrom = GetDlgCtrlID(hWnd);
				PostMessage(GetParent(hWnd), WM_NOTIFY, pEBSP->NM.idFrom, (LPARAM)&pEBSP->NM);
				PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			ZeroMemory(&pEBSP->NM, sizeof(pEBSP->NM));
			pEBSP->NM.code = EBSN_CHANGE;
			pEBSP->NM.hwndFrom = hWnd;
			pEBSP->NM.idFrom = GetDlgCtrlID(hWnd);
			PostMessage(GetParent(hWnd), WM_NOTIFY, pEBSP->NM.idFrom, (LPARAM)&pEBSP->NM);
			break;
		}
		case WM_ENABLE:
			PostMessage(hWnd, EBSM_DRAWCONTROL, 0, 0);
			break;
		case EBSM_DRAWCONTROL:
		{
			if (pEBSP->hDBDC)
			{
				HPEN hIPen, hOldPen;
				LONG lPos;
				TRIVERTEX TV[2] = {};
				GRADIENT_RECT GR = {};
				RECT RCB = {};
				if (IsWindowEnabled(hWnd))
				{
					HBRUSH hBkBrush = CreateSolidBrush(pEBSP->crBkColor);
					FillRect(pEBSP->hDBDC, &pEBSP->CRC, hBkBrush);
					DeleteObject(hBkBrush);
				}
				else
				{
					FillRect(pEBSP->hDBDC, &pEBSP->CRC, GetSysColorBrush(COLOR_3DFACE));
				}
				if (pEBSP->lMode == EBS_MODE_VERTICAL)
				{
					lPos = pEBSP->CRC.bottom - (LONG)(((double)pEBSP->CRC.bottom /
						(double)(pEBSP->lMax - pEBSP->lMin)) * (pEBSP->lPos - pEBSP->lMin));
				}
				else
				{
					lPos = (LONG)(((double)pEBSP->CRC.right / (double)(pEBSP->lMax -
						pEBSP->lMin)) * (pEBSP->lPos - pEBSP->lMin));
				}
				//Заливаем часть без градиента
				//--------------------------------------------------------------------
				CopyRect(&RCB, &pEBSP->CRC);
				if (pEBSP->lMode == EBS_MODE_VERTICAL)
				{
					RCB.right -= ((pEBSP->CRC.right / 3) * 2);
					RCB.top = lPos;
				}
				else
				{
					RCB.bottom -= ((pEBSP->CRC.bottom / 3) * 2);
					RCB.right = lPos;
				}
				if (IsWindowEnabled(hWnd))
				{
					HBRUSH hLnBrush = CreateSolidBrush(pEBSP->crLineColorOne);
					FillRect(pEBSP->hDBDC, &RCB, hLnBrush);
					DeleteObject(hLnBrush);
				}
				else
				{
					FillRect(pEBSP->hDBDC, &RCB, GetSysColorBrush(COLOR_3DFACE));
				}
				//--------------------------------------------------------------------
				if (pEBSP->lMode == EBS_MODE_VERTICAL)
				{
					TV[0].x = pEBSP->CRC.left + (RCB.right - RCB.left);
					TV[0].y = lPos;
				}
				else
				{
					TV[0].x = pEBSP->CRC.left;
					TV[0].y = pEBSP->CRC.top + (RCB.bottom - RCB.top);
				}
				if (IsWindowEnabled(hWnd))
				{
					TV[0].Red = GetRValue(pEBSP->crLineColorOne) << 8;
					TV[0].Green = GetGValue(pEBSP->crLineColorOne) << 8;
					TV[0].Blue = GetBValue(pEBSP->crLineColorOne) << 8;
				}
				else
				{
					TV[0].Red = GetRValue(GetSysColor(COLOR_3DFACE)) << 8;
					TV[0].Green = GetGValue(GetSysColor(COLOR_3DFACE)) << 8;
					TV[0].Blue = GetBValue(GetSysColor(COLOR_3DFACE)) << 8;
				}
				TV[0].Alpha = 0;
				if (pEBSP->lMode == EBS_MODE_HORIZONTAL)
				{
					TV[1].x = lPos;
				}
				else TV[1].x = pEBSP->CRC.right;
				TV[1].y = pEBSP->CRC.bottom;
				if (IsWindowEnabled(hWnd))
				{
					TV[1].Red =  GetRValue(pEBSP->crLineColorTwo) << 8;
					TV[1].Green = GetGValue(pEBSP->crLineColorTwo) << 8;
					TV[1].Blue =  GetBValue(pEBSP->crLineColorTwo) << 8;
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
				GradientFill(pEBSP->hDBDC, &TV[0], 2, (PVOID)&GR, 1, (pEBSP->lMode == EBS_MODE_HORIZONTAL)?
					GRADIENT_FILL_RECT_V:GRADIENT_FILL_RECT_H);
				if (IsWindowEnabled(hWnd))
				{
					hIPen = CreatePen(PS_SOLID, 1, pEBSP->crLineColorTwo);
				}
				else
				{
					hIPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
				}
				hOldPen = (HPEN)SelectObject(pEBSP->hDBDC, hIPen);
				if (pEBSP->lMode == EBS_MODE_VERTICAL)
				{
					MoveToEx(pEBSP->hDBDC, pEBSP->CRC.right, lPos, NULL);
					LineTo(pEBSP->hDBDC, pEBSP->CRC.left, lPos);
				}
				else if (pEBSP->lMode == EBS_MODE_HORIZONTAL)
				{
					MoveToEx(pEBSP->hDBDC, lPos - 1, pEBSP->CRC.top, NULL);
					LineTo(pEBSP->hDBDC, lPos - 1, pEBSP->CRC.bottom);
				}
				DeleteObject(SelectObject(pEBSP->hDBDC, hOldPen));
				if (IsWindowEnabled(hWnd))
				{
					HBRUSH hBr1Brush, hBr2Brush;
					hBr1Brush = CreateSolidBrush(pEBSP->crBrColorOne);
					FrameRect(pEBSP->hDBDC, &pEBSP->CRC, hBr1Brush);
					hBr2Brush = CreateSolidBrush(pEBSP->crBrColorTwo);
					InflateRect(&pEBSP->CRC, -1, -1);
					FrameRect(pEBSP->hDBDC, &pEBSP->CRC, hBr2Brush);
					InflateRect(&pEBSP->CRC, 1, 1);
					DeleteObject(hBr1Brush);
					DeleteObject(hBr2Brush);
				}
				else
				{
					FrameRect(pEBSP->hDBDC, &pEBSP->CRC, GetSysColorBrush(COLOR_3DDKSHADOW));
					InflateRect(&pEBSP->CRC, -1, -1);
					FrameRect(pEBSP->hDBDC, &pEBSP->CRC, GetSysColorBrush(COLOR_3DHIGHLIGHT));
					InflateRect(&pEBSP->CRC, 1, 1);
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
					CopyRect(&RCGE, &pEBSP->CRC);
					RCGE.right -= RCGE.left;
					RCGE.bottom -= RCGE.top;
					if (pEBSP->lMode == EBS_MODE_VERTICAL)
						RCGE.right >>= 1;
					else
						RCGE.bottom >>= 1;
					RCGE.left = 0;
					RCGE.top = 0;
					hLDC = CreateCompatibleDC(pEBSP->hDBDC);
					hTmpBitmap = CreateCompatibleBitmap(pEBSP->hDBDC, RCGE.right, RCGE.bottom);
					hOldBitmap = (HBITMAP)SelectObject(hLDC, hTmpBitmap);
					hGlBrush = CreateSolidBrush(RGB(255, 255, 255));
					FillRect(hLDC, &RCGE, hGlBrush);
					DeleteObject(hGlBrush);
					BF.SourceConstantAlpha = 80;
					BF.BlendOp = AC_SRC_OVER;
					AlphaBlend(pEBSP->hDBDC, pEBSP->CRC.left, pEBSP->CRC.top, RCGE.right, RCGE.bottom, hLDC, 0, 0, RCGE.right, RCGE.bottom, BF);
					SelectObject(hLDC, hOldBitmap);
					DeleteObject(hTmpBitmap);
					DeleteDC(hLDC);
				}
				//--------------------------------------------------------------------
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
			BitBlt(hDC, 0, 0, pEBSP->CRC.right, pEBSP->CRC.bottom, pEBSP->hDBDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &PS);
			break;
		}
		case WM_DESTROY:
			SelectObject(pEBSP->hDBDC, pEBSP->hOldDBBitmap);
			DeleteObject(pEBSP->hDBBitmap);
			DeleteDC(pEBSP->hDBDC);
			ReleaseDC(hWnd, pEBSP->hDC);
			delete pEBSP;
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
