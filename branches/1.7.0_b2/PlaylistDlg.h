#ifndef PLAYLISTDLG_H
#define PLAYLISTDLG_H

#define WM_FINDMSG	RegisterWindowMessage(FINDMSGSTRING)

typedef struct _PLITEMDESC
{
	WCHAR lpwTitle[128];
	UINT uDuration;
	LONG_PTR lpReserved;
} PLITEMDESC, *LPPLITEMDESC;

INT_PTR CALLBACK PlaylistDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetActiveItem(LPCWSTR lpwFileName);
void UpdatePlaylistColors();
static void DeleteItem(int intItemIndex);

#endif
