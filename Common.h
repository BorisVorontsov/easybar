#ifndef COMMON_H
#define COMMON_H

#define SDA(x)			if(x) {delete[] x; x = 0;}
#define SDO(x)			if(x) {delete x; x = 0;}

void Draw3DText(HWND hWnd, LPWSTR lpwText, COLORREF crBkColorOne, COLORREF crBkColorTwo,
				DWORD dwBkDirection, COLORREF crFontColor, COLORREF crShadowColor,
				LONG lShadOffsetX, LONG lShadOffsetY);
void SetLBHorizontalExtent(HWND hListBox);
void MoveToCenter(HWND hWnd, LONG lXOffset, LONG lYOffset);
void StickyWindow(HWND hWnd, LPRECT pCRC);
void ShowMousePointer(BOOL bShow);
BOOL GetAppPath(HINSTANCE hApp, LPWSTR lpwPath, DWORD dwPathLen, BOOL bAddQuotes);

#define RFV_MAJOR		0x10
#define RFV_MINOR		0x20
#define RFV_RELEASE		0x40
#define RFV_BUILD		0x80

DWORD ReadFileVersion(LPCWSTR lpwFileName, LPWSTR lpwResult, UINT uResultSize, DWORD dwFlags);

DWORD GetOpenDialog(HINSTANCE hInstance, HWND hWnd, LPCWSTR lpwTitle, LPWSTR lpwFileName,
					DWORD dwFNSize, LPCWSTR lpwFilter, DWORD dwFilterIndex, BOOL bMultiSelect = FALSE);
DWORD GetSaveDialog(HINSTANCE hInstance, HWND hWnd, LPCWSTR lpwTitle, LPWSTR lpwFileName,
					DWORD dwFNSize, LPCWSTR lpwFilter, LPDWORD pFilterIndex, LPCWSTR lpwDefExt);
BOOL GetColorDialog(HINSTANCE hInstance, HWND hWnd, LPCOLORREF crColor);
DWORD GetBrowseForFolderDialog(HWND hWnd, LPWSTR lpwFolder, LPCWSTR lpwTitle);
void AdjustPrivilege(LPWSTR lpwPrivilege);
BOOL IsFile(LPWSTR lpwPath);
BOOL IsDirectory(LPWSTR lpwPath);
COLORREF Blend(COLORREF crFrColor, COLORREF crBkColor, const double dblLevel);
void CheckBounds(long *Value, long Min, long Max);
BOOL IsNumeric(LPCWSTR lpwText);
BOOL IsURL(LPCWSTR lpwText);

DWORD WinAPIErrMsg(DWORD dwCode, LPCWSTR lpwComment = 0);

DWORD DebugMsgLngA(long Value);
DWORD DebugMsgLngW(long Value);
DWORD DebugMsgStrA(LPSTR String);
DWORD DebugMsgStrW(LPWSTR String);

#endif