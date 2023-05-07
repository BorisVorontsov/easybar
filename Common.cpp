//Здесь находятся общие функции, которые могут быть использованы
//не только из основного модуля EasyBar.cpp

#define _SCL_SECURE_NO_WARNINGS

#include <windows.h>
#include <shlobj.h>
//#include <shobjidl.h>
#include <stdio.h>

#include <string>
using namespace std;

#include "common.h"

#pragma comment (lib, "msimg32.lib")

//Рисование текста с эффектом 3D
void Draw3DText(HWND hWnd,
				LPWSTR lpText,
				COLORREF crBkColorOne,
				COLORREF crBkColorTwo,
				DWORD dwBkDirection,
				COLORREF crFontColor,
				COLORREF crShadowColor,
				LONG lShadOffsetX,
				LONG lShadOffsetY)
{
	RECT RC = {};
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
		TRIVERTEX TV[2] = {};
		GRADIENT_RECT GR = {};
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
		DrawText(hDC, lpText, -1, &RC, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
		OffsetRect(&RC, -(lShadOffsetX), -(lShadOffsetY));
	}
	SetTextColor(hDC, crFontColor);
	DrawText(hDC, lpText, -1, &RC, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}

//Устанавливает ListBox'у параметры для горизонтальной прокрутки
void SetLBHorizontalExtent(HWND hListBox)
{
	WCHAR lpText[MAX_PATH] = {};
	HDC hDC;
	HFONT hLBFont, hOldLBFont;
	RECT RCL = {};
	LONG_PTR lLBStyle;
	LONG lMaxTextWidth = 0;
	ULONG i, uItemsCnt;
	hDC = GetDC(hListBox);
	if (!(hLBFont = (HFONT)SendMessage(hListBox, WM_GETFONT, 0, 0)))
		hLBFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hOldLBFont = (HFONT)SelectObject(hDC, hLBFont);
	uItemsCnt = (ULONG)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
	for (i = 0; i < uItemsCnt; i++)
	{
		SendMessage(hListBox, LB_GETTEXT, i, (LPARAM)lpText);
		DrawText(hDC, lpText, -1, &RCL, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
		if (RCL.right > lMaxTextWidth) lMaxTextWidth = RCL.right;
	}
	lLBStyle = GetWindowLongPtr(hListBox, GWL_STYLE);
	if ((lLBStyle & WS_VSCROLL) == WS_VSCROLL)
		lMaxTextWidth += GetSystemMetrics(SM_CXVSCROLL);
	SendMessage(hListBox, LB_SETHORIZONTALEXTENT, lMaxTextWidth, 0);
	SelectObject(hDC, hOldLBFont);
	ReleaseDC(hListBox, hDC);
}

//Перемещение окна в центр экрана (с указанными сдвигами)
void MoveToCenter(HWND hWnd,
				  LONG lXOffset,
				  LONG lYOffset)
{
	RECT RC = {};
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
	RECT RCW = {};
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
				LPWSTR lpPath,
				DWORD dwPathLen,
				BOOL bAddQuotes)
{
	LPWSTR lpBuff = new WCHAR[dwPathLen];
	if (GetModuleFileName(hApp, lpBuff, dwPathLen) == 0)
	{
		delete[] lpBuff;
		return FALSE;
	}
	if (wcschr(lpBuff, ' ') && bAddQuotes)
	{
		WCHAR lpQt[1] = {};
		lpQt[0] = '"';
		wcscpy(lpPath, lpQt);
		wcscat(lpPath, lpBuff);
		wcscat(lpPath, lpQt);
	}
	else wcscpy(lpPath, lpBuff);
	delete[] lpBuff;
	return TRUE;
}

DWORD ReadFileVersion(LPCWSTR lpFileName,
					  LPWSTR lpResult,
					  UINT uResultSize,
					  DWORD dwFlags)
{
	DWORD dwDummy;
	BYTE *bBuffer;
	VS_FIXEDFILEINFO *VS_BUFF;
	DWORD dwBufferLen;
	UINT uVerBufferLen;
	WCHAR lpTmp[8] = {};
	SIZE_T szRLen;
	dwBufferLen = GetFileVersionInfoSize(lpFileName, &dwDummy);
	if (dwBufferLen < 1)
	{
		wcscpy(lpResult, L"None");
		return 0;
	}
	bBuffer = new BYTE[dwBufferLen];
	GetFileVersionInfo(lpFileName, 0, dwBufferLen, &bBuffer[0]);
	VerQueryValue(&bBuffer[0], L"\\", (PVOID *)&VS_BUFF, &uVerBufferLen);
	ZeroMemory(lpResult, uResultSize);
	if ((dwFlags & RFV_MAJOR) == RFV_MAJOR)
	{
		_itow(HIWORD(VS_BUFF->dwProductVersionMS), lpTmp, 10);
		wcscpy(lpResult, lpTmp);
	}
	if ((dwFlags & RFV_MINOR) == RFV_MINOR)
	{
		szRLen = wcslen(lpResult);
		if (szRLen && (lpResult[szRLen - 1] != '\0'))
			wcscat(lpResult, L".");
		_itow(LOWORD(VS_BUFF->dwProductVersionMS), lpTmp, 10);
		wcscat(lpResult, lpTmp);
	}
	if ((dwFlags & RFV_RELEASE) == RFV_RELEASE)
	{
		szRLen = wcslen(lpResult);
		if (szRLen && (lpResult[szRLen - 1] != '\0'))
			wcscat(lpResult, L".");
		_itow(HIWORD(VS_BUFF->dwProductVersionLS), lpTmp, 10);
		wcscat(lpResult, lpTmp);
	}
	if ((dwFlags & RFV_BUILD) == RFV_BUILD)
	{
		szRLen = wcslen(lpResult);
		if (szRLen && (lpResult[szRLen - 1] != '\0'))
			wcscat(lpResult, L".");
		_itow(LOWORD(VS_BUFF->dwProductVersionLS), lpTmp, 10);
		wcscat(lpResult, lpTmp);
	}
	delete[] bBuffer;
	return 1;
}

SIZE_T GetOpenDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCWSTR lpTitle,
					LPWSTR lpFileName,
					SIZE_T szFNSize,
					LPCWSTR lpFilter,
					DWORD dwFilterIndex,
					BOOL bMultiSelect,
					LPCWSTR lpInitialDir)
{
	OPENFILENAME OFN = {};
	OFN.lStructSize = sizeof(OFN);
	OFN.hInstance = hInstance;
	OFN.hwndOwner = hWnd;
	OFN.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES;
	if (bMultiSelect)
		OFN.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	OFN.lpstrTitle = lpTitle;
	OFN.lpstrFile = lpFileName;
	OFN.nMaxFile = (DWORD)szFNSize;
	OFN.lpstrFilter = lpFilter;
	OFN.nFilterIndex = dwFilterIndex;
	OFN.lpstrInitialDir = lpInitialDir;
	if (GetOpenFileName(&OFN))
	{
		return wcslen(lpFileName);
	}
	else return 0;
}

SIZE_T GetSaveDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCWSTR lpTitle,
					LPWSTR lpFileName,
					SIZE_T szFNSize,
					LPCWSTR lpFilter,
					LPDWORD pFilterIndex,
					LPCWSTR lpDefExt,
					LPCWSTR lpInitialDir)
{
	OPENFILENAME OFN = {};
	OFN.lStructSize = sizeof(OFN);
	OFN.hInstance = hInstance;
	OFN.hwndOwner = hWnd;
	OFN.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT |
		OFN_NOREADONLYRETURN | OFN_EXPLORER;
	OFN.lpstrTitle = lpTitle;
	OFN.lpstrFile = lpFileName;
	OFN.nMaxFile = (DWORD)szFNSize;
	OFN.lpstrFilter = lpFilter;
	OFN.nFilterIndex = *pFilterIndex;
	OFN.lpstrDefExt = lpDefExt;
	OFN.lpstrInitialDir = lpInitialDir;
	if (GetSaveFileName(&OFN))
	{
		*pFilterIndex = OFN.nFilterIndex;
		return wcslen(lpFileName);
	}
	else return 0;
}

