#ifndef VIDEODLG_H
#define VIDEODLG_H

typedef struct _VWDATA
{
	DWORD dwVWPosFlag; 
	RECT rcNormalPos;
	DWORD dwSMPTimeout;
	HWND hPlayerVW;
} VWDATA, *LPVWDATA;

#define VWPF_NORMAL				0x00000010
#define VWPF_FULLSCREEN			0x00000020

#define VWM_SAVENORMALSTATE		WM_USER + 0x00000800 //wParam: 0, lParam: 0, return: *
#define VWM_CHANGESTATE			WM_USER + 0x00000810 //wParam: VWS_%, lParam: 0, return: *

#define VW_MIN_WIDTH			0x00000040
#define VW_MIN_HEIGHT			0x00000040

#define VWS_NORMAL				VWPF_NORMAL
#define VWS_FULLSCREEN			VWPF_FULLSCREEN

INT_PTR CALLBACK VideoDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ScaleVideoWindow(HWND hWnd, DWORD dwZoomIndex, LPRECT pVWRC = 0);

#endif