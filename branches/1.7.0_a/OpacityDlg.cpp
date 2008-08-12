#include <windows.h>
#include <stdio.h>
#include <commctrl.h>

#include "resource.h"
#include "easybar.h"
#include "opacitydlg.h"

INT_PTR CALLBACK OpacityDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			SendDlgItemMessage(hWnd, IDC_SLDOL, TBM_SETRANGEMIN, TRUE, 1);
			SendDlgItemMessage(hWnd, IDC_SLDOL, TBM_SETRANGEMAX, TRUE, 100);
			SendDlgItemMessage(hWnd, IDC_SLDOL, TBM_SETPOS, TRUE, dwOpacityLevel);
			PostMessage(hWnd, WM_HSCROLL, MAKEWPARAM(TB_ENDTRACK, 0),
				(LPARAM)GetDlgItem(hWnd, IDC_SLDOL));
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BTNOK:
					dwOpacityLevel = SendDlgItemMessage(hWnd, IDC_SLDOL, TBM_GETPOS, 0, 0);
					EndDialog(hWnd, 0);
					break;
				case IDC_BTNCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			return TRUE;
		case WM_HSCROLL:
		{
			if ((HWND)lParam == GetDlgItem(hWnd, IDC_SLDOL))
			{
				switch (LOWORD(wParam))
				{
					case TB_PAGEUP:
					case TB_PAGEDOWN:	
					case TB_THUMBTRACK:
					case TB_THUMBPOSITION:
					case TB_ENDTRACK:
						LPWSTR lpwCL = new WCHAR[64];
						int intPos = SendDlgItemMessage(hWnd, IDC_SLDOL, TBM_GETPOS, 0, 0);
						swprintf(lpwCL, L"Custom level: %i%c", intPos, '%');
						SetDlgItemText(hWnd, IDC_STCCL, lpwCL);
						delete[] lpwCL;
						break;
				}
			}
			return TRUE;
		}
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}