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

	COLORREF crFontColorOne; //Цвет текста выделенного пункта
	COLORREF crFontColorTwo; //Цвет тени у текста выделенного пункта
	COLORREF crFontColorThree; //Цвет текста обычного пункта
	COLORREF crFontColorFour; //Цвет текста подсвеченного пункта
	COLORREF crBkColorOne; //Цвет левой панели списка (область для изображений)
	COLORREF crBkColorTwo; //Цвет пунктов списка 1
	COLORREF crBkColorThree; //Цвет пунктов списка 2
	COLORREF crSelColorOne; //Первый цвет у градиентного фона выделенного пункта
	COLORREF crSelColorTwo; //Второй цвет у градиентного фона выделенного пункта
	COLORREF crBrColorOne; //Первый цвет рамки выделенного пункта
	COLORREF crBrColorTwo; //Второй цвет рамки выделенного пункта
	COLORREF crTrColor; //Прозрачный цвет, который будет игнорироваться при рисовании изображений
	HFONT hFntStandardItem; //Шрифт для стандартных пунктов
	HFONT hFntHLItem; //Шрифт для подсвеченного пункта
	HBITMAP hBmpMark; //Изображение пометки, размером 16x16 пикселей

	HDC hLPDC; //Левая панель
	HDC hLPIMDC; //Левая панель + неактивная пометка
	HDC hLPAMDC; //Левая панель + активная пометка
	HDC hLPDMDC; //Левая панель + недоступная пометка
	HDC hNI1DC; //Обычный пункт (цвет 1)
	HDC hNI2DC; //Обычный пункт (цвет 2)
	HDC hBDC; //Фон
	HDC hSIDC;	//Выделенный пункт

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