#include <windows.h>

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
			GetFGStreams();
			GetFGStreams(TRUE);
			if (SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETCOUNT, 0, 0))
				PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_CBOAS, CBN_SELCHANGE), 0);
			if (SendDlgItemMessage(hWnd, IDC_CBOVS, CB_GETCOUNT, 0, 0))
				PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_CBOVS, CBN_SELCHANGE), 0);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_CBOAS:
				case IDC_CBOVS:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						LPWSTR lpwText;
						BOOL bVideoStm = (LOWORD(wParam) == IDC_CBOVS);
						BOOL bStmState;
						int intSelItem = SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETCURSEL, 0, 0);
						DWORD dwTextSize = SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETLBTEXTLEN, intSelItem, 0);
						lpwText = new WCHAR[dwTextSize + 1];
						ZeroMemory(lpwText, (dwTextSize + 1) * sizeof(WCHAR));
						SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETLBTEXT, intSelItem, (LPARAM)lpwText);
						bStmState = pEngine->IsStreamSelected((bVideoStm)?DSST_VIDEO:DSST_AUDIO, lpwText);
						EnableWindow(GetDlgItem(hWnd, (bVideoStm)?IDC_BTNSVS:IDC_BTNSAS), !bStmState);
						delete[] lpwText;
					}
					break;
				}
				case IDC_BTNSAS:
				case IDC_BTNSVS:
				{
					LPWSTR lpwText;
					BOOL bVideoStm = (LOWORD(wParam) == IDC_BTNSVS);
					int intSelItem = SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETCURSEL, 0, 0);
					DWORD dwTextSize = SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETLBTEXTLEN, intSelItem, 0);
					lpwText = new WCHAR[dwTextSize + 1];
					ZeroMemory(lpwText, (dwTextSize + 1) * sizeof(WCHAR));
					SendDlgItemMessage(hWnd, (bVideoStm)?IDC_CBOVS:IDC_CBOAS, CB_GETLBTEXT, intSelItem, (LPARAM)lpwText);
					pEngine->SelectStream((bVideoStm)?DSST_VIDEO:DSST_AUDIO, lpwText);
					EnableWindow(GetDlgItem(hWnd, (bVideoStm)?IDC_BTNSVS:IDC_BTNSAS), FALSE);
					delete[] lpwText;
					break;
				}
			}
			return TRUE;
	}
	return FALSE;
}

//Обновляет списки потоков (аудио/видео)
void GetFGStreams(BOOL bVideo)
{
	if (!hPPAdjustments) return;
	ULONG i;
	DWORD dwFGSASize = 0;
	int intSelStm = 0;
	pEngine->GetAvailableStreams((bVideo)?DSST_VIDEO:DSST_AUDIO, 0, &dwFGSASize, 0);
	if (!dwFGSASize)
	{
		EnableWindow(GetDlgItem(hPPAdjustments, (bVideo)?IDC_CBOVS:IDC_CBOAS), FALSE);
		EnableWindow(GetDlgItem(hPPAdjustments, (bVideo)?IDC_BTNSVS:IDC_BTNSAS), FALSE);
	}
	LPWSTR *lpwFGStreams = new LPWSTR[dwFGSASize];
	for (i = 0; i < dwFGSASize; i++)
		lpwFGStreams[i] = new WCHAR[MAX_PATH];
	if (pEngine->GetAvailableStreams((bVideo)?DSST_VIDEO:DSST_AUDIO, &lpwFGStreams[0],
		&dwFGSASize, MAX_PATH) >= 0)
	{
		SendDlgItemMessage(hPPAdjustments, (bVideo)?IDC_CBOVS:IDC_CBOAS, CB_RESETCONTENT, 0, 0);
		for (i = 0; i < dwFGSASize; i++)
		{
			int intIndex = SendDlgItemMessage(hPPAdjustments, (bVideo)?IDC_CBOVS:IDC_CBOAS, CB_ADDSTRING, 0,
				(LPARAM)lpwFGStreams[i]);
			if (pEngine->IsStreamSelected((bVideo)?DSST_VIDEO:DSST_AUDIO, lpwFGStreams[i]))
				intSelStm = i;
		}
		SendDlgItemMessage(hPPAdjustments, (bVideo)?IDC_CBOVS:IDC_CBOAS, CB_SETCURSEL, intSelStm, 0);
	}
	for (i = 0; i < dwFGSASize; i++)
		delete[] lpwFGStreams[i];
	delete[] lpwFGStreams;
}