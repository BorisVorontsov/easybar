#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "videomode.h"
#include "easybar.h"
#include "fsvideomodedlg.h"

LPVIDEOMODE *pVMArray;
DWORD dwVMArrSize;

INT_PTR CALLBACK FSVideoModeDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			ULONG i;
			WCHAR lpText[128] = {};
			DWORD dwSelectedVM;
			VIDEOMODE SVM = {};
			dwVMArrSize = pVideoMode->EnumVideoModes(0);
			pVMArray = new LPVIDEOMODE[dwVMArrSize];
			for (i = 0; i < dwVMArrSize; i++)
				pVMArray[i] = new VIDEOMODE;
			if (pVideoMode->EnumVideoModes(&pVMArray[0]))
			{
				SendDlgItemMessage(hWnd, IDC_CBOVM, CB_RESETCONTENT, 0, 0);
				for (i = 0; i < dwVMArrSize; i++)
				{
					swprintf(lpText, L"%i x %i, %i Bits, %i Hz", pVMArray[i]->dwPelsWidth,
						pVMArray[i]->dwPelsHeight, pVMArray[i]->dwBitsPerPel, pVMArray[i]->dwDisplayFrequency);
					SendDlgItemMessage(hWnd, IDC_CBOVM, CB_ADDSTRING, 0, (LPARAM)lpText);
				}
				SVM.dwPelsWidth = dwFSVideoModeWidth;
				SVM.dwPelsHeight = dwFSVideoModeHeight;
				SVM.dwBitsPerPel = dwFSVideoModeBPP;
				SVM.dwDisplayFrequency = dwFSVideoModeDF;
				dwSelectedVM = pVideoMode->GetVideoModeIndex(SVM);
				if (dwSelectedVM != VM_ERROR)
				{
					SendDlgItemMessage(hWnd, IDC_CBOVM, CB_SETCURSEL, dwSelectedVM, 0);
				}
			}
			SendDlgItemMessage(hWnd, IDC_CHKCVM, BM_SETCHECK, dwChangeFSVideoMode, 0);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BTNOK:
				{
					WCHAR lpText[128] = {};
					DWORD dwSelItem = (DWORD)SendDlgItemMessage(hWnd, IDC_CBOVM, CB_GETCURSEL, 0, 0);
					dwFSVideoModeWidth = pVMArray[dwSelItem]->dwPelsWidth;
					dwFSVideoModeHeight = pVMArray[dwSelItem]->dwPelsHeight;
					dwFSVideoModeBPP = pVMArray[dwSelItem]->dwBitsPerPel;
					dwFSVideoModeDF = pVMArray[dwSelItem]->dwDisplayFrequency;
					dwChangeFSVideoMode = (DWORD)SendDlgItemMessage(hWnd, IDC_CHKCVM, BM_GETCHECK, 0, 0);
					EndDialog(hWnd, 0);
					break;
				}
				case IDC_BTNCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			return TRUE;
		case WM_DESTROY:
		{
			for (ULONG i = 0; i < dwVMArrSize; i++)
				delete pVMArray[i];
			delete[] pVMArray;
			return TRUE;
		}
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}