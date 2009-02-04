//Здесь находятся общие функции, которые могут быть использованы
//не только из основного модуля

#include <windows.h>
#include <shlobj.h>

#include "common.h"

#pragma comment (lib, "msimg32.lib")

//Рисование текста с эффектом 3D
void Draw3DText(HWND hWnd,
				LPWSTR lpwText,
				COLORREF crBkColorOne,
				COLORREF crBkColorTwo,
				DWORD dwBkDirection,
				COLORREF crFontColor,
				COLORREF crShadowColor,
				LONG lShadOffsetX,
				LONG lShadOffsetY)
{
	RECT RC = { 0 };
	HDC hDC = GetDC(hWnd);
	HFONT hFont = CreateFont(-MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72)
		, 0, 0, 0, 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Tahoma");
	HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
	if (GetBkMode(hDC) != TRANSPARENT) SetBkMode(hDC, TRANSPARENT);
	GetClientRect(hWnd, &RC);
	if (crBkColorOne == crBkColorTwo)
	{
		HBRUSH hBkBrush = CreateSolidBrush(crBkColorOne);
		FillRect(hDC, &RC, hBkBrush);
		DeleteObject(hBkBrush);
	}
	else
	{
		TRIVERTEX TV[2] = { 0 };
		GRADIENT_RECT GR = { 0 };
		TV[0].x = RC.left;
		TV[0].y = RC.top;
		TV[0].Red = GetRValue(crBkColorOne) << 8;
		TV[0].Green = GetGValue(crBkColorOne) << 8;
		TV[0].Blue = GetBValue(crBkColorOne) << 8;
		TV[0].Alpha = 0;
		TV[1].x = RC.right;
		TV[1].y = RC.bottom;
		TV[1].Red =  GetRValue(crBkColorTwo) << 8;
		TV[1].Green = GetGValue(crBkColorTwo) << 8;
		TV[1].Blue =  GetBValue(crBkColorTwo) << 8;
		TV[1].Alpha = 0;
		GR.UpperLeft = 0;
		GR.LowerRight = 1;
		GradientFill(hDC, &TV[0], 2, (PVOID)&GR, 1, (dwBkDirection)?
			GRADIENT_FILL_RECT_V:GRADIENT_FILL_RECT_H);
	}
	if ((lShadOffsetX != 0) || (lShadOffsetY != 0))
	{
		OffsetRect(&RC, lShadOffsetX, lShadOffsetY);
		SetTextColor(hDC, crShadowColor);
		DrawText(hDC, lpwText, -1, &RC, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
		OffsetRect(&RC, -(lShadOffsetX), -(lShadOffsetY));
	}
	SetTextColor(hDC, crFontColor);
	DrawText(hDC, lpwText, -1, &RC, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}

//Устанавливает ListBox'у параметры для горизонтальной прокрутки
void SetLBHorizontalExtent(HWND hListBox)
{
	WCHAR lpwText[MAX_PATH] = { 0 };
	HDC hDC;
	HFONT hOldFont;
	RECT RCL = { 0 };
	LONG lLBStyle, lMaxTextWidth = 0;
	ULONG i, uItemsCnt;
	hDC = GetDC(hListBox);
	hOldFont = (HFONT)SelectObject(hDC, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
	uItemsCnt = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
	for (i = 0; i < uItemsCnt; i++)
	{
		SendMessage(hListBox, LB_GETTEXT, i, (LPARAM)lpwText);
		DrawText(hDC, lpwText, -1, &RCL, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
		if (RCL.right > lMaxTextWidth) lMaxTextWidth = RCL.right;
	}
	lLBStyle = GetWindowLong(hListBox, GWL_STYLE);
	if ((lLBStyle & WS_VSCROLL) == WS_VSCROLL)
		lMaxTextWidth += GetSystemMetrics(SM_CXVSCROLL);
	SendMessage(hListBox, LB_SETHORIZONTALEXTENT, lMaxTextWidth, 0);
	SelectObject(hDC, hOldFont);
	ReleaseDC(hListBox, hDC);
}

//Перемещение окна в центр экрана (с указанными сдвигами)
void MoveToCenter(HWND hWnd,
				  LONG lXOffset,
				  LONG lYOffset)
{
	RECT RC = { 0 };
	int intSX = 0, intSY = 0;
	int intNL = 0, intNT = 0;
	GetWindowRect(hWnd, &RC); 
	intSX = GetSystemMetrics(SM_CXSCREEN);
	intSY = GetSystemMetrics(SM_CYSCREEN);
	intNL = ((intSX / 2) - ((RC.right - RC.left) / 2)) + lXOffset;
	intNT = ((intSY / 2) - ((RC.bottom - RC.top) / 2)) + lYOffset;
	SetWindowPos(hWnd, 0, intNL, intNT, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

//Прилипание к краям экрана
void StickyWindow(HWND hWnd, LPRECT pCRC)
{
	RECT RCW = { 0 };
	int intThreshold = 5; //Порог прилипания
	int intSX = 0, intSY = 0;
	GetWindowRect(hWnd, &RCW);
	intSX = GetSystemMetrics(SM_CXSCREEN);
	intSY = GetSystemMetrics(SM_CYSCREEN);
	if ((pCRC->left <= intThreshold) && (pCRC->left >= -(intThreshold))) pCRC->left = 0;
	if ((pCRC->right >= (intSX - intThreshold)) && (pCRC->right <= (intSX + intThreshold)))
		pCRC->left = (intSX - (RCW.right - RCW.left));
	if ((pCRC->top <= intThreshold) && (pCRC->top >= -(intThreshold))) pCRC->top = 0;
	if ((pCRC->bottom >= (intSY - intThreshold)) && (pCRC->bottom <= (intSY + intThreshold)))
		pCRC->top = (intSY - (RCW.bottom - RCW.top));
	pCRC->right = pCRC->left + (RCW.right - RCW.left);
	pCRC->bottom = pCRC->top + (RCW.bottom - RCW.top);
}

//Обертка над ShowCursor. Позволяет одним вызовом показать/скрыть курсор
//(независимо от состояния счетчика у ShowCursor)
void ShowMousePointer(BOOL bShow)
{
	if (bShow)
	{
		while (ShowCursor(TRUE) < 0){};
	}
	else
	{
		while (ShowCursor(FALSE) >= 0){};
	}
}

BOOL GetAppPath(HINSTANCE hApp,
				LPWSTR lpwPath,
				DWORD dwPathLen,
				BOOL bAddQuotes)
{
    LPWSTR lpwBuff = new WCHAR[dwPathLen];
	if (GetModuleFileName(hApp, lpwBuff, dwPathLen) == 0)
	{
		delete[] lpwBuff;
		return FALSE;
	}
	if (wcschr(lpwBuff, ' ') && bAddQuotes)
	{
		WCHAR lpwQt[1] = { 0 };
		lpwQt[0] = '"';
		wcscpy(lpwPath, lpwQt);
		wcscat(lpwPath, lpwBuff);
		wcscat(lpwPath, lpwQt);
	}
	else wcscpy(lpwPath, lpwBuff);
	delete[] lpwBuff;
	return TRUE;
}

DWORD ReadFileVersion(LPCWSTR lpwFileName,
					  LPWSTR lpwResult,
					  UINT uResultSize,
					  DWORD dwFlags)
{
    DWORD dwDummy;
	BYTE *bBuffer;
	VS_FIXEDFILEINFO *VS_BUFF;
    DWORD dwBufferLen;
    UINT uVerBufferLen;
	WCHAR lpwTmp[8] = { 0 };
	UINT uRLen;
    dwBufferLen = GetFileVersionInfoSize(lpwFileName, &dwDummy);
    if (dwBufferLen < 1)
	{
		wcscpy(lpwResult, L"None");
		return 0;
	}
    bBuffer = new BYTE[dwBufferLen];
    GetFileVersionInfo(lpwFileName, 0, dwBufferLen, &bBuffer[0]);
    VerQueryValue(&bBuffer[0], L"\\", (PVOID *)&VS_BUFF, &uVerBufferLen);
	ZeroMemory(lpwResult, uResultSize);
	if ((dwFlags & RFV_MAJOR) == RFV_MAJOR)
	{
		_itow(HIWORD(VS_BUFF->dwProductVersionMS), lpwTmp, 10);
		wcscpy(lpwResult, lpwTmp);
	}
	if ((dwFlags & RFV_MINOR) == RFV_MINOR)
	{
		uRLen = wcslen(lpwResult);
		if (uRLen && (lpwResult[uRLen - 1] != '\0'))
			wcscat(lpwResult, L".");
		_itow(LOWORD(VS_BUFF->dwProductVersionMS), lpwTmp, 10);
		wcscat(lpwResult, lpwTmp);
	}
	if ((dwFlags & RFV_RELEASE) == RFV_RELEASE)
	{
		uRLen = wcslen(lpwResult);
		if (uRLen && (lpwResult[uRLen - 1] != '\0'))
			wcscat(lpwResult, L".");
		_itow(HIWORD(VS_BUFF->dwProductVersionLS), lpwTmp, 10);
		wcscat(lpwResult, lpwTmp);
	}
	if ((dwFlags & RFV_BUILD) == RFV_BUILD)
	{
		uRLen = wcslen(lpwResult);
		if (uRLen && (lpwResult[uRLen - 1] != '\0'))
			wcscat(lpwResult, L".");
		_itow(LOWORD(VS_BUFF->dwProductVersionLS), lpwTmp, 10);
		wcscat(lpwResult, lpwTmp);
	}
	delete[] bBuffer;
	return 1;
}

DWORD GetOpenDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCWSTR lpwTitle,
					LPWSTR lpwFileName,
					DWORD dwFNSize,
					LPCWSTR lpwFilter,
					DWORD dwFilterIndex,
					BOOL bMultiSelect,
					LPCWSTR lpwInitialDir)
{
	OPENFILENAME OFN = { 0 };
	OFN.lStructSize = sizeof(OFN);
	OFN.hInstance = hInstance;
	OFN.hwndOwner = hWnd;
	OFN.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES;
	if (bMultiSelect)
		OFN.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	OFN.lpstrTitle = lpwTitle;
	OFN.lpstrFile = lpwFileName;
	OFN.nMaxFile = dwFNSize;
	OFN.lpstrFilter = lpwFilter;
	OFN.nFilterIndex = dwFilterIndex;
	OFN.lpstrInitialDir = lpwInitialDir;
	if (GetOpenFileName(&OFN))
	{
		return wcslen(lpwFileName);
	}
	else return 0;
}

DWORD GetSaveDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCWSTR lpwTitle,
					LPWSTR lpwFileName,
					DWORD dwFNSize,
					LPCWSTR lpwFilter,
					LPDWORD pFilterIndex,
					LPCWSTR lpwDefExt,
					LPCWSTR lpwInitialDir)
{
	OPENFILENAME OFN = { 0 };
	OFN.lStructSize = sizeof(OFN);
	OFN.hInstance = hInstance;
	OFN.hwndOwner = hWnd;
	OFN.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT |
		OFN_NOREADONLYRETURN | OFN_EXPLORER;
	OFN.lpstrTitle = lpwTitle;
	OFN.lpstrFile = lpwFileName;
	OFN.nMaxFile = dwFNSize;
	OFN.lpstrFilter = lpwFilter;
	OFN.nFilterIndex = *pFilterIndex;
	OFN.lpstrDefExt = lpwDefExt;
	OFN.lpstrInitialDir = lpwInitialDir;
	if (GetSaveFileName(&OFN))
	{
		*pFilterIndex = OFN.nFilterIndex;
		return wcslen(lpwFileName);
	}
	else return 0;
}

BOOL GetColorDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCOLORREF crColor)
{
	CHOOSECOLOR CC = { 0 };
	ULONG i, lClr = 0;
	COLORREF crCustomColors[16] = { 0 };
	for (i = 0; i < 16; i++)
	{
		crCustomColors[i] = RGB(255 - lClr, 255 - lClr, 255 - lClr);
		lClr += 17;
	}
	CC.lStructSize = sizeof(CC);
	CC.hInstance = (HWND)hInstance;
	CC.hwndOwner = hWnd;
	CC.rgbResult = *crColor;
	CC.lpCustColors = crCustomColors;
	CC.Flags = CC_RGBINIT | CC_FULLOPEN;
    if (ChooseColor(&CC))
	{
		*crColor = CC.rgbResult;
		return TRUE;
	}
	return FALSE;
}

DWORD GetBrowseForFolderDialog(HWND hWnd, LPWSTR lpwFolder, LPCWSTR lpwTitle, LPCWSTR lpwInitialDir)
{
	CoInitialize(NULL);
	DWORD dwResult = 0;
    BROWSEINFO BI = { 0 };
    LPITEMIDLIST pIIDL;
    BI.hwndOwner = hWnd;
	BI.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	BI.lpszTitle = lpwTitle;
	BI.lpfn = BFFCallbackProc;
	BI.lParam = (LPARAM)lpwInitialDir;
    pIIDL = SHBrowseForFolder(&BI);
	if (!pIIDL)
	{
		CoUninitialize();
		return dwResult;
	}
    if (SHGetPathFromIDList(pIIDL, lpwFolder))
	{
		dwResult = wcslen(lpwFolder);
	}
	CoTaskMemFree(pIIDL);
	CoUninitialize();
	return dwResult;
}

int CALLBACK BFFCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		if (lpData)
			SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

void AdjustPrivilege(LPWSTR lpwPrivilege)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES TKP = { 0 };
	TOKEN_PRIVILEGES TKPOLD = { 0 };
	DWORD dwPSLength;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
        LookupPrivilegeValue(NULL, lpwPrivilege, &TKP.Privileges[0].Luid);
        TKP.PrivilegeCount = 1;
        TKP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &TKP, sizeof(TKPOLD), &TKPOLD, &dwPSLength);
    }
}

