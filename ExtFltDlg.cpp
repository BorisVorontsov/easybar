#include <windows.h>
//#include <dshow.h>

#include "resource.h"
#include "common.h"
#include "engine.h"
#include "easybar.h"
#include "extfltdlg.h"

INT_PTR CALLBACK ExtFltDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			ULONG i;
			DWORD dwFASize = 0;
			pEngine->UpdateDSFiltersArray(DSF_CATEGORY_LEGACY_AM_FILTER);
			pEngine->GetDSFiltersNames(0, &dwFASize, 0);
			LPWSTR *lpDSFilters = new LPWSTR[dwFASize];
			for (i = 0; i < dwFASize; i++)
				lpDSFilters[i] = new WCHAR[MAX_PATH];
			if (pEngine->GetDSFiltersNames(&lpDSFilters[0], &dwFASize, MAX_PATH) >= 0)
			{
				SendDlgItemMessage(hWnd, IDC_LSTAF, LB_RESETCONTENT, 0, 0);
				for (i = 0; i < dwFASize; i++)
				{
					SendDlgItemMessage(hWnd, IDC_LSTAF, LB_ADDSTRING, 0,
						(LPARAM)lpDSFilters[i]);
				}
				SendDlgItemMessage(hWnd, IDC_LSTAF, LB_SETCURSEL, 0, 0);
				SetLBHorizontalExtent(GetDlgItem(hWnd, IDC_LSTAF));
			}
			for (i = 0; i < dwFASize; i++)
				delete[] lpDSFilters[i];
			delete[] lpDSFilters;
			for (i = 0; i < APP_MAX_STRINGS; i++)
			{
				if (!lpExternalFilters[i]) break;
				SendDlgItemMessage(hWnd, IDC_LSTSF, LB_ADDSTRING, 0,
					(LPARAM)lpExternalFilters[i]);
			}
			SendDlgItemMessage(hWnd, IDC_LSTSF, LB_SETCURSEL, 0, 0);
			SetLBHorizontalExtent(GetDlgItem(hWnd, IDC_LSTSF));
			SendDlgItemMessage(hWnd, IDC_CHKUEF, BM_SETCHECK, dwUseExternalFilters, 0);
			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_LSTAF, LBN_SELCHANGE), 0);
			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_CHKUEF, 0), 0);
			SetTimer(hWnd, 1, 200, 0);
			SendMessage(hWnd, WM_TIMER, 1, 0);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_LSTAF:
					switch (HIWORD(wParam))
					{
						case LBN_SELCHANGE:
						{
							WCHAR lpText[MAX_PATH] = {}, lpMerit[64] = {};
							LPWSTR lpCLSID = 0;
							DSFILTERINFO DSFI = {};
							DWORD dwSelItem = (DWORD)SendDlgItemMessage(hWnd, IDC_LSTAF, LB_GETCURSEL, 0, 0);
							SendDlgItemMessage(hWnd, IDC_LSTAF, LB_GETTEXT, dwSelItem, (LPARAM)lpText);
							pEngine->GetDSFilterInfo(lpText, &DSFI);
							StringFromCLSID(DSFI.cCLSID, &lpCLSID);
							SetDlgItemText(hWnd, IDC_EDTFCLSID, lpCLSID);
							CoTaskMemFree(lpCLSID);
							switch (DSFI.dwMerit)
							{
								case MERIT_PREFERRED:
									swprintf(lpMerit, L"MERIT_PREFERRED (0x%x)", MERIT_PREFERRED);
									break;
								case MERIT_NORMAL:
									swprintf(lpMerit, L"MERIT_NORMAL (0x%x)", MERIT_NORMAL);
									break;
								case MERIT_UNLIKELY:
									swprintf(lpMerit, L"MERIT_UNLIKELY (0x%x)", MERIT_UNLIKELY);
									break;
								case MERIT_DO_NOT_USE:
									swprintf(lpMerit, L"MERIT_DO_NOT_USE (0x%x)", MERIT_DO_NOT_USE);
									break;
								case MERIT_SW_COMPRESSOR:
									swprintf(lpMerit, L"MERIT_SW_COMPRESSOR (0x%x)", MERIT_SW_COMPRESSOR);
									break;
								case MERIT_HW_COMPRESSOR:
									swprintf(lpMerit, L"MERIT_SW_COMPRESSOR (0x%x)", MERIT_HW_COMPRESSOR);
									break;
								default:
									swprintf(lpMerit, L"UNKNOWN (0x%x)", DSFI.dwMerit);
									break;
							}
							SetDlgItemText(hWnd, IDC_EDTFM, lpMerit);
							break;
						}
						case LBN_DBLCLK:
							if (IsWindowEnabled(GetDlgItem(hWnd, IDC_BTNADD)))
								PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BTNADD, 0), 0);
							break;
					}
					break;
				case IDC_LSTSF:

					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						if (IsWindowEnabled(GetDlgItem(hWnd, IDC_BTNREM)))
							PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BTNREM, 0), 0);
					}
					break;
				case IDC_BTNADD:
				{
					WCHAR lpText[MAX_PATH] = {};
					DWORD dwSelItem = (DWORD)SendDlgItemMessage(hWnd, IDC_LSTAF, LB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hWnd, IDC_LSTAF, LB_GETTEXT, dwSelItem, (LPARAM)lpText);
					SendDlgItemMessage(hWnd, IDC_LSTSF, LB_ADDSTRING, 0, (LPARAM)lpText);
					DWORD dwSFCount = (DWORD)SendDlgItemMessage(hWnd, IDC_LSTSF, LB_GETCOUNT, 0, 0);
					SendDlgItemMessage(hWnd, IDC_LSTSF, LB_SETCURSEL, dwSFCount - 1, 0);
					SetLBHorizontalExtent(GetDlgItem(hWnd, IDC_LSTSF));
					break;
				}
				case IDC_BTNREM:
				{
					DWORD dwSelItem = (DWORD)SendDlgItemMessage(hWnd, IDC_LSTSF, LB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hWnd, IDC_LSTSF, LB_DELETESTRING, dwSelItem, 0);
					if (dwSelItem > 1)
					{
						SendDlgItemMessage(hWnd, IDC_LSTSF, LB_SETCURSEL, dwSelItem - 1, 0);
					}
					else
					{
						SendDlgItemMessage(hWnd, IDC_LSTSF, LB_SETCURSEL, 0, 0);
					}
					SetLBHorizontalExtent(GetDlgItem(hWnd, IDC_LSTSF));
					break;
				}
				case IDC_CHKUEF:
				{
					BOOL bFlag = (BOOL)SendDlgItemMessage(hWnd, IDC_CHKUEF, BM_GETCHECK, 0, 0);
					EnableWindow(GetDlgItem(hWnd, IDC_LSTSF), bFlag);
					break;
				}
				case IDC_BTNOK:
				{
					ULONG i = 0;
					WCHAR lpText[MAX_PATH] = {};
					for (; (i < APP_MAX_STRINGS) && (lpExternalFilters[i]); i++)
					{
						delete[] lpExternalFilters[i];
						lpExternalFilters[i] = 0;
					}
					DWORD dwSFCount = (DWORD)SendDlgItemMessage(hWnd, IDC_LSTSF, LB_GETCOUNT, 0, 0);
					for (i = 0; i < dwSFCount; i++)
					{
						SendDlgItemMessage(hWnd, IDC_LSTSF, LB_GETTEXT, i, (LPARAM)lpText);
						lpExternalFilters[i] = new WCHAR[MAX_PATH];
						wcscpy(lpExternalFilters[i], lpText);
					}
					dwUseExternalFilters = (DWORD)SendDlgItemMessage(hWnd, IDC_CHKUEF, BM_GETCHECK, 0, 0);
					EndDialog(hWnd, 0);
					break;
				}
				case IDC_BTNCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			return TRUE;
		case WM_TIMER:
		{
			if (wParam == 1)
			{
				BOOL bCtlsFlag1  = (SendDlgItemMessage(hWnd, IDC_LSTAF, LB_GETCURSEL, 0, 0) != LB_ERR);
				BOOL bCtlsFlag2  = (SendDlgItemMessage(hWnd, IDC_LSTSF, LB_GETCURSEL, 0, 0) != LB_ERR);
				EnableWindow(GetDlgItem(hWnd, IDC_BTNADD), bCtlsFlag1);
				EnableWindow(GetDlgItem(hWnd, IDC_BTNREM), bCtlsFlag2);
			}
			return TRUE;
		}
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}