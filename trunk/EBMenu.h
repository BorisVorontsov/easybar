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
	COLORREF crFontColorOne; //Цвет текста выделенного пункта
	COLORREF crFontColorTwo; //Цвет тени у текста выделенного пункта
	COLORREF crFontColorThree; //Цвет текста обычного пункта
	COLORREF crBkColorOne; //Цвет левой панели меню (область для изображений)
	COLORREF crBkColorTwo; //Цвет фона для обычных пунктов меню
	COLORREF crBkColorThree; //Цвет фона для пунктов меню верхнего уровня (панель меню)
	COLORREF crSelColorOne; //Первый увет у градиентного фона выделенного пункта
	COLORREF crSelColorTwo; //Второй увет у градиентного фона выделенного пункта
	COLORREF crBrColorOne; //Первый цвет рамки выделенного пункта
	COLORREF crBrColorTwo; //Второй цвет рамки выделенного пункта
	COLORREF crTrColor; //Прозрачный цвет, который будет игнорироваться при рисовании изображений
	HFONT hFntStandardItem; //Шрифт для обычных пунктов меню
	HFONT hFntDefaultItem; //Шрифт для пунктов меню "по умолчанию"
	HBITMAP hBmpCheck; //Изображение CheckMark, размером 16x16 пикселей
	HBITMAP hBmpRadioCheck; //Изображение RadioCheckMark, размером 16x16 пикселей
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