//Функция разбирает строку фильтра старого формата (Type Description|*.file_ext1;*.file_ext2||) и
//заполняет массив структур COMDLG_FILTERSPEC для новых интерфейсов для работы с общими диалогами
//Вызывающий ответственен за удаление строк и массива структур посредством delete[]
static void IFileDialogCreateFilterSpecs(LPCWSTR lpOldFormatFilter,
								  COMDLG_FILTERSPEC **ppCDFS,
								  LPUINT pFSCnt)
{
	SIZE_T szDelPos = 0, szPrevOff = 0, szNewOff = 0, szTmpSize;
	wstring strFilter, strTmp;
	UINT nFS = 0;

	strFilter = lpOldFormatFilter;

	*pFSCnt = 0;
	//Считаем количество определений по разделителям
	while ((szDelPos = strFilter.find(L"|", ((szDelPos == 0)?szDelPos:++szDelPos))) != -1)
		(*pFSCnt)++;

	//Не считаем последний разделитель и делим на 2 для получения количества пар определений
	--(*pFSCnt) /= 2;

	*ppCDFS = new COMDLG_FILTERSPEC[*pFSCnt];

	do
	{
		//Сначала ищем описание типа(ов)
		szNewOff = strFilter.find(L"|", szNewOff);
		strTmp = strFilter.substr(szPrevOff, szNewOff - szPrevOff);
		szTmpSize = strTmp.size();
		(*ppCDFS)[nFS].pszName = new WCHAR[szTmpSize + 1];
		((LPWSTR)(*ppCDFS)[nFS].pszName)[szTmpSize] = L'\0';
		strTmp.copy((LPWSTR)(*ppCDFS)[nFS].pszName, szTmpSize);
		szPrevOff = ++szNewOff;

		//Затем, ищем маску типа(ов)
		szNewOff = strFilter.find(L"|", szNewOff);
		strTmp = strFilter.substr(szPrevOff, szNewOff - szPrevOff);
		szTmpSize = strTmp.size();
		(*ppCDFS)[nFS].pszSpec = new WCHAR[szTmpSize + 1];
		((LPWSTR)(*ppCDFS)[nFS].pszSpec)[szTmpSize] = L'\0';
		strTmp.copy((LPWSTR)(*ppCDFS)[nFS].pszSpec, szTmpSize);
		szPrevOff = ++szNewOff;
		nFS++;
	} while ((strFilter[szNewOff] != L'|') && (strFilter[szNewOff] != L'\0'));
}

