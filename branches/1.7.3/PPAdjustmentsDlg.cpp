#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "engine.h"
#include "easybar.h"
#include "ppadjustmentsdlg.h"

static HWND hPPAdjustments = 0;

static BOOL bWorkWithSplitter;

INT_PTR CALLBACK PPAdjustmentsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			hPPAdjustments = hWnd;
			PPSetDefFileInfo(hWnd, pEngine->m_lpFileName);
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
						int intSelItem = (int)SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETCURSEL, 0, 0);
						BOOL bASSelected;
						if (bWorkWithSplitter)
						{
							LPWSTR lpText;
							DWORD dwTextSize = (int)SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETLBTEXTLEN, intSelItem, 0);
							lpText = new WCHAR[dwTextSize + 1];
							ZeroMemory(lpText, (dwTextSize + 1) * sizeof(WCHAR));
							SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETLBTEXT, intSelItem, (LPARAM)lpText);
							bASSelected = pEngine->IsStreamSelected(DSST_AUDIO, lpText);
							delete[] lpText;
						}
						else
						{
							bASSelected = pEngine->IsAudioStreamSelected_E(intSelItem);
						}
						EnableWindow(GetDlgItem(hWnd, IDC_BTNSAS), !bASSelected);
					}
					break;
				case IDC_BTNSAS:
				{
					int intSelItem = (int)SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETCURSEL, 0, 0);
					if (bWorkWithSplitter)
					{
						LPWSTR lpText;
						DWORD dwTextSize = (DWORD)SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETLBTEXTLEN, intSelItem, 0);
						lpText = new WCHAR[dwTextSize + 1];
						ZeroMemory(lpText, (dwTextSize + 1) * sizeof(WCHAR));
						SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETLBTEXT, intSelItem, (LPARAM)lpText);
						pEngine->SelectStream(DSST_AUDIO, lpText);
						delete[] lpText;
					}
					else
					{
						pEngine->SelectAudioStream_E(intSelItem, 0, 0, 0);
					}
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

	lFGASCnt = (ULONG)pEngine->GetAudioStreamsCount_E();

	if (!lFGASCnt)
	{
		EnableWindow(GetDlgItem(hPPAdjustments, IDC_CBOAS), FALSE);
		EnableWindow(GetDlgItem(hPPAdjustments, IDC_BTNSAS), FALSE);
		return;
	}

	SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_RESETCONTENT, 0, 0);

	if (lFGASCnt == 1)
	{
		//Возможно, удастся получить что-нибудь от сплиттера...
		DWORD dwFGASASize = 0;
		pEngine->GetAvailableStreams(DSST_AUDIO, 0, &dwFGASASize, 0);
		if (dwFGASASize)
		{
			LPWSTR *lpFGStreams = new LPWSTR[dwFGASASize];
			for (i = 0; i < dwFGASASize; i++)
				lpFGStreams[i] = new WCHAR[MAX_PATH];
			pEngine->GetAvailableStreams(DSST_AUDIO, &lpFGStreams[0], &dwFGASASize, MAX_PATH);
			for (i = 0; i < dwFGASASize; i++)
			{
				int intIndex = (int)SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_ADDSTRING, 0,
					(LPARAM)lpFGStreams[i]);
				if (pEngine->IsStreamSelected(DSST_AUDIO, lpFGStreams[i]))
					intSelStm = i;
			}
			for (i = 0; i < dwFGASASize; i++)
				delete[] lpFGStreams[i];
			delete[] lpFGStreams;
			bWorkWithSplitter = TRUE;
		}
		else
		{
			//Если от сплиттера ничего не получили, просто перечисляем рендерреры аудио
AudioStreams_EnumRenderrers:
			WCHAR lpText[128] = {};
			for (i = 0; i < lFGASCnt; i++)
			{
				swprintf(lpText, L"Audio stream #%i", i + 1);
				SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_ADDSTRING, 0, (LPARAM)lpText);
				if (pEngine->IsAudioStreamSelected_E(i))
					intSelStm = i;
			}
			bWorkWithSplitter = FALSE;
		}
	}
	else goto AudioStreams_EnumRenderrers;

	SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_SETCURSEL, intSelStm, 0);
}