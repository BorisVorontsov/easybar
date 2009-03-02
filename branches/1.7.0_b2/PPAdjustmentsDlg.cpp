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
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						/*LPWSTR lpwText;
						BOOL bVideoStm = (LOWORD(wParam) == IDC_CBOVS);
						BOOL bStmState;
						int intSelItem = SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETCURSEL, 0, 0);
						DWORD dwTextSize = SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETLBTEXTLEN, intSelItem, 0);
						lpwText = new WCHAR[dwTextSize + 1];
						ZeroMemory(lpwText, (dwTextSize + 1) * sizeof(WCHAR));
						SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETLBTEXT, intSelItem, (LPARAM)lpwText);
						bStmState = pEngine->IsStreamSelected((bVideoStm)?DSST_VIDEO:DSST_AUDIO, lpwText);
						EnableWindow(GetDlgItem(hWnd, (bVideoStm)?IDC_BTNSVS:IDC_BTNSAS), !bStmState);
						delete[] lpwText;*/
					}
					break;
				}
				case IDC_BTNSAS:
				{
					/*LPWSTR lpwText;
					BOOL bVideoStm = (LOWORD(wParam) == IDC_BTNSVS);
					int intSelItem = SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETCURSEL, 0, 0);
					DWORD dwTextSize = SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETLBTEXTLEN, intSelItem, 0);
					lpwText = new WCHAR[dwTextSize + 1];
					ZeroMemory(lpwText, (dwTextSize + 1) * sizeof(WCHAR));
					SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETLBTEXT, intSelItem, (LPARAM)lpwText);
					pEngine->SelectStream((bVideoStm)?DSST_VIDEO:DSST_AUDIO, lpwText);
					EnableWindow(GetDlgItem(hWnd, (bVideoStm)?IDC_BTNSVS:IDC_BTNSAS), FALSE);
					delete[] lpwText;
					break;*/
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