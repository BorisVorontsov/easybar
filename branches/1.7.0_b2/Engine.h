#ifndef ENGINE_H
#define ENGINE_H

#ifndef __DSHOW_INCLUDED__
#include <dshow.h>
#endif

#ifndef _INC_MATH
#include <math.h>
#endif

//--------------------------------------------------------------------------
MIDL_INTERFACE("97f7c4d4-547b-4a5f-8332-536430ad2e4d")
IAMFilterData : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE
		ParseFilterData (BYTE* rgbFilterData, ULONG cb, BYTE** prgbRegFilter2) PURE;
	virtual HRESULT STDMETHODCALLTYPE
		CreateFilterData (REGFILTER2* prf2, BYTE** prgbFilterData, ULONG* pcb) PURE;
};
//--------------------------------------------------------------------------

#define DS_CB_WND_CLASS		L"DSCB_"

#define DS_MEDIAEVENTEX		WM_APP + 0x800
#define E_MAX_ARR_SIZE		1024
#define E_MAX_BA			64
#define E_MAX_BF			128

//Состояния "движка"
typedef enum _ENGINESTATE
{
	E_STATE_STOPPED = 0,
	E_STATE_PAUSED = 1,
	E_STATE_PLAYING = 2
} ENGINESTATE;

//Тип потока (для переключения)
typedef enum _DSSTREAMTYPE
{
	DSST_AUDIO = 0,
	DSST_VIDEO = 1
} DSSTREAMTYPE;

//Категории фильтров DirectShow
typedef enum _DSFCATEGORY
{
	DSF_CATEGORY_AUDIO_INPUT_DEVICE = 0,
	DSF_CATEGORY_AUDIO_COMPRESSOR = 2,
	DSF_CATEGORY_AUDIO_RENDERER = 4,
	DSF_CATEGORY_LEGACY_AM_FILTER = 6,
	DSF_CATEGORY_MIDI_RENDERER = 8,
	DSF_CATEGORY_VIDEO_INPUT_DEVICE = 10,
	DSF_CATEGORY_VIDEO_COMPRESSOR = 12
} DSFCATEGORY;

//Категории DirectX Media Objects
typedef enum _DMOCATEGORY
{
	DMO_CATEGORY_AUDIO_DECODER = 0,
	DMO_CATEGORY_AUDIO_EFFECT = 2,
	DMO_CATEGORY_AUDIO_ENCODER = 4,
	DMO_CATEGORY_VIDEO_DECODER = 6,
	DMO_CATEGORY_VIDEO_EFFECT = 8,
	DMO_CATEGORY_VIDEO_ENCODER = 10,
	DMO_CATEGORY_AUDIO_CAPTURE_EFFECT = 12
} DMOCATEGORY;

//Информация о фильтре DirectShow
typedef struct _DSFILTERINFO
{
	CLSID cCLSID;
	DWORD dwMerit;
	/*
	DWORD dwReserved1; //Кол-во Pin'ов
	void *pReserved2; //Массив PININFO v. 1
	void *pReserved3; //Массив PININFO v. 2
	*/
} DSFILTERINFO, *LPDSFILTERINFO;

//Информация о файле мультимедиа
typedef struct _MEDIACONTENT
{
	WCHAR Author[64];
	WCHAR Title[64];
	WCHAR Rating[64];
	WCHAR Copyright[128];
	WCHAR Description[255];
	WCHAR MoreInfo[255];
} MEDIACONTENT, *LPMEDIACONTENT;

//Safe Release
#define SR(x) if(x) {(x)->Release(); x = NULL;}

