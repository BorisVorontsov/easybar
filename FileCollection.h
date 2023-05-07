#ifndef FILECOLLECTION_H
#define FILECOLLECTION_H

#ifndef _INC_TIME
#include <time.h>
#endif

//Максимальное количество файлов в программе
#define FC_MAX_FILES			1024

//Флаги для функций
#define FCF_FIRST				0x00000000
#define FCF_FORWARD				0x00000001
#define FCF_BACKWARD			0x00000002
#define FCF_RANDOM				0x00000004
#define FCF_RECENT				0x00000008
#define FCF_BYINDEX				0x00000010
#define FCF_BYFILENAME			0x00000020

//Класс шлет уведомления (события) окну через это сообщение
#define WM_FCNOTIFICATION		WM_USER + 0x00000300

//События
#define FCN_FILEADDED			0x00000001 //WM_FCNOTIFICATION, HIWORD(wParam): N_CODE, lParam: File_Name
#define FCN_FILEDELETED			0x00000002 //WM_FCNOTIFICATION, HIWORD(wParam): N_CODE, lParam: Index
#define FCN_CURFILECHANGED		0x00000003 //WM_FCNOTIFICATION, HIWORD(wParam): N_CODE, lParam: File_Name
#define FCN_COLCLEARED			0x00000004 //WM_FCNOTIFICATION, HIWORD(wParam): N_CODE
#define FCN_FILEUDCHANGED		0x00000005 //WM_FCNOTIFICATION, HIWORD(wParam): N_CODE, lParam: Index

typedef struct _FCSTRUCT
{
	LPWSTR lpPath;
	DWORD dwRndIndex;
	LONG_PTR lpUserData;
} FCSTRUCT, *LPFCSTRUCT;

class CFileCollection
{
public:
	CFileCollection();
	~CFileCollection();
	int FileCount();
	int AppendFile(LPWSTR lpFileName, BOOL bSendNfn = TRUE);
	int AppendFile(LPWSTR lpFileName, LONG_PTR lpUD, BOOL bSendNfn = TRUE);
	int GetFile(LPWSTR lpFileName, DWORD dwIndex, DWORD dwFlag);
	int GetFileIndex(LPCWSTR lpFileName, DWORD dwFlag);
	int NextFile(LPWSTR lpFileName, DWORD dwIndex, DWORD dwFlag);
	int MoveFile(LPCWSTR lpFileName, DWORD dwIndex, DWORD dwNewIndex, DWORD dwFlag);
	int DeleteFile(LPCWSTR lpFileName, DWORD dwIndex, DWORD dwFlag, BOOL bSendNfn = TRUE);
	void Clear(BOOL bSendNfn = TRUE);
	int IsFileAvailable(DWORD dwFlag);
	int SetRecentFile(LPCWSTR lpFileName, BOOL bSendNfn = TRUE);
	int SetUserData(LPCWSTR lpFileName, DWORD dwIndex, DWORD dwFlag, LONG_PTR lpUD, BOOL bSendNfn = TRUE);
	int GetUserData(LPCWSTR lpFileName, DWORD dwIndex, DWORD dwFlag, LONG_PTR& lpUD);
	void SetCallbackWnd(HWND hWnd);
	HWND GetCallbackWnd();
protected:
	//
private:
	void SetCurrentFile(LPCWSTR lpPath, BOOL bSendNfn = TRUE);
	LPFCSTRUCT m_pFileCollection[FC_MAX_FILES];
	LPWSTR m_lpCurrentFile;
	HWND m_hCBWnd;
};

extern CFileCollection *pFileCollection;

__inline void Randomize(){
	srand((unsigned int)time(0));
}

__inline int Random(int val){
	return (val)?(rand()%(val)):0;
}

#endif
