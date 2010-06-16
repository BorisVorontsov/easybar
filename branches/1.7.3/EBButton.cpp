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

#include "ebbutton.h"

#pragma comment (lib, "msimg32.lib")

BOOL InitEBButton(HINSTANCE hInstance)
{
	WNDCLASSEX WCEX = {};
	WCEX.cbSize = sizeof(WCEX);
	WCEX.style = CS_HREDRAW | CS_VREDRAW;
	WCEX.hInstance = hInstance;
	WCEX.hCursor = LoadCursor(0, (LPCTSTR)IDC_ARROW);
	WCEX.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WCEX.lpfnWndProc = EBButtonProc;
	WCEX.lpszClassName = EBBUTTON_CLASS;
	if (!RegisterClassEx(&WCEX)) return FALSE;
	return TRUE;
}

LRESULT CALLBACK EBButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPEBBPROPERTIES pEBBP = NULL;
	if (uMsg == WM_NCCREATE)
	{
		pEBBP = new EBBPROPERTIES;
		ZeroMemory(pEBBP, sizeof(EBBPROPERTIES));
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pEBBP);
	}
	else
	{
		pEBBP = (LPEBBPROPERTIES)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	switch (uMsg)
	{
		case WM_CREATE:
			//Инициализация
			//----------------------------------------------------------
			pEBBP->hDC = GetDC(hWnd);
			pEBBP->hFont = CreateFont(-MulDiv(8, GetDeviceCaps(pEBBP->hDC, LOGPIXELSY), 72)
				, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Tahoma"));
			pEBBP->crFontColorOne = RGB(255, 255, 255);
			pEBBP->crFontColorTwo = RGB(128, 128, 128);
			pEBBP->crBkColorOne = RGB(255, 0, 0);
			pEBBP->crBkColorTwo = RGB(0, 0, 255);
			pEBBP->crBrColorOne = RGB(0, 0, 0);
			pEBBP->crBrColorTwo = RGB(255, 255, 255);
			pEBBP->crTrColor = RGB(255, 0, 255);
			ZeroMemory(pEBBP->lpCurrentText, sizeof(pEBBP->lpCurrentText));
			break;
		case WM_SIZE:
			GetClientRect(hWnd, &pEBBP->CRC);
			if (pEBBP->hDBDC)
			{
				SelectObject(pEBBP->hDBDC, pEBBP->hOldDBBitmap);
				DeleteObject(pEBBP->hDBBitmap);
			}
			else
			{
				pEBBP->hDBDC = CreateCompatibleDC(pEBBP->hDC);
			}
			pEBBP->hDBBitmap = CreateCompatibleBitmap(pEBBP->hDC, pEBBP->CRC.right, pEBBP->CRC.bottom);
			pEBBP->hOldDBBitmap = (HBITMAP)SelectObject(pEBBP->hDBDC, pEBBP->hDBBitmap);
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case EBBM_SETFONT:
			pEBBP->hFont = (HFONT)wParam;
			if (LOWORD(lParam) == TRUE)
				PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case EBBM_GETFONT:
			return (LRESULT)pEBBP->hFont;
		case EBBM_SETFONTCOLORS:
			pEBBP->crFontColorOne = (COLORREF)wParam;
			pEBBP->crFontColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case EBBM_GETFONTCOLORS:
			*(PLONG)wParam = pEBBP->crFontColorOne;
			*(PLONG)lParam = pEBBP->crFontColorTwo;
			break;
		case EBBM_SETBKCOLORS:
			pEBBP->crBkColorOne = (COLORREF)wParam;
			pEBBP->crBkColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case EBBM_GETBKCOLORS:
			*(PLONG)wParam = pEBBP->crBkColorOne;
			*(PLONG)lParam = pEBBP->crBkColorTwo;
			break;
		case EBBM_SETBRCOLORS:
			pEBBP->crBrColorOne = (COLORREF)wParam;
			pEBBP->crBrColorTwo = (COLORREF)lParam;
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case EBBM_GETBRCOLORS:
			*(PLONG)wParam = pEBBP->crBrColorOne;
			*(PLONG)lParam = pEBBP->crBrColorTwo;
			break;
		case EBBM_SETTRCOLOR:
			pEBBP->crTrColor = (COLORREF)lParam;
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case EBBM_GETTRCOLOR:
			return pEBBP->crTrColor;
		case EBBM_SETBITMAP:
			pEBBP->hBitmap = (HBITMAP)lParam;
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case EBBM_GETBITMAP:
			return (LRESULT)pEBBP->hBitmap;
		case WM_SETTEXT:
			_tcscpy(pEBBP->lpCurrentText, (PTCHAR)lParam);
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			return TRUE;
		case WM_GETTEXT:
			_tcsncpy((PTCHAR)lParam, pEBBP->lpCurrentText, wParam);
			return _tcslen((PTCHAR)lParam);
		case WM_SYSCOLORCHANGE:
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case WM_XBUTTONUP:
			PostMessage(GetParent(hWnd), uMsg, wParam, lParam);
			break;
		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			pEBBP->EBBMF.dwFlag = EBBMF_DOWN;
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case WM_LBUTTONUP:
		{
			if (pEBBP->EBBMF.dwFlag != EBBMF_DOWN) break;
			POINT PTMU = { LOWORD(lParam), HIWORD(lParam) };
			ReleaseCapture();
			pEBBP->EBBMF.dwFlag = 0;
			if (PtInRect(&pEBBP->CRC, PTMU))
			{
				PostMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), EBBN_CLICKED),
					(LPARAM)hWnd);
			}
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		}
		case WM_ENABLE:
			PostMessage(hWnd, EBBM_DRAWCONTROL, 0, 0);
			break;
		case EBBM_DRAWCONTROL:
		{
			if (pEBBP->hDBDC)
			{
				HDC hTDC, hEDC;
				HBITMAP hEBitmap;
				TRIVERTEX TV[2] = {};
				GRADIENT_RECT GR = {};
				RECT RCB = {};
				HFONT hOldFont;
				BITMAPINFO BMI = {}, EBMI = {};
				ULONG i, lR, lG, lB, lS;
				LPBYTE pPixels;
				LONG lX = 0, lY = 0, lXOffset = (pEBBP->EBBMF.dwFlag == EBBMF_DOWN)?1:0,
					lYOffset = (pEBBP->EBBMF.dwFlag == EBBMF_DOWN)?1:0;
				SetBkMode(pEBBP->hDBDC, TRANSPARENT);
				hOldFont = (HFONT)SelectObject(pEBBP->hDBDC, pEBBP->hFont);
				//Заливаем часть без градиента
				//--------------------------------------------------------------------
				CopyRect(&RCB, &pEBBP->CRC);
				RCB.bottom -= ((pEBBP->CRC.bottom / 3) * 2);
				if (IsWindowEnabled(hWnd))
				{
					HBRUSH hBkBrush = CreateSolidBrush(pEBBP->crBkColorOne);
					FillRect(pEBBP->hDBDC, &RCB, hBkBrush);
					DeleteObject(hBkBrush);
				}
				else
				{
					FillRect(pEBBP->hDBDC, &RCB, GetSysColorBrush(COLOR_3DFACE));
				}
				//--------------------------------------------------------------------
				TV[0].x = pEBBP->CRC.left;
				TV[0].y = pEBBP->CRC.top + (RCB.bottom - RCB.top);
				if (IsWindowEnabled(hWnd))
				{
					TV[0].Red = GetRValue(pEBBP->crBkColorOne) << 8;
					TV[0].Green = GetGValue(pEBBP->crBkColorOne) << 8;
					TV[0].Blue = GetBValue(pEBBP->crBkColorOne) << 8;
				}
				else
				{
					TV[0].Red = GetRValue(GetSysColor(COLOR_3DFACE)) << 8;
					TV[0].Green = GetGValue(GetSysColor(COLOR_3DFACE)) << 8;
					TV[0].Blue = GetBValue(GetSysColor(COLOR_3DFACE)) << 8;
				}
				TV[0].Alpha = 0;
				TV[1].x = pEBBP->CRC.right;
				TV[1].y = pEBBP->CRC.bottom;
				if (IsWindowEnabled(hWnd))
				{
					TV[1].Red =  GetRValue(pEBBP->crBkColorTwo) << 8;
					TV[1].Green = GetGValue(pEBBP->crBkColorTwo) << 8;
					TV[1].Blue =  GetBValue(pEBBP->crBkColorTwo) << 8;
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
				GradientFill(pEBBP->hDBDC, &TV[0], 2, (PVOID)&GR, 1, GRADIENT_FILL_RECT_V);
				if (pEBBP->hBitmap)
				{
					hTDC = CreateCompatibleDC(pEBBP->hDBDC);
					SelectObject(hTDC, pEBBP->hBitmap);
					GetObject(pEBBP->hBitmap, sizeof(BMI), &BMI);
					lX = pEBBP->CRC.right / 2 - BMI.bmiHeader.biWidth / 2;
					lY = pEBBP->CRC.bottom / 2 - BMI.bmiHeader.biHeight / 2;
					if (!IsWindowEnabled(hWnd))
					{
						//Спец. эффект - преобразование цвета изображения в серые тона
						hEDC = CreateCompatibleDC(pEBBP->hDBDC);
						hEBitmap = CreateCompatibleBitmap(pEBBP->hDBDC, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight);
						SelectObject(hEDC, hEBitmap);
						BitBlt(hEDC, 0, 0, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight, hTDC, 0, 0, SRCCOPY);
						EBMI.bmiHeader.biSize = sizeof(EBMI.bmiHeader);
						EBMI.bmiHeader.biWidth = BMI.bmiHeader.biWidth;
						EBMI.bmiHeader.biHeight = BMI.bmiHeader.biHeight;
						EBMI.bmiHeader.biPlanes = 1;
						EBMI.bmiHeader.biBitCount = 32;
						EBMI.bmiHeader.biCompression = BI_RGB;
						pPixels = new BYTE[EBMI.bmiHeader.biWidth * EBMI.bmiHeader.biHeight * 4];
						ZeroMemory(pPixels, EBMI.bmiHeader.biWidth * EBMI.bmiHeader.biHeight * 4);
						GetDIBits(hEDC, pEBBP->hBitmap, 0, EBMI.bmiHeader.biHeight, pPixels, &EBMI, DIB_RGB_COLORS);
						for (i = 0; i < (ULONG)(EBMI.bmiHeader.biWidth * EBMI.bmiHeader.biHeight); i++)
						{
							lR = pPixels[2];
							lG = pPixels[1];
							lB = pPixels[0];
							if (RGB(lR, lG, lB) != pEBBP->crTrColor)
							{
								lS = (lR * 77 + lG * 150 + lB * 28) / 255;
								pPixels[2] = (BYTE)lS;
								pPixels[1] = (BYTE)lS;
								pPixels[0] = (BYTE)lS;
							}
							pPixels += 4;
						}
						pPixels -= (EBMI.bmiHeader.biWidth * EBMI.bmiHeader.biHeight * 4);
						SetDIBitsToDevice(hEDC, 0, 0, EBMI.bmiHeader.biWidth, EBMI.bmiHeader.biHeight, 0, 
							0, 0, EBMI.bmiHeader.biHeight, pPixels, &EBMI, DIB_RGB_COLORS);
						TransparentBlt(pEBBP->hDBDC, lX, lY, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight,
							hEDC, 0, 0, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight, pEBBP->crTrColor);
						delete[] pPixels;
						DeleteDC(hEDC);
						DeleteObject(hEBitmap);
					}
					else
					{
						TransparentBlt(pEBBP->hDBDC, lX + lXOffset, lY + lYOffset, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight,
							hTDC, 0, 0, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight, pEBBP->crTrColor);
					}
					DeleteDC(hTDC);
				}
				if (_tcslen(pEBBP->lpCurrentText))
				{
					if (IsWindowEnabled(hWnd))
					{
						SetTextColor(pEBBP->hDBDC, pEBBP->crFontColorTwo);
					}
					else
					{
						SetTextColor(pEBBP->hDBDC, GetSysColor(COLOR_3DHIGHLIGHT));
					}
					OffsetRect(&pEBBP->CRC, 1 + lXOffset, 1 + lYOffset);
					DrawText(pEBBP->hDBDC, pEBBP->lpCurrentText, -1, &pEBBP->CRC, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
					if (IsWindowEnabled(hWnd))
					{
						SetTextColor(pEBBP->hDBDC, pEBBP->crFontColorOne);
					}
					else
					{
						SetTextColor(pEBBP->hDBDC, GetSysColor(COLOR_3DSHADOW));
					}
					OffsetRect(&pEBBP->CRC, -1, -1);
					DrawText(pEBBP->hDBDC, pEBBP->lpCurrentText, -1, &pEBBP->CRC, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
					OffsetRect(&pEBBP->CRC, -lXOffset, -lYOffset);
				}
				if (IsWindowEnabled(hWnd))
				{
					HBRUSH hBr1Brush, hBr2Brush;
					hBr1Brush = CreateSolidBrush(pEBBP->crBrColorOne);
					FrameRect(pEBBP->hDBDC, &pEBBP->CRC, hBr1Brush);
					hBr2Brush = CreateSolidBrush(pEBBP->crBrColorTwo);
					InflateRect(&pEBBP->CRC, -1, -1);
					FrameRect(pEBBP->hDBDC, &pEBBP->CRC, hBr2Brush);
					InflateRect(&pEBBP->CRC, 1, 1);
					DeleteObject(hBr1Brush);
					DeleteObject(hBr2Brush);
				}
				else
				{
					FrameRect(pEBBP->hDBDC, &pEBBP->CRC, GetSysColorBrush(COLOR_3DDKSHADOW));
					InflateRect(&pEBBP->CRC, -1, -1);
					FrameRect(pEBBP->hDBDC, &pEBBP->CRC, GetSysColorBrush(COLOR_3DHIGHLIGHT));
					InflateRect(&pEBBP->CRC, 1, 1);
				}
				//--------------------------------------------------------------------
				if (IsWindowEnabled(hWnd))
				{
					HDC hLDC;
					HBRUSH hGlBrush;
					HBITMAP hTmpBitmap, hOldBitmap;
					RECT RCGE = {};
					BLENDFUNCTION BF = {};
					CopyRect(&RCGE, &pEBBP->CRC);
					RCGE.right -= RCGE.left;
					RCGE.bottom -= RCGE.top;
					RCGE.bottom >>= 1;
					RCGE.left = 0;
					RCGE.top = 0;
					hLDC = CreateCompatibleDC(pEBBP->hDBDC);
					hTmpBitmap = CreateCompatibleBitmap(pEBBP->hDBDC, RCGE.right, RCGE.bottom);
					hOldBitmap = (HBITMAP)SelectObject(hLDC, hTmpBitmap);
					hGlBrush = CreateSolidBrush(RGB(255, 255, 255));
					FillRect(hLDC, &RCGE, hGlBrush);
					DeleteObject(hGlBrush);
					BF.SourceConstantAlpha = 80;
					BF.BlendOp = AC_SRC_OVER;
					AlphaBlend(pEBBP->hDBDC, pEBBP->CRC.left, pEBBP->CRC.top, RCGE.right, RCGE.bottom, hLDC, 0, 0, RCGE.right, RCGE.bottom, BF);
					SelectObject(hLDC, hOldBitmap);
					DeleteObject(hTmpBitmap);
					DeleteDC(hLDC);
				}
				//--------------------------------------------------------------------
				SelectObject(pEBBP->hDBDC, hOldFont);
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
			BitBlt(hDC, 0, 0, pEBBP->CRC.right, pEBBP->CRC.bottom, pEBBP->hDBDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &PS);
			break;
		}
		case WM_DESTROY:
		{
			SelectObject(pEBBP->hDBDC, pEBBP->hOldDBBitmap);
			DeleteObject(pEBBP->hDBBitmap);
			DeleteDC(pEBBP->hDBDC);
			ReleaseDC(hWnd, pEBBP->hDC);
			DeleteObject(pEBBP->hFont);
			DeleteObject(pEBBP->hBitmap);
			delete pEBBP;
			break;
		}
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