class CDirectShow
{
public:
    CDirectShow();
    ~CDirectShow();
	//Базовые функции
	//-----------------------------------------------------------
	int Initialize();
    int Open();
    void Play();
	void Pause();
    void Stop();
	void Close();
	double GetRate();
	void SetRate(double dblRate);
	int IsSeekable();
    __int64 GetLength(BOOL bInMS = TRUE);
	void SetLength(__int64 intNewLen, BOOL bInMS = TRUE);
    __int64 GetPosition(BOOL bInMS = TRUE);
	void SetPosition(__int64 intNewPos,
					 BOOL bInMS = TRUE,
					 BOOL bSeekToKeyFrame = FALSE);
	int GetMute();
	void SetMute(int intValue);
	int GetVolume();
    void SetVolume(int intValue);
	int GetBalance();
	void SetBalance(int intValue);
	ENGINESTATE GetState();
	int IsVideo();
	int CanStep(DWORD dwFrames);
	void FrameStep(DWORD dwFrames);
	int GetOriginalVideoSize(LPSIZE pSZ);
	int GetVideoSize(LPRECT pRC);
	void SetVideoSize(RECT RC);
	HWND GetVideoOwner();
	void SetVideoOwner(HWND hOwner);
	int GetVideoStyles(LPLONG pStyle, LPLONG pExStyle);
	void SetVideoStyles(LONG lStyle, LONG lExStyle);
	int GetVideoVisible();
	void SetVideoVisible(BOOL bVisible);
	int GetFullscreen();
	void SetFullscreen(int intMode);
	int CopyCurrentFrame();
	int SaveCurrentFrame(LPCWSTR lpwFileName);
	//Дополнительные функции
	//-----------------------------------------------------------
	int GetAvailableStreams(DSSTREAMTYPE dStreamType,
	                        LPWSTR *lpwStmArr,
		                    LPDWORD pArrSize,
							DWORD dwBuffSize);
	int SelectStream(DSSTREAMTYPE dStreamType,
		             LPCWSTR lpwStmName);
	BOOL IsStreamSelected(DSSTREAMTYPE dStreamType,
		                  LPCWSTR lpwStmName);

	int GetAudioStreamsCount_E();
	int SelectAudioStream_E(int intStmIndex);
	BOOL IsAudioStreamSelected_E(int intStmIndex);

	int AddFGToROT();
	int GetDSFiltersNames(LPWSTR *lpwDSFilArr,
		                  LPDWORD pArrSize,
						  DWORD dwBuffSize);
	int GetDSFilterInfo(LPCWSTR lpwDSFilName, LPDSFILTERINFO pDSFI);
	int AddDSFilterToFilterGraph(LPCWSTR lpwDSFilName);
	void UpdateDSFiltersArray(DSFCATEGORY dCategory);
	int GetFGFiltersNames(LPWSTR *lpwFGFilArr,
		                  LPDWORD pArrSize,
						  DWORD dwBuffSize);
	int FGFiltersPropertyPages(LPCWSTR lpwFGFilName, BOOL bCheck);
	void UpdateFGFiltersArray();
	int GetDMONames(LPWSTR *lpwDMOArr,
		            LPDWORD pArrSize,
					DWORD dwBuffSize);
	int AddDMOToFilterGraph(LPCWSTR lpwDMOName);
	void UpdateDMOArray(DMOCATEGORY dCategory);
	int GetMediaContent(LPMEDIACONTENT pMC);
    LPWSTR m_lpwFileName;
	LPCWSTR m_lpwAppName;
	HWND m_hAppWnd;
protected:
    //
private:
	void RemoveFGFromROT();
	void DSErrorMsg(HRESULT hr, LPCWSTR lpwEM);
	void FreeMediaType(AM_MEDIA_TYPE *pMT);
	void InitDSCBWnd(BOOL bCreate = TRUE);
	static LRESULT CALLBACK DSCBWndProc(HWND hWnd,
		                                UINT uMsg,
										WPARAM wParam,
										LPARAM lParam);
	HWND m_hDSCBWnd;
	BOOL m_bNoFGError;
	ULONG m_lCounter;
	DWORD m_dwROTRegister;
	ULONG m_lBACount;
	int m_intCurrentBA;
	ULONG m_lDSFilCount;
	ULONG m_lFGFilCount;
	ULONG m_lDMOCount;
	int m_intPrevVol;
	IAMStreamSelect *m_pAMStreamSelect;
	IGraphBuilder *m_pGraphBuilder;
	IMediaControl *m_pMediaControl;
	IMediaSeeking *m_pMediaSeeking;
	IMediaEventEx *m_pMediaEventEx;
	IBasicVideo2 *m_pBasicVideo2;
	IVideoWindow *m_pVideoWindow;
	IVideoFrameStep *m_pVideoFrameStep;
	IBasicAudio *m_pBasicAudio[E_MAX_BA];
	IMoniker *m_pDSFMoniker[E_MAX_ARR_SIZE];
	IBaseFilter *m_pFGBaseFilter[E_MAX_BF];
	LPWSTR m_lpwDMONames[E_MAX_ARR_SIZE];
	CLSID m_cDMOCLSIDs[E_MAX_ARR_SIZE];
	GUID m_gidRecentDMOCat;
};

//intValue: 0 - 100
__inline int PercentsTodB_LogScale(int intValue) {
	return (intValue != 0)?(int)((log10(1.0 * intValue) - 2) * 5000):-10000;
}

__inline int PercentsTodB_LinScale(int intValue) {
	return (intValue != 0)?-10000 + (intValue * 100):-10000;
}

extern CDirectShow *pEngine;

#endif