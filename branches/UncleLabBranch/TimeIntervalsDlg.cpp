#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "easybar.h"
#include "timeintervalsdlg.h"

INT_PTR CALLBACK TimeIntervalsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			SendDlgItemMessage(hWnd, IDC_SPNSTI, UDM_SETRANGE, 0, UD_MAXVAL);
			SendDlgItemMessage(hWnd, IDC_SPNSTI, UDM_SETPOS, 0, MAKELPARAM((WORD)dwSeekValue1, 0));
			SendDlgItemMessage(hWnd, IDC_SPNJTI, UDM_SETRANGE, 0, UD_MAXVAL);
			SendDlgItemMessage(hWnd, IDC_SPNJTI, UDM_SETPOS, 0, MAKELPARAM((WORD)dwSeekValue2, 0));
			SendDlgItemMessage(hWnd, IDC_SPNLJTI, UDM_SETRANGE, 0, UD_MAXVAL);
			SendDlgItemMessage(hWnd, IDC_SPNLJTI, UDM_SETPOS, 0, MAKELPARAM((WORD)dwSeekValue3, 0));
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BTNDEFAULT:
					SendDlgItemMessage(hWnd, IDC_SPNSTI, UDM_SETPOS, 0, MAKELPARAM(EB_SEEK_VALUE_1, 0));
					SendDlgItemMessage(hWnd, IDC_SPNJTI, UDM_SETPOS, 0, MAKELPARAM(EB_SEEK_VALUE_2, 0));
					SendDlgItemMessage(hWnd, IDC_SPNLJTI, UDM_SETPOS, 0, MAKELPARAM(EB_SEEK_VALUE_3, 0));
					break;
				case IDC_BTNOK:
					dwSeekValue1 = (DWORD)LOWORD(SendDlgItemMessage(hWnd, IDC_SPNSTI, UDM_GETPOS, 0, 0));
					dwSeekValue2 = (DWORD)LOWORD(SendDlgItemMessage(hWnd, IDC_SPNJTI, UDM_GETPOS, 0, 0));
					dwSeekValue3 = (DWORD)LOWORD(SendDlgItemMessage(hWnd, IDC_SPNLJTI, UDM_GETPOS, 0, 0));
					EndDialog(hWnd, 0);
					break;
				case IDC_BTNCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			return TRUE;
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}