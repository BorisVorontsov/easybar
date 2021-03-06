#if (_WIN32_WINNT < 0x0500)
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <math.h>

#include "resource.h"
#include "playervwdlg.h"
#include "common.h"
#include "engine.h"
#include "easybar.h"
#include "videodlg.h"

extern VWDATA VWD;

static POINT PTMM = { 0 };

static LONG lOldWndStyle;
static LONG lOldWndExStyle;

INT_PTR CALLBACK VideoDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//������������� �������
			//--------------------------------------------------------------------
			RECT RC = { 0 };
			ScaleVideoWindow(hWnd, 1, &RC);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, (RC.right - RC.left),
				(RC.bottom - RC.top), SWP_NOACTIVATE);
			MoveToCenter(hWnd, 0, 0);
			VWD.dwVWPosFlag = VWPF_NORMAL;
			VWD.dwSMPTimeout = 0;
			SetTimer(hWnd, 1, 200, 0);
			/*
			//-------------------------------------------------------------------------------
			VWD.hPlayerVW = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PLAYER_VW),
				hWnd, PlayerDlgProc, 0);
			SetWindowPos(VWD.hPlayerVW, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
			ShowWindow(VWD.hPlayerVW, SW_SHOW);
			//-------------------------------------------------------------------------------
			*/
			return TRUE;
		}
		case WM_ERASEBKGND:
		{
			RECT RCW = { 0 };
			HDC hDC = (HDC)wParam;
			GetClientRect(hWnd, &RCW);
			FillRect(hDC, &RCW, (HBRUSH)GetStockObject(BLACK_BRUSH));
			return TRUE;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if (VWD.dwVWPosFlag == VWPF_FULLSCREEN)
			{
				ShowMousePointer(TRUE);
				VWD.dwSMPTimeout = GetTickCount();
			}
			return TRUE;
		case WM_MOUSEMOVE:
			//���������� ���������������� � ����������� �������
			if ((abs(LOWORD(lParam) - PTMM.x) >= 10) || (abs(HIWORD(lParam) - PTMM.y) >= 10))
			{
				PTMM.x = LOWORD(lParam);
				PTMM.y = HIWORD(lParam);
				if (VWD.dwVWPosFlag == VWPF_FULLSCREEN)
				{
					ShowMousePointer(TRUE);
					VWD.dwSMPTimeout = GetTickCount();
				}
			}
			return TRUE;
		case WM_LBUTTONUP:
			PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDC_EBBPP, 0), 0);
			return TRUE;
		case WM_MBUTTONUP:
			PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FULLSCREEN_FULLSCREENNORMAL, 0), 0);
			return TRUE;
		case WM_RBUTTONUP:
		{
			if (VWD.dwVWPosFlag == VWPF_NORMAL)
			{
				LONG lCurWndStyle = GetWindowLong(hWnd, GWL_STYLE);
				if ((lCurWndStyle & WS_CAPTION) == WS_CAPTION)
				{
					lCurWndStyle ^= WS_CAPTION;
				}
				else
				{
					lCurWndStyle |= WS_CAPTION;
				}
				SetWindowLong(hWnd, GWL_STYLE, lCurWndStyle);
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED |
					SWP_NOSIZE | SWP_NOMOVE);
			}
			else
			{
				MessageBeep(0);
			}
			return TRUE;
		}
		case WM_WINDOWPOSCHANGED:
		{
			RECT RCW = { 0 }, RCS = { 0 }, RCV = { 0 };
			GetClientRect(hWnd, &RCW);
			GetClientRect(GetDlgItem(hWnd, IDC_STCLOGO), &RCS);
			RedrawWindow(hWnd, 0, 0, RDW_ERASE | RDW_INVALIDATE);
			MoveWindow(GetDlgItem(hWnd, IDC_STCLOGO), RCW.right / 2 - RCS.right / 2,
				RCW.bottom / 2 - RCS.bottom / 2, RCS.right, RCS.bottom, TRUE);
			if (dwKeepAspectRatio)
			{
				double dblAspectRatio;
				SIZE SZ = { 0 };
				switch (dwAspectRatioIndex)
				{
					case 0:
						pEngine->GetOriginalVideoSize(&SZ);
						break;
					case 1:
						SZ.cx = 4;
						SZ.cy = 3;
						break;
					case 2:
						SZ.cx = 5;
						SZ.cy = 4;
						break;
					case 3:
						SZ.cx = 16;
						SZ.cy = 9;
						break;
				}
				if (SZ.cx >= SZ.cy)
				{
					dblAspectRatio = (double)SZ.cx / (double)SZ.cy;
					if ((RCW.right / dblAspectRatio) < RCW.bottom)
					{
						RCV.right = RCW.right;
						RCV.bottom = (LONG)(RCW.right / dblAspectRatio);
						RCV.top = RCW.bottom / 2 - RCV.bottom / 2;
					}
					else
					{
						RCV.bottom = RCW.bottom;
						RCV.right = (LONG)(RCW.bottom * dblAspectRatio);
						RCV.left = RCW.right / 2 - RCV.right / 2;
					}
				}
				else
				{
					dblAspectRatio = (double)SZ.cy / (double)SZ.cx;
					if ((RCW.bottom / dblAspectRatio) < RCW.right)
					{
						RCV.bottom = RCW.bottom;
						RCV.right = (LONG)(RCW.bottom / dblAspectRatio);
						RCV.left = RCW.right / 2 - RCV.right / 2;
					}
					else
					{
						RCV.right = RCW.right;
						RCV.bottom = (LONG)(RCW.right * dblAspectRatio);
						RCV.top = RCW.bottom / 2 - RCV.bottom / 2;
					}
				}
			}
			else
			{
				CopyRect(&RCV, &RCW);
			}
			pEngine->SetVideoSize(RCV);
			return TRUE;
		}
		case VWM_SAVENORMALSTATE:
			GetWindowRect(hWnd, &VWD.rcNormalPos);
			return TRUE;
		case VWM_CHANGESTATE:
		{
			LONG lCurWndStyle = GetWindowLong(hWnd, GWL_STYLE);
			LONG lCurWndExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
			switch (wParam)
			{
				case VWS_FULLSCREEN:
				{
					int intSX = GetSystemMetrics(SM_CXSCREEN);
					int intSY = GetSystemMetrics(SM_CYSCREEN);
					lOldWndStyle = lCurWndStyle;
					lOldWndExStyle = lCurWndExStyle;
					if ((lCurWndStyle & WS_CAPTION) == WS_CAPTION) lCurWndStyle ^= WS_CAPTION;
					if ((lCurWndStyle & WS_OVERLAPPED) == WS_OVERLAPPED) lCurWndStyle ^= WS_OVERLAPPED;
					if ((lCurWndStyle & WS_THICKFRAME) == WS_THICKFRAME) lCurWndStyle ^= WS_THICKFRAME;
					if ((lCurWndExStyle & WS_EX_WINDOWEDGE) == WS_EX_WINDOWEDGE) lCurWndExStyle ^= WS_EX_WINDOWEDGE;
					SetWindowLong(hWnd, GWL_STYLE, lCurWndStyle);
					SetWindowLong(hWnd, GWL_EXSTYLE, lCurWndExStyle);
					if (pEngine->GetState() == E_STATE_STOPPED)
					{
						ShowWindow(GetDlgItem(hWnd, IDC_STCLOGO), SW_HIDE);
					}
					MoveWindow(hWnd, 0, 0, intSX, intSY, TRUE);
					if (pEngine->GetState() == E_STATE_STOPPED)
					{
						ShowWindow(GetDlgItem(hWnd, IDC_STCLOGO), SW_SHOW);
					}
					VWD.dwSMPTimeout = GetTickCount();
					VWD.dwVWPosFlag = VWPF_FULLSCREEN;
					break;
				}
				case VWS_NORMAL:
					SetWindowLong(hWnd, GWL_STYLE, lOldWndStyle);
					SetWindowLong(hWnd, GWL_EXSTYLE, lOldWndExStyle);
					if (pEngine->GetState() == E_STATE_STOPPED)
					{
						ShowWindow(GetDlgItem(hWnd, IDC_STCLOGO), SW_HIDE);
					}
					MoveWindow(hWnd, VWD.rcNormalPos.left, VWD.rcNormalPos.top, VWD.rcNormalPos.right - VWD.rcNormalPos.left,
						VWD.rcNormalPos.bottom - VWD.rcNormalPos.top, TRUE);
					if (pEngine->GetState() == E_STATE_STOPPED)
					{
						ShowWindow(GetDlgItem(hWnd, IDC_STCLOGO), SW_SHOW);
					}
					ShowMousePointer(TRUE);
					VWD.dwVWPosFlag = VWPF_NORMAL;
					break;
			}
			return TRUE;
		}
		case WM_MOVING:
			StickyWindow(hWnd, (LPRECT)lParam);
			return TRUE;
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO pMMI = (LPMINMAXINFO)lParam;
			pMMI->ptMinTrackSize.x = VW_MIN_WIDTH;
			pMMI->ptMinTrackSize.y = VW_MIN_HEIGHT;
			return TRUE;
		}
		case WM_DROPFILES:
			PostMessage(hMainWnd, uMsg, wParam, lParam);
			return TRUE;
		case WM_TIMER:
			if (wParam == 1)
			{
				if (pEngine->GetState() == E_STATE_STOPPED)
				{
					if (pEngine->GetVideoVisible())
					{
						pEngine->SetVideoVisible(FALSE);
					}
					if (!IsWindowVisible(GetDlgItem(hWnd, IDC_STCLOGO)))
					{
						ShowWindow(GetDlgItem(hWnd, IDC_STCLOGO), SW_SHOW);
					}
				}
				else
				{
					if (IsWindowVisible(GetDlgItem(hWnd, IDC_STCLOGO)))
					{
						ShowWindow(GetDlgItem(hWnd, IDC_STCLOGO), SW_HIDE);
					}
					if (!pEngine->GetVideoVisible())
					{
						pEngine->SetVideoVisible(TRUE);
					}
				}
				if (VWD.dwSMPTimeout && (VWD.dwVWPosFlag == VWPF_FULLSCREEN))
				{
					if ((GetTickCount() - VWD.dwSMPTimeout) >= 2000)
					{
						ShowMousePointer(FALSE);
						VWD.dwSMPTimeout = 0;
					}
				}
			}
			return TRUE;
		case WM_CLOSE:
			if (VWD.dwVWPosFlag == VWPF_FULLSCREEN) ShowMousePointer(TRUE);
			KillTimer(hWnd, 1);
			/*
			DestroyWindow(VWD.hPlayerVW);
			*/
			DestroyWindow(hWnd);
			return TRUE;
	}
	return FALSE;
}