SIZE_T GetOpenDialog_Vista(HINSTANCE hInstance,
						  HWND hWnd,
						  LPCWSTR lpTitle,
						  LPWSTR lpFileName,
						  SIZE_T szFNSize,
						  LPCWSTR lpFilter,
						  DWORD dwFilterIndex,
						  BOOL bMultiSelect,
						  LPCWSTR lpInitialDir)
{
	IFileOpenDialog *pFOD;
	DWORD dwOptions;
	SIZE_T szResultSize = 0;

	HRESULT hResult = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_INPROC_SERVER, __uuidof(IFileOpenDialog),
		(LPVOID*)&pFOD);

	if (SUCCEEDED(hResult))
	{
		dwOptions = FOS_FORCEFILESYSTEM;
		if (bMultiSelect)
			dwOptions |= FOS_ALLOWMULTISELECT;
		pFOD->SetOptions(dwOptions);
		pFOD->SetTitle(lpTitle);
		if (wcslen(lpFileName))
			pFOD->SetFileName(lpFileName);

		//Если фильтр задан, парсим его и передаем в IFileOpenDialog..
		if (lpFilter)
		{
			COMDLG_FILTERSPEC *pCDFS;
			UINT nFSCnt;
			IFileDialogCreateFilterSpecs(lpFilter, &pCDFS, &nFSCnt);

			pFOD->SetFileTypes(nFSCnt, pCDFS);

			for (UINT i = 0; i < nFSCnt; i++) {
				delete[] pCDFS[i].pszName;
				delete[] pCDFS[i].pszSpec;
			}
			delete[] pCDFS;

			pFOD->SetFileTypeIndex(dwFilterIndex);
		}
		if (lpInitialDir)
		{
			IShellItem *pInitDir;
			//SHCreateItemFromParsingName(lpInitialDir, NULL, __uuidof(IShellItem), (LPVOID *)&pInitDir);
			LPITEMIDLIST pIIDL;
			SHParseDisplayName(lpInitialDir, NULL, &pIIDL, 0, NULL);
			SHCreateItemFromIDList(pIIDL, __uuidof(IShellItem), (LPVOID *)&pInitDir);
			pFOD->SetFolder(pInitDir);
		}
		if (SUCCEEDED(pFOD->Show(hWnd)))
		{
			//Если всего один файл, то используем GetResult
			if (!bMultiSelect)
			{
GOD_OneResult:
				IShellItem *pResult;
				LPWSTR lpResult;

				pFOD->GetResult(&pResult);
				pResult->GetDisplayName(SIGDN_FILESYSPATH, &lpResult);
				wcsncpy(lpFileName, lpResult, szFNSize);

				CoTaskMemFree(lpResult);
			}
			else //Иначе, проходимся по всем полученным файлам
			{
				IShellItemArray *pResults;
				ULONG nResultsCnt;
				IEnumShellItems *pEnumResults;
				ULONG nSuccessRetrieved;
				IShellItem *pFileName;
				LPWSTR lpResult;

				pFOD->GetResults(&pResults);
				pResults->GetCount(&nResultsCnt);
				if (nResultsCnt == 1)
					goto GOD_OneResult;

				pResults->EnumItems(&pEnumResults);

				pEnumResults->Next(1, &pFileName, &nSuccessRetrieved);
				do
				{
					pFileName->GetDisplayName(SIGDN_FILESYSPATH, &lpResult);
					//Достаточно ли места в переданном буфере? (+1 для завершающего разделителя)
					if (((wcslen(lpFileName) + wcslen(lpResult)) + 1) > szFNSize)
						break;
					wcscat(lpFileName, lpResult);
					wcscat(lpFileName, L"|");

					CoTaskMemFree(lpResult);
				}
				while (pEnumResults->Next(1, &pFileName, &nSuccessRetrieved) == S_OK);
				wcscat(lpFileName, L"|");
			}

			szResultSize = wcslen(lpFileName);
		}

		if (pFOD)
			pFOD->Release();
	}

	return szResultSize;
}

