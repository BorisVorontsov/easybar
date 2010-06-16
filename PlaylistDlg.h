#ifndef PLAYLISTDLG_H
#define PLAYLISTDLG_H

#define WM_FINDMSG			RegisterWindowMessage(FINDMSGSTRING)

#define PLW_MIN_WIDTH		160
#define PLW_MIN_HEIGHT		64

typedef struct _PLITEMDESC
{
	WCHAR lpTitle[128];
	UINT uDuration;
	LONG_PTR lpReserved;
} PLITEMDESC, *LPPLITEMDESC;

INT_PTR CALLBACK PlaylistDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetActiveItem(LPCWSTR lpFileName);
void ModifyActiveItemState();
void UpdatePlaylistColors();
static void DeleteItem(int intItemIndex);

#endif
