#include <windows.h>

#include "resource.h"
#include "common.h"
#include "easybar.h"
#include "colorsdlg.h"

extern HINSTANCE hAppInstance;

static LPDWORD pOldColors;
static DWORD dwSelColor;

INT_PTR CALLBACK ColorsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_ADDSTRING, 0, (LPARAM)L"Background");
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 0, dwBackgroundColor);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_ADDSTRING, 0, (LPARAM)L"Gradient (Top/Left Side)");
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 1, dwGradientColor1);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_ADDSTRING, 0, (LPARAM)L"Gradient (Bottom/Right Side)");
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 2, dwGradientColor2);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_ADDSTRING, 0, (LPARAM)L"Border (Outer)");
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 3, dwBorderColor1);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_ADDSTRING, 0, (LPARAM)L"Border (Inner)");
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 4, dwBorderColor2);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_ADDSTRING, 0, (LPARAM)L"Text");
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 5, dwTextColor);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_ADDSTRING, 0, (LPARAM)L"Text Shadow");
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 6, dwTextShadowColor);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_ADDSTRING, 0, (LPARAM)L"Active Item Text");
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 7, dwActiveItemTextColor);
			SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETCURSEL, 0, 0);
			//--------------------------------------------------------------------
			//DWORD dwClrsCnt = SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_GETCOUNT, 0, 0);
			pOldColors = new DWORD[8/*dwClrsCnt*/];
			for (UINT i = 0; i < 8/*dwClrsCnt*/; i++)
				pOldColors[i] = SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_GETITEMDATA, i, 0);
			//--------------------------------------------------------------------
			SendDlgItemMessage(hWnd, IDC_CHKP, BM_SETCHECK, BST_CHECKED, 0);
			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_CBOITEMS, CBN_SELCHANGE), 0);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_CBOITEMS:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						DWORD dwSelItem = SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_GETCURSEL, 0, 0);
						dwSelColor = SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_GETITEMDATA, dwSelItem, 0);
						RedrawWindow(GetDlgItem(hWnd, IDC_STCCOLOR), 0, 0, RDW_INVALIDATE | RDW_ERASE);
					}
					break;
				}
				case IDC_STCCOLOR:
				{
					if (HIWORD(wParam) == STN_DBLCLK)
					{
						if (GetColorDialog(hAppInstance, hWnd, (LPCOLORREF)&dwSelColor))
						{
							DWORD dwSelItem = SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_GETCURSEL, 0, 0);
							SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, dwSelItem, dwSelColor);
							RedrawWindow(GetDlgItem(hWnd, IDC_STCCOLOR), 0, 0, RDW_INVALIDATE | RDW_ERASE);
							if (SendDlgItemMessage(hWnd, IDC_CHKP, BM_GETCHECK, 0, 0) == BST_CHECKED)
							{
								ApplyCurrentColors(GetDlgItem(hWnd, IDC_CBOITEMS));
								UpdateEBColors();
							}
						}
					}
					break;
				}
				case IDC_BTNDEFAULT:
					SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 0, EB_BACKGROUND_COLOR);
					SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 1, EB_GRADIENT_COLOR_1);
					SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 2, EB_GRADIENT_COLOR_2);
					SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 3, EB_BORDER_COLOR_1);
					SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 4, EB_BORDER_COLOR_2);
					SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 5, EB_TEXT_COLOR);
					SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 6, EB_TEXT_SHADOW_COLOR);
					SendDlgItemMessage(hWnd, IDC_CBOITEMS, CB_SETITEMDATA, 7, EB_ACTIVE_ITEM_TEXT_COLOR);
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_CBOITEMS, CBN_SELCHANGE), 0);
					ApplyCurrentColors(GetDlgItem(hWnd, IDC_CBOITEMS));
					UpdateEBColors();
					break;
				case IDC_CHKP:
					if (SendDlgItemMessage(hWnd, IDC_CHKP, BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						ApplyCurrentColors(GetDlgItem(hWnd, IDC_CBOITEMS));
						UpdateEBColors();
					}
					else
					{
						RestoreOldColors();
						UpdateEBColors();
					}
					break;
				case IDC_BTNOK:
					ApplyCurrentColors(GetDlgItem(hWnd, IDC_CBOITEMS));
					EndDialog(hWnd, 0);
					break;
				case IDC_BTNCANCEL:
					RestoreOldColors();
					EndDialog(hWnd, 0);
					break;
			}
			return TRUE;
		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == GetDlgItem(hWnd, IDC_STCCOLOR))
			{
				return (INT_PTR)CreateSolidBrush(dwSelColor);
			}
			return FALSE;
		case WM_DESTROY:
			delete[] pOldColors;
			return TRUE;
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}

void ApplyCurrentColors(HWND hCBO)
{
	dwBackgroundColor = SendMessage(hCBO, CB_GETITEMDATA, 0, 0);
	dwGradientColor1 = SendMessage(hCBO, CB_GETITEMDATA, 1, 0);
	dwGradientColor2 = SendMessage(hCBO, CB_GETITEMDATA, 2, 0);
	dwBorderColor1 = SendMessage(hCBO, CB_GETITEMDATA, 3, 0);
	dwBorderColor2 = SendMessage(hCBO, CB_GETITEMDATA, 4, 0);
	dwTextColor = SendMessage(hCBO, CB_GETITEMDATA, 5, 0);
	dwTextShadowColor = SendMessage(hCBO, CB_GETITEMDATA, 6, 0);
	dwActiveItemTextColor = SendMessage(hCBO, CB_GETITEMDATA, 7, 0);
}

void RestoreOldColors()
{
	dwBackgroundColor = pOldColors[0];
	dwGradientColor1 = pOldColors[1];
	dwGradientColor2 = pOldColors[2];
	dwBorderColor1 = pOldColors[3];
	dwBorderColor2 = pOldColors[4];
	dwTextColor = pOldColors[5];
	dwTextShadowColor = pOldColors[6];
	dwActiveItemTextColor = pOldColors[7];
}