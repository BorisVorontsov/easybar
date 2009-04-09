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
						BOOL bASSelected;
						if (bWorkWithSplitter)
						{
                            LPWSTR lpwText;
                            DWORD dwTextSize = SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETLBTEXTLEN, intSelItem, 0);
                            lpwText = new WCHAR[dwTextSize + 1];
                            ZeroMemory(lpwText, (dwTextSize + 1) * sizeof(WCHAR));
                            SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETLBTEXT, intSelItem, (LPARAM)lpwText);
                            bASSelected = pEngine->IsStreamSelected(DSST_AUDIO, lpwText);
							delete[] lpwText;
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
					int intSelItem = SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETCURSEL, 0, 0);
					if (bWorkWithSplitter)
					{
                        LPWSTR lpwText;
                        DWORD dwTextSize = SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETLBTEXTLEN, intSelItem, 0);
                        lpwText = new WCHAR[dwTextSize + 1];
                        ZeroMemory(lpwText, (dwTextSize + 1) * sizeof(WCHAR));
                        SendDlgItemMessage(hWnd, IDC_CBOAS, CB_GETLBTEXT, intSelItem, (LPARAM)lpwText);
                        pEngine->SelectStream(DSST_AUDIO, lpwText);
						delete[] lpwText;
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
			LPWSTR *lpwFGStreams = new LPWSTR[dwFGASASize];
			for (i = 0; i < dwFGASASize; i++)
				lpwFGStreams[i] = new WCHAR[MAX_PATH];
			pEngine->GetAvailableStreams(DSST_AUDIO, &lpwFGStreams[0], &dwFGASASize, MAX_PATH);
			for (i = 0; i < dwFGASASize; i++)
			{
				int intIndex = SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_ADDSTRING, 0,
					(LPARAM)lpwFGStreams[i]);
				if (pEngine->IsStreamSelected(DSST_AUDIO, lpwFGStreams[i]))
					intSelStm = i;
			}
			for (i = 0; i < dwFGASASize; i++)
				delete[] lpwFGStreams[i];
			delete[] lpwFGStreams;
			bWorkWithSplitter = TRUE;
		}
		else
		{
			//Если от сплиттера ничего не получили, просто перечисляем рендерреры аудио
AudioStreams_EnumRenderrers:
			WCHAR lpwText[128] = { 0 };
			for (i = 0; i < lFGASCnt; i++)
			{
				swprintf(lpwText, L"Audio stream #%i", i + 1);
				SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_ADDSTRING, 0, (LPARAM)lpwText);
				if (pEngine->IsAudioStreamSelected_E(i))
					intSelStm = i;
			}
			bWorkWithSplitter = FALSE;
		}
	}
	else goto AudioStreams_EnumRenderrers;

	SendDlgItemMessage(hPPAdjustments, IDC_CBOAS, CB_SETCURSEL, intSelStm, 0);
}