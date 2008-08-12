#ifndef EBMENU_H
#define EBMENU_H

#define EBM_ITEM_DATA_TLMENU					0x0010

#define EBM_METRICS_TLMENU_HEIGHT				0x0010
#define EBM_METRICS_TLMENU_LEFT_TEXT_INDENT		0x0003
#define EBM_METRICS_TLMENU_RIGHT_TEXT_INDENT	0x0003
#define EBM_METRICS_TLMENU_LEFT_SEL_INDENT		0x0000
#define EBM_METRICS_TLMENU_RIGHT_SEL_INDENT		0x0000

#define EBM_MI_IA_INDENT						0x000A

#define EBM_METRICS_STD_INDENT					0x0001
#define EBM_METRICS_LEFT_TEXT_INDENT			0x0008
#define EBM_METRICS_RIGHT_TEXT_INDENT			0x0014
#define EBM_METRICS_SEPARATOR_HEIGHT			0x0007
#define EBM_METRICS_PA_WIDTH					0x0010
#define EBM_METRICS_PA_HEIGHT					0x0010
#define EBM_METRICS_LEFT_SEL_INDENT				EBM_METRICS_STD_INDENT + EBM_METRICS_PA_WIDTH + \
												EBM_METRICS_STD_INDENT + 0x0002
#define EBM_METRICS_RIGHT_SEL_INDENT			0x0000

class CEBMenu
{
public:
	CEBMenu();
	~CEBMenu();
	BOOL InitEBMenu(HWND hMenuOwner, BOOL bPopupMenu = FALSE);
	DWORD MeasureItem(WPARAM wParam, LPARAM lParam);
	DWORD DrawItem(WPARAM wParam, LPARAM lParam);
	HMENU GetCurrentMenu();
	COLORREF crFontColorOne; //���� ������ ����������� ������
	COLORREF crFontColorTwo; //���� ���� � ������ ����������� ������
	COLORREF crFontColorThree; //���� ������ �������� ������
	COLORREF crBkColorOne; //���� ����� ������ ���� (������� ��� �����������)
	COLORREF crBkColorTwo; //���� ���� ��� ������� ������� ����
	COLORREF crBkColorThree; //���� ���� ��� ������� ���� �������� ������ (������ ����)
	COLORREF crSelColorOne; //������ ���� � ������������ ���� ����������� ������
	COLORREF crSelColorTwo; //������ ���� � ������������ ���� ����������� ������
	COLORREF crBrColorOne; //������ ���� ����� ����������� ������
	COLORREF crBrColorTwo; //������ ���� ����� ����������� ������
	COLORREF crTrColor; //���������� ����, ������� ����� �������������� ��� ��������� �����������
	HFONT hFntStandardItem; //����� ��� ������� ������� ����
	HFONT hFntDefaultItem; //����� ��� ������� ���� "�� ���������"
	HBITMAP hBmpCheck; //����������� CheckMark, �������� 16x16 ��������
	HBITMAP hBmpRadioCheck; //����������� RadioCheckMark, �������� 16x16 ��������
protected:
	//
private:
	DWORD CreateODMenu(HMENU hMenu, BOOL bTLMenu = FALSE);
	BOOL DrawCheckMark(HDC hDC, LONG lX, LONG lY, BOOL bDisabled, BOOL bSelected,
		               BOOL bRadioCheck);
	BOOL DrawMenuString(HDC hDC, LPCWSTR lpwString, RECT RCT,
		                BOOL bDisabled, BOOL bSelected, BOOL bDefault, BOOL bTLMenu);
	HWND m_MenuOwner;
};

#endif