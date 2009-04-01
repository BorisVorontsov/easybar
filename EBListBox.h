#ifndef EBLISTBOX_H
#define EBLISTBOX_H

#define EBLB_METRICS_STD_INDENT				0x0001
#define EBLB_METRICS_LEFT_TEXT_INDENT		0x0003
#define EBLB_METRICS_RIGHT_TEXT_INDENT		0x0003
#define EBLB_METRICS_PA_WIDTH				0x0010
#define EBLB_METRICS_PA_HEIGHT				0x0010
#define EBLB_METRICS_LEFT_SEL_INDENT		EBLB_METRICS_STD_INDENT + EBLB_METRICS_PA_WIDTH + \
											EBLB_METRICS_STD_INDENT + 0x0002
#define EBLB_METRICS_RIGHT_SEL_INDENT		0x0002

#define EBLB_METRICS_COL2_WIDTH				0x0030

__declspec(selectany) UINT EBLB_METRICS_STD_HEIGHT = EBLB_METRICS_STD_INDENT + EBLB_METRICS_PA_HEIGHT +
	EBLB_METRICS_STD_INDENT;

#define LBIS_HIGHLIGHTED					0x00000001

typedef struct _LBITEMDESC
{
	_LBITEMDESC(){dwStyle = 0; pTag = NULL; szTagSize = 0;};
	~_LBITEMDESC(){if (pTag) delete[] pTag;};
	DWORD dwStyle;
	LPBYTE pTag;
	SIZE_T szTagSize;
	LONGLONG llReserved;
} LBITEMDESC, *LPLBITEMDESC;

class CEBListBox
{
public:
	CEBListBox();
	~CEBListBox();
	BOOL InitEBListBox(HWND hListBox);
	DWORD MeasureItem(WPARAM wParam, LPARAM lParam);
	DWORD DrawItem(WPARAM wParam, LPARAM lParam);
	BOOL HighlightItem(UINT uItemIndex);
	BOOL IsItemHighlighted(UINT uItemIndex);
	UINT GetHighlightedItemIndex();
	BOOL SetItemTag(UINT uItemIndex, LPCBYTE pTag, SIZE_T szTagSize);
	SIZE_T GetItemTagSize(UINT uItemIndex);
	BOOL GetItemTag(UINT uItemIndex, LPBYTE pTag);
	BOOL DeleteItemTag(UINT uItemIndex);
	BOOL Sort(BOOL bReverse);
	HWND GetCurrentListBox();
	LONG_PTR GetOldListBoxProc();
	void Refresh();

	COLORREF crFontColorOne; //���� ������ ����������� ������
	COLORREF crFontColorTwo; //���� ���� � ������ ����������� ������
	COLORREF crFontColorThree; //���� ������ �������� ������
	COLORREF crFontColorFour; //���� ������ ������������� ������
	COLORREF crBkColorOne; //���� ����� ������ ������ (������� ��� �����������)
	COLORREF crBkColorTwo; //���� ������� ������ 1
	COLORREF crBkColorThree; //���� ������� ������ 2
	COLORREF crSelColorOne; //������ ���� � ������������ ���� ����������� ������
	COLORREF crSelColorTwo; //������ ���� � ������������ ���� ����������� ������
	COLORREF crBrColorOne; //������ ���� ����� ����������� ������
	COLORREF crBrColorTwo; //������ ���� ����� ����������� ������
	COLORREF crTrColor; //���������� ����, ������� ����� �������������� ��� ��������� �����������
	HFONT hFntStandardItem; //����� ��� ����������� �������
	HFONT hFntHLItem; //����� ��� ������������� ������
	HBITMAP hBmpMark; //����������� �������, �������� 16x16 ��������

	HDC hLPDC; //����� ������
	HDC hLPIMDC; //����� ������ + ���������� �������
	HDC hLPAMDC; //����� ������ + �������� �������
	HDC hLPDMDC; //����� ������ + ����������� �������
	HDC hNI1DC; //������� ����� (���� 1)
	HDC hNI2DC; //������� ����� (���� 2)
	HDC hBDC; //���
	HDC hSIDC;	//���������� �����

	HBITMAP hLPBmp, hOldLPBmp;
	HBITMAP hLPIMBmp, hOldLPIMBmp;
	HBITMAP hLPAMBmp, hOldLPAMBmp;
	HBITMAP hLPDMBmp, hOldLPDMBmp;
	HBITMAP hNI1Bmp, hOldNI1Bmp;
	HBITMAP hNI2Bmp, hOldNI2Bmp;
	HBITMAP hBBmp, hOldBBmp;
	HBITMAP hSIBmp, hOldSIBmp;

protected:
	//
private:
	static int Compare(LPCVOID pArg1, LPCVOID pArg2);
	static int CompareReverse(LPCVOID pArg1, LPCVOID pArg2);
	static LRESULT CALLBACK ListBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam,
		                                LPARAM lParam);
	BOOL DrawMark(HDC hDC, LONG lX, LONG lY, BOOL bDisabled, BOOL bSelected);
	BOOL DrawLBString(HDC hDC, LPCWSTR lpwString, RECT RCT,
                      BOOL bDisabled, BOOL bSelected, BOOL bHighlighted);
	LONG_PTR m_lOldLBProc;
	HWND m_hListBox;
};

#endif