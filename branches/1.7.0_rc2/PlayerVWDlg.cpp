#include <windows.h>

#include "playervwdlg.h"

INT_PTR CALLBACK PlayerVWDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				//
			}
			return TRUE;

	}
	return FALSE;
}