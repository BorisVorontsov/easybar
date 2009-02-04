#ifndef COLORSDLG_H
#define COLORSDLG_H

INT_PTR CALLBACK ColorsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void ApplyCurrentColors(HWND hCBO);
static void RestoreOldColors();

#endif