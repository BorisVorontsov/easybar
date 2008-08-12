#ifndef TOOLTIPS_H
#define TOOLTIPS_H

class CToolTips
{
public:
	CToolTips();
	~CToolTips();
	int Initialize();
	int AddToolTip(HWND hCtrl, LPCWSTR lpwText);
	void GetToolTip(HWND hCtrl, LPWSTR lpwText);
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