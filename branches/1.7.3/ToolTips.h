#ifndef TOOLTIPS_H
#define TOOLTIPS_H

class CToolTips
{
public:
	CToolTips();
	~CToolTips();
	int Initialize();
	int AddToolTip(HWND hCtrl, LPCWSTR lpText);
	void GetToolTip(HWND hCtrl, LPWSTR lpText);
	int RemoveToolTip(HWND hCtrl);
	int Destroy();
	HINSTANCE m_hInstance;
	HWND m_hOwner;
protected:
	//
private:
	HWND m_hToolTipWnd;
};

#endif