//�������� dwZoomIndex: 0 - (50%), 1 - (100%), 2 - (200%)
//���� pRC �� ����� ����, ������� ������ ����� �������
//� ����, ��� ���� �� ������� �������� ����
void ScaleVideoWindow(HWND hWnd, DWORD dwZoomIndex, LPRECT pVWRC)
{
	double dblAspectRatio;
	SIZE SZ = { 0 }, SZA = { 0 };
	RECT RC = { 0 };
	pEngine->GetOriginalVideoSize(&SZ);
	GetWindowRect(hWnd, &RC);
	if (dwKeepAspectRatio)
	{
		switch (dwAspectRatioIndex)
		{
			case 0:
				SZA.cx = SZ.cx;
				SZA.cy = SZ.cy;
				break;
			case 1:
				SZA.cx = 4;
				SZA.cy = 3;
				break;
			case 2:
				SZA.cx = 5;
				SZA.cy = 4;
				break;
			case 3:
				SZA.cx = 16;
				SZA.cy = 9;
				break;
		}
		if (SZ.cx >= SZ.cy)
		{
			dblAspectRatio = (double)SZA.cx / (double)SZA.cy;
			SZ.cy = (LONG)(SZ.cx / dblAspectRatio);
		}
		else
		{
			dblAspectRatio = (double)SZA.cy / (double)SZA.cx;
			SZ.cx = (LONG)(SZ.cy / dblAspectRatio);
		}
	}
	switch (dwZoomIndex)
	{
		case 0:
			SZ.cx /= 2;
			SZ.cy /= 2;
			break;
		case 1:
			//
			break;
		case 2:
			SZ.cx *= 2;
			SZ.cy *= 2;
			break;
	}
	//���� ������ ������� � ��-���������� ����� ����...
	SZ.cx += (GetSystemMetrics(SM_CXSIZEFRAME) * 2);
	SZ.cy += (GetSystemMetrics(SM_CYSIZEFRAME) * 2);
	if (pVWRC)
	{
		pVWRC->left = RC.left;
		pVWRC->top = RC.top;
		pVWRC->right = RC.left + SZ.cx;
		pVWRC->bottom = RC.top + SZ.cy;
	}
	else
	{
		SetWindowPos(hWnd, HWND_TOP, 0, 0, SZ.cx, SZ.cy, SWP_NOMOVE | SWP_NOACTIVATE);
	}
}