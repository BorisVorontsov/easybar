#ifndef VIDEOMODE_H
#define VIDEOMODE_H

typedef struct _VIDEOMODE
{
	DWORD dwPelsWidth;
	DWORD dwPelsHeight;
	DWORD dwBitsPerPel;
	DWORD dwDisplayFrequency;
} VIDEOMODE, *LPVIDEOMODE;

#define CVMF_UPDATEREGISTRY		0x00000010
#define CVMF_TEST				0x00000020

#define VM_ERROR				0xFA64

class CVideoMode
{
public:
	CVideoMode();
	~CVideoMode();
	DWORD EnumVideoModes(LPVIDEOMODE *pVMArr) const;
	void GetCurrentVideoMode(LPVIDEOMODE pVM) const;
	DWORD GetVideoModeIndex(VIDEOMODE VM) const;
	BOOL ChangeVideoMode(VIDEOMODE VM, DWORD dwFlag);
	void RestoreOriginalVideoMode();
protected:
	//
private:
	//
};

extern CVideoMode *pVideoMode;

#endif