SIZE_T GetSaveDialog_Vista(HINSTANCE hInstance,
						  HWND hWnd,
						  LPCWSTR lpTitle,
						  LPWSTR lpFileName,
						  SIZE_T szFNSize,
						  LPCWSTR lpFilter,
						  LPDWORD pFilterIndex,
						  LPCWSTR lpDefExt,
						  LPCWSTR lpInitialDir)
{
	IFileSaveDialog *pFSD;
	SIZE_T szResultSize = 0;

	HRESULT hResult = CoCreateInstance(__uuidof(FileSaveDialog), NULL, CLSCTX_INPROC_SERVER, __uuidof(IFileSaveDialog),
		(LPVOID*)&pFSD);

	if (SUCCEEDED(hResult))
	{
		pFSD->SetOptions(FOS_OVERWRITEPROMPT);
		pFSD->SetTitle(lpTitle);
		if (wcslen(lpFileName))
			pFSD->SetFileName(lpFileName);

		if (lpFilter)
		{
			COMDLG_FILTERSPEC *pCDFS;
			UINT nFSCnt;
			IFileDialogCreateFilterSpecs(lpFilter, &pCDFS, &nFSCnt);

			pFSD->SetFileTypes(nFSCnt, pCDFS);

			for (UINT i = 0; i < nFSCnt; i++) {
				delete[] pCDFS[i].pszName;
				delete[] pCDFS[i].pszSpec;
			}
			delete[] pCDFS;

			pFSD->SetFileTypeIndex(*pFilterIndex);
		}

		if (lpDefExt)
			pFSD->SetDefaultExtension(lpDefExt);

		if (lpInitialDir)
		{
			IShellItem *pInitDir;
			//SHCreateItemFromParsingName(lpInitialDir, NULL, __uuidof(IShellItem), (LPVOID *)&pInitDir);
			LPITEMIDLIST pIIDL;
			SHParseDisplayName(lpInitialDir, NULL, &pIIDL, 0, NULL);
			SHCreateItemFromIDList(pIIDL, __uuidof(IShellItem), (LPVOID *)&pInitDir);
			pFSD->SetFolder(pInitDir);
		}

		if (SUCCEEDED(pFSD->Show(hWnd)))
		{

			IShellItem *pResult;
			LPWSTR lpResult;

			pFSD->GetResult(&pResult);
			pResult->GetDisplayName(SIGDN_FILESYSPATH, &lpResult);
			wcsncpy(lpFileName, lpResult, szFNSize);

			CoTaskMemFree(lpResult);

			pFSD->GetFileTypeIndex((LPUINT)pFilterIndex);

			szResultSize = wcslen(lpFileName);
		}

		if (pFSD)
			pFSD->Release();
	}

	return szResultSize;
}

