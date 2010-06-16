#ifndef COMMON_H
#define COMMON_H

#define SDA(x)			if(x) {delete[] x; x = NULL;}
#define SDO(x)			if(x) {delete x; x = NULL;}

void Draw3DText(HWND hWnd, LPWSTR lpText, COLORREF crBkColorOne, COLORREF crBkColorTwo,
				DWORD dwBkDirection, COLORREF crFontColor, COLORREF crShadowColor,
				LONG lShadOffsetX, LONG lShadOffsetY);
void SetLBHorizontalExtent(HWND hListBox);
void MoveToCenter(HWND hWnd, LONG lXOffset, LONG lYOffset);
void StickyWindow(HWND hWnd, LPRECT pCRC);
void ShowMousePointer(BOOL bShow);
BOOL GetAppPath(HINSTANCE hApp, LPWSTR lpPath, DWORD dwPathLen, BOOL bAddQuotes);

#define RFV_MAJOR		0x10
#define RFV_MINOR		0x20
#define RFV_RELEASE		0x40
#define RFV_BUILD		0x80

DWORD ReadFileVersion(LPCWSTR lpFileName, LPWSTR lpResult, UINT uResultSize, DWORD dwFlags);

SIZE_T GetOpenDialog(HINSTANCE hInstance, HWND hWnd, LPCWSTR lpTitle, LPWSTR lpFileName,
					DWORD dwFNSize, LPCWSTR lpFilter, DWORD dwFilterIndex, BOOL bMultiSelect = FALSE,
					LPCWSTR lpInitialDir = NULL);
SIZE_T GetSaveDialog(HINSTANCE hInstance, HWND hWnd, LPCWSTR lpTitle, LPWSTR lpFileName,
					DWORD dwFNSize, LPCWSTR lpFilter, LPDWORD pFilterIndex, LPCWSTR lpDefExt,
					LPCWSTR lpInitialDir = NULL);
BOOL GetColorDialog(HINSTANCE hInstance, HWND hWnd, LPCOLORREF crColor);
SIZE_T GetBrowseForFolderDialog(HWND hWnd, LPWSTR lpFolder, LPCWSTR lpTitle, LPCWSTR lpInitDir = NULL);
static int CALLBACK BFFCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
LPWSTR CreateUniqueName();
void AdjustPrivilege(LPWSTR lpPrivilege);
BOOL IsFile(LPWSTR lpPath);
BOOL IsDirectory(LPWSTR lpPath);
COLORREF Blend(COLORREF crFrColor, COLORREF crBkColor, const double dblLevel);
void CheckBounds(long *Value, long Min, long Max);
BOOL IsNumeric(LPCWSTR lpText);
BOOL IsURL(LPCWSTR lpText);
void ProcessMessages();
DWORD WinAPIErrMsg(DWORD dwCode, LPCWSTR lpComment = NULL);

DWORD DebugMsgLngA(long Value);
DWORD DebugMsgLngW(long Value);
DWORD DebugMsgStrA(LPSTR String);
DWORD DebugMsgStrW(LPWSTR String);

#endif