BOOL IsFile(LPWSTR lpwPath)
{
	LPWSTR lpwTmp = new WCHAR[MAX_PATH];
	DWORD dwErr;
	wcscpy(lpwTmp, lpwPath);
	if ((lpwTmp[0] == '"') && (lpwTmp[wcslen(lpwTmp) - 1] == '"'))
	{
		lpwTmp++;
		lpwTmp[wcslen(lpwTmp) - 1] = '\0';
	}
	HANDLE hFile = CreateFile(lpwTmp, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	delete[] lpwTmp;
	if (hFile == INVALID_HANDLE_VALUE)
	{	
		dwErr = GetLastError();
		switch (dwErr)
		{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
			case ERROR_INVALID_NAME:
				return FALSE;
			default:
				return TRUE;
		}
	}
	CloseHandle(hFile);
	return TRUE;
}

BOOL IsDirectory(LPWSTR lpwPath)
{
	WIN32_FILE_ATTRIBUTE_DATA FAD = { 0 };
	LPWSTR lpwTmp = new WCHAR[MAX_PATH];
	wcscpy(lpwTmp, lpwPath);
	if ((lpwTmp[0] == '"') && (lpwTmp[wcslen(lpwTmp) - 1] == '"'))
	{
		lpwTmp++;
		lpwTmp[wcslen(lpwTmp) - 1] = '\0';
	}
	if (GetFileAttributesEx(lpwTmp, GetFileExInfoStandard, (LPVOID)&FAD) != 0)
	{
		if ((FAD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			delete[] lpwTmp;
			return TRUE;
		}
	}
	delete[] lpwTmp;
	return FALSE;
}

//Допустимый уровень: от 0 до 1
COLORREF Blend(COLORREF crFrColor, COLORREF crBkColor, const double dblLevel)
{
	long lR1 = GetRValue(crFrColor), lG1 = GetGValue(crFrColor), lB1 = GetBValue(crFrColor);
	long lR2 = GetRValue(crBkColor), lG2 = GetGValue(crBkColor), lB2 = GetBValue(crBkColor);
	long lR3 = 0, lG3 = 0, lB3 = 0;
	lR3 = (long)(lR1 * (1 - dblLevel) + lR2 * dblLevel);
	lG3 = (long)(lG1 * (1 - dblLevel) + lG2 * dblLevel);
	lB3 = (long)(lB1 * (1 - dblLevel) + lB2 * dblLevel);
	CheckBounds(&lR3, 0, 255);
	CheckBounds(&lG3, 0, 255);
	CheckBounds(&lB3, 0, 255);
	return RGB(lR3, lG3, lB3);
}

void CheckBounds(long *Value, long Min, long Max)
{
	if ((*Value >= Min) && (*Value <= Max)) return;
	if (*Value < Min) *Value = Min;
	if (*Value > Max) *Value = Max;
}

BOOL IsNumeric(LPCWSTR lpwText)
{
	DWORD i = 0, dwTextSize = wcslen(lpwText);
	if (dwTextSize == 0) return FALSE;
	for (; i < (dwTextSize - 1); i++);
	{
		if (((lpwText[i] < '0') || (lpwText[i] > '9')) &&
			(lpwText[i] != '.')) return FALSE;
	}
	return TRUE;
}

BOOL IsURL(LPCWSTR lpwText)
{
	if (wcsstr(lpwText, L"http://") == lpwText) return TRUE;
	if (wcsstr(lpwText, L"https://") == lpwText) return TRUE;
	if (wcsstr(lpwText, L"ftp://") == lpwText) return TRUE;
	if (wcsstr(lpwText, L"mms://") == lpwText) return TRUE; //Под вопросом
	if (wcsstr(lpwText, L"file://") == lpwText) return TRUE;
	/* Другие протоколы пока учитывать не будем */
	return FALSE;
}

void ProcessMessages()
{
	MSG Msg;
    if (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

DWORD WinAPIErrMsg(DWORD dwCode, LPCWSTR lpwComment)
{
	WCHAR lpwError[MAX_PATH] = { 0 };
	WCHAR lpwMsg[512] = { 0 };
    DWORD dwResult = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwCode, 0,
		lpwError, MAX_PATH, NULL);
	if (dwResult)
	{
		if (lpwComment)
		{
			wcscpy(lpwMsg, lpwComment);
			wcscat(lpwMsg, L"\n\n");
			wcscat(lpwMsg, lpwError);
		}
		else
		{
			wcscpy(lpwMsg, lpwError);
		}
		MessageBox(NULL, lpwMsg, L"Error", MB_ICONEXCLAMATION);
	}
	return dwResult;
}

DWORD DebugMsgLngA(long Value)
{
	char lpDbgMsg[64] = { 0 };
	_ltoa(Value, lpDbgMsg, 10);
	return MessageBoxA(NULL, lpDbgMsg, "Debug", MB_OKCANCEL |
		MB_DEFBUTTON1 | MB_ICONEXCLAMATION);
}

DWORD DebugMsgLngW(long Value)
{
	WCHAR lpwDbgMsg[64] = { 0 };
	_ltow(Value, lpwDbgMsg, 10);
	return MessageBoxW(NULL, lpwDbgMsg, L"Debug", MB_OKCANCEL |
		MB_DEFBUTTON1 | MB_ICONEXCLAMATION);
}

DWORD DebugMsgStrA(LPSTR String)
{
	return MessageBoxA(NULL, String, "Debug", MB_OKCANCEL |
		MB_DEFBUTTON1 | MB_ICONEXCLAMATION);
}

DWORD DebugMsgStrW(LPWSTR String)
{
	return MessageBoxW(NULL, String, L"Debug", MB_OKCANCEL |
		MB_DEFBUTTON1 | MB_ICONEXCLAMATION);
}