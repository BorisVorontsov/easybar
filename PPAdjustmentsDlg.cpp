#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "engine.h"
#include "easybar.h"
#include "ppadjustmentsdlg.h"

static HWND hPPAdjustments = 0;

INT_PTR CALLBACK PPAdjustmentsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			hPPAdjustments = hWnd;
			PPSetDefFileInfo(hWnd, pEngine->m_lpwFileName);
			GetFGAudioStreams();
			if (SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETCOUNT, 0, 0))
				PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_CBOAS, CBN_SELCHANGE), 0);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_CBOAS:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int intSelItem = SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETCURSEL, 0, 0);
						BOOL bASSelected = pEngine->IsAudioStreamSelected_E(intSelItem);
						EnableWindow(GetDlgItem(hWnd, IDC_BTNSAS), !bASSelected);
					}
					break;
				case IDC_BTNSAS:
				{
					int intSelItem = SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETCURSEL, 0, 0);
					pEngine->SelectAudioStream_E(intSelItem, 0, 0, 0);
				}
			}
			return TRUE;
		case WM_DESTROY:
			hPPAdjustments = 0;
			return TRUE;
	}
	return FALSE;
}

//Обновляет списки потоков (аудио)
void GetFGAudioStreams()
{
	if (!hPPAdjustments) return;
	ULONG i, lFGASCnt;
	int intSelStm = 0;
	WCHAR lpwText[128] = { 0 };
	lFGASCnt = (ULONG)pEngine->GetAudioStreamsCount_E();

	if (!lFGASCnt)
	{
		EnableWindow(GetDlgItem(hPPAdjustments, IDC_CBOAS), FALSE);
		EnableWindow(GetDlgItem(hPPAdjustments, IDC_BTNSAS), FALSE);
		return;
	}
	
	SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_RESETCONTENT, 0, 0);
	for (i = 0; i < lFGASCnt; i++)
	{
		swprintf(lpwText, L"Audio stream #%i", i + 1);
		SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_ADDSTRING, 0, (LPARAM)lpwText);
		if (pEngine->IsAudioStreamSelected_E(i))
			intSelStm = i;
	}
	SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_SETCURSEL, intSelStm, 0);
}