BOOL GetColorDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCOLORREF crColor)
{
	CHOOSECOLOR CC = {};
	ULONG i, lClr = 0;
	COLORREF crCustomColors[16] = {};
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

SIZE_T GetBrowseForFolderDialog(HWND hWnd, LPWSTR lpFolder, LPCWSTR lpTitle, LPCWSTR lpInitialDir)
{
	HRESULT hResult = CoInitialize(NULL);
	if (FAILED(hResult)) return 0;
	SIZE_T szResult = 0;
	BROWSEINFO BI = {};
	LPITEMIDLIST pIIDL;
	BI.hwndOwner = hWnd;
	BI.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	BI.lpszTitle = lpTitle;
	BI.lpfn = BFFCallbackProc;
	BI.lParam = (LPARAM)lpInitialDir;
	pIIDL = SHBrowseForFolder(&BI);
	if (!pIIDL)
	{
		CoUninitialize();
		return szResult;
	}
	if (SHGetPathFromIDList(pIIDL, lpFolder))
	{
		szResult = wcslen(lpFolder);
	}
	CoTaskMemFree(pIIDL);
	if (hResult == S_OK)
		CoUninitialize();
	return szResult;
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

//Получение уникального имени в формате "_имя_раб._стола_-_ID_сессии_"
//Вызывающий ответственнен за удаление строки посредством delete[]
LPWSTR CreateUniqueName()
{
	LPWSTR lpDesktopName = NULL, lpResult;
	WCHAR lpSessionID[32] = {};
	DWORD dwDesktopNameSize;
	SIZE_T szSessionIDSize;
	HDESK hDesktop;
	HANDLE hToken;
	PTOKEN_STATISTICS pTS;
	DWORD dwTSSize;

	hDesktop = GetThreadDesktop(GetCurrentThreadId());
	GetUserObjectInformation(hDesktop, UOI_NAME, lpDesktopName, 0, &dwDesktopNameSize);
	lpDesktopName = new WCHAR[dwDesktopNameSize / sizeof(WCHAR)];
	ZeroMemory(lpDesktopName, dwDesktopNameSize);
	GetUserObjectInformation(hDesktop, UOI_NAME, lpDesktopName, dwDesktopNameSize,
		&dwDesktopNameSize);

	OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	GetTokenInformation(hToken, TokenStatistics, NULL, 0, &dwTSSize);

	pTS = (PTOKEN_STATISTICS)new BYTE[dwTSSize];

	if (GetTokenInformation(hToken, TokenStatistics, (LPVOID)pTS, dwTSSize, &dwTSSize))
	{
		swprintf(lpSessionID, L"-%08x%08x", pTS->AuthenticationId.HighPart,
			pTS->AuthenticationId.LowPart);
	}

	delete[] pTS;

	dwDesktopNameSize /= sizeof(WCHAR);
	szSessionIDSize =  wcslen(lpSessionID);
	lpResult = new WCHAR[dwDesktopNameSize + szSessionIDSize + 1];
	wcscpy(lpResult, lpDesktopName);
	wcscat(lpResult, lpSessionID);
	lpResult[dwDesktopNameSize + szSessionIDSize] = '\0';

	return lpResult;
}

void AdjustPrivilege(LPWSTR lpPrivilege)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES TKP = {};
	TOKEN_PRIVILEGES TKPOLD = {};
	DWORD dwPSLength;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		LookupPrivilegeValue(NULL, lpPrivilege, &TKP.Privileges[0].Luid);
		TKP.PrivilegeCount = 1;
		TKP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, FALSE, &TKP, sizeof(TKPOLD), &TKPOLD, &dwPSLength);
	}
}

BOOL IsFile(LPWSTR lpPath)
{
	LPWSTR lpTmp = new WCHAR[MAX_PATH];
	DWORD dwErr;
	wcscpy(lpTmp, lpPath);
	if ((lpTmp[0] == '"') && (lpTmp[wcslen(lpTmp) - 1] == '"'))
	{
		lpTmp++;
		lpTmp[wcslen(lpTmp) - 1] = '\0';
	}
	HANDLE hFile = CreateFile(lpTmp, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	delete[] lpTmp;
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

BOOL IsDirectory(LPWSTR lpPath)
{
	WIN32_FILE_ATTRIBUTE_DATA FAD = {};
	LPWSTR lpTmp = new WCHAR[MAX_PATH];
	wcscpy(lpTmp, lpPath);
	if ((lpTmp[0] == '"') && (lpTmp[wcslen(lpTmp) - 1] == '"'))
	{
		lpTmp++;
		lpTmp[wcslen(lpTmp) - 1] = '\0';
	}
	if (GetFileAttributesEx(lpTmp, GetFileExInfoStandard, (LPVOID)&FAD) != 0)
	{
		if ((FAD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			delete[] lpTmp;
			return TRUE;
		}
	}
	delete[] lpTmp;
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

BOOL IsNumeric(LPCWSTR lpText)
{
	SIZE_T i = 0, szTextSize = wcslen(lpText);
	if (szTextSize == 0) return FALSE;
	for (; i < (szTextSize - 1); i++)
	{
		if ((lpText[i] == '-') && (i == 0)) continue;
		if (((lpText[i] < '0') || (lpText[i] > '9')) &&
			(lpText[i] != '.')) return FALSE;
	}
	return TRUE;
}

BOOL IsURL(LPCWSTR lpText)
{
	if (wcsstr(lpText, L"http://") == lpText) return TRUE;
	if (wcsstr(lpText, L"https://") == lpText) return TRUE;
	if (wcsstr(lpText, L"ftp://") == lpText) return TRUE;
	if (wcsstr(lpText, L"mms://") == lpText) return TRUE; //Под вопросом
	if (wcsstr(lpText, L"file://") == lpText) return TRUE;
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

DWORD WinAPIErrMsg(DWORD dwCode, LPCWSTR lpComment)
{
	WCHAR lpError[MAX_PATH] = {};
	WCHAR lpMsg[512] = {};
	DWORD dwResult = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwCode, 0,
		lpError, MAX_PATH, NULL);
	if (dwResult)
	{
		if (lpComment)
		{
			wcscpy(lpMsg, lpComment);
			wcscat(lpMsg, L"\n\n");
			wcscat(lpMsg, lpError);
		}
		else
		{
			wcscpy(lpMsg, lpError);
		}
		MessageBox(NULL, lpMsg, L"Error", MB_ICONEXCLAMATION);
	}
	return dwResult;
}

DWORD DebugMsgLngA(long Value)
{
	char lpDbgMsg[64] = {};
	_ltoa(Value, lpDbgMsg, 10);
	return MessageBoxA(NULL, lpDbgMsg, "Debug", MB_OKCANCEL |
		MB_DEFBUTTON1 | MB_ICONEXCLAMATION);
}

DWORD DebugMsgLngW(long Value)
{
	WCHAR lpDbgMsg[64] = {};
	_ltow(Value, lpDbgMsg, 10);
	return MessageBoxW(NULL, lpDbgMsg, L"Debug", MB_OKCANCEL |
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