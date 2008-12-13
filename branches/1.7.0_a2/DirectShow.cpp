//Воспроизведение файлов посредством DirectShow

#include <dshow.h>
#include <dmoreg.h>
#include <dmodshow.h>
#include <stdio.h>
#include <objbase.h>
#include <initguid.h>
#include <qnetwork.h>

#include "engine.h"

#pragma comment (lib, "strmiids.lib")
#pragma comment (lib, "quartz.lib")
#pragma comment (lib, "msdmo.lib")
#pragma comment (lib, "dmoguids.lib")

//Конструктор по умолчанию
//Здесь происходит инициализация всех внутренних и внешних
//переменных класса
CDirectShow::CDirectShow()
{
	//Инициализация переменных класса
	//------------------------------------------------
    m_lpwFileName = NULL;
	m_lpwAppName = NULL;
	m_hAppWnd = NULL;
	m_hDSCBWnd = NULL;
	m_bNoFGError = FALSE;
	m_lCounter = 0;
	m_dwROTRegister = 0;
	m_lDSFilCount = 0;
	m_lFGFilCount = 0;
	m_lDMOCount = 0;
	m_pGraphBuilder = NULL;
	m_pMediaControl = NULL;
	m_pMediaSeeking = NULL;
	m_pMediaEventEx = NULL;
	m_pBasicAudio = NULL;
	m_pBasicVideo2 = NULL;
	m_pVideoWindow = NULL;
	m_pVideoFrameStep = NULL;
	//Инициализация массивов
	//------------------------------------------------
	for (m_lCounter = 0; m_lCounter < E_MAX_ARR_SIZE; m_lCounter++)
	{
		m_pDSFMoniker[m_lCounter] = NULL;
		m_pFGBaseFilter[m_lCounter] = NULL;
		m_lpwDMONames[m_lCounter] = NULL;
		ZeroMemory(&m_cDMOCLSIDs[m_lCounter], sizeof(m_cDMOCLSIDs[m_lCounter]));
	}
	ZeroMemory(&m_gidRecentDMOCat, sizeof(m_gidRecentDMOCat));
	//Инициализация окна для обработки сообщений от IMediaEventEx и IVideoWindow
	//------------------------------------------------
	InitDSCBWnd();
	//Инициализация COM
	//------------------------------------------------
    CoInitialize(NULL);
}

//Деструктор
CDirectShow::~CDirectShow()
{
	//Деинициализация окна
	//------------------------------------------------
	InitDSCBWnd(FALSE);
	//Деинициализация COM
	//------------------------------------------------
	CoUninitialize();
}

//Инициализация/деинициализация окна для обработки сообщений от IMediaEventEx и IVideoWindow
void CDirectShow::InitDSCBWnd(BOOL bCreate)
{
	if (bCreate)
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		WNDCLASSEX WCEX = { 0 };
		WCEX.cbSize = sizeof(WNDCLASSEX); 
		WCEX.lpfnWndProc = (WNDPROC)DSCBWndProc;
		WCEX.hInstance = hInstance;
		WCEX.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		WCEX.lpszClassName = DS_CB_WND_CLASS;
		RegisterClassEx(&WCEX);
		m_hDSCBWnd = CreateWindow(WCEX.lpszClassName, NULL, WS_POPUP,
			16, 16, 32, 32, NULL, NULL, hInstance, (LPVOID)this);
	}
	else DestroyWindow(m_hDSCBWnd);
}

LRESULT CALLBACK CDirectShow::DSCBWndProc(HWND hWnd,
										  UINT uMsg,
										  WPARAM wParam,
										  LPARAM lParam)
{
	CDirectShow *pThis = NULL;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
		pThis = (CDirectShow *)pCS->lpCreateParams;
		SetProp(hWnd, L"_this_", (HANDLE)pThis);
	}
	else
	{
		pThis = (CDirectShow *)GetProp(hWnd, L"_this_");
	}
	switch (uMsg)
	{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			PostMessage(pThis->GetVideoOwner(), uMsg, wParam, lParam);
			break;
		case DS_MEDIAEVENTEX:
			if (!pThis->m_pMediaEventEx) break;
			long lEvent, lParam1, lParam2;
			pThis->m_pMediaEventEx->GetEvent(&lEvent, &lParam1, &lParam2, 0);
			switch (lEvent)
			{
				case EC_DISPLAY_CHANGED:
					//
					break;
				case EC_VIDEO_SIZE_CHANGED:
					//
					break;
				case EC_USERABORT:
					//
					break;
				case EC_WINDOW_DESTROYED:
					//
					break;
				case EC_COMPLETE:
					//
					break;
			}
			pThis->m_pMediaEventEx->FreeEventParams(lEvent, lParam1, lParam2);
			break;
		case WM_DESTROY:
			RemoveProp(hWnd, L"_this_");
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

//Инициализация объекта 'filter graph manager'. Этот метод может помочь при добавлении дополнительных
//фильтров (DMO) в граф, так как добавление возможно только до рендеринга графа.
//Если DMO добавлять не требуется, этот метод можно не вызывать, его вызовет метод Open() автоматически.
int CDirectShow::Initialize()
{
	if (FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
		IID_IGraphBuilder, (LPVOID *)&m_pGraphBuilder)))
	{
		//Не удалось создать 'filter graph manager'... Возможно, DirectShow не установлен?
		MessageBox(m_hAppWnd, L"Failed to create an instance of the 'DirectShow filter graph manager'!",
			(m_lpwAppName)?m_lpwAppName:L"Error", MB_ICONSTOP);
		m_bNoFGError = TRUE;
		return -1;
	}
	return 0;
}

//В случае успеха функция должна вернуть нуль
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::Open()
{
	if (!m_pGraphBuilder)
	{
		if (!m_bNoFGError)
		{
			if (Initialize() < 0) return -1;
		} else return -1;
	}
	HRESULT hRRF = m_pGraphBuilder->RenderFile(m_lpwFileName, NULL);
	switch (hRRF)
	{
		case S_OK:
		case VFW_S_AUDIO_NOT_RENDERED:
		case VFW_S_DUPLICATE_NAME:
		case VFW_S_PARTIAL_RENDER:
		case VFW_S_VIDEO_NOT_RENDERED:
			//Не реагируем на эти значения
			break;
		default:
			//Не получилось создать граф на базе данного файла
			//Вероятно, в системе не хватает кодека(ов)
			DSErrorMsg(hRRF, NULL);
			return -1;
	}
    m_pGraphBuilder->QueryInterface(IID_IMediaControl, (LPVOID *)&m_pMediaControl);
	m_pGraphBuilder->QueryInterface(IID_IMediaSeeking, (LPVOID *)&m_pMediaSeeking);
    m_pGraphBuilder->QueryInterface(IID_IMediaEventEx, (LPVOID *)&m_pMediaEventEx);
	m_pGraphBuilder->QueryInterface(IID_IBasicAudio, (LPVOID *)&m_pBasicAudio);
	m_intPrevVol = 1;
	m_pGraphBuilder->QueryInterface(IID_IBasicVideo, (LPVOID *)&m_pBasicVideo2);
	m_pGraphBuilder->QueryInterface(IID_IVideoWindow, (LPVOID *)&m_pVideoWindow);
	m_pGraphBuilder->QueryInterface(IID_IVideoFrameStep, (LPVOID *)&m_pVideoFrameStep);
	UpdateFGFiltersArray();
	if (m_pMediaSeeking)
		m_pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
	if (m_pVideoWindow)
	{
		m_pVideoWindow->put_AutoShow(OAFALSE);
		m_pVideoWindow->put_MessageDrain((OAHWND)m_hDSCBWnd);
	}
	if (m_pMediaEventEx)
		m_pMediaEventEx->SetNotifyWindow((OAHWND)m_hDSCBWnd, DS_MEDIAEVENTEX, 0);
	return 0;
}

void CDirectShow::Play()
{
	if (m_pMediaControl)
		m_pMediaControl->Run();
}

void CDirectShow::Pause()
{
	if (m_pMediaControl)
		m_pMediaControl->Pause();
}

void CDirectShow::Stop()
{
	if (m_pMediaControl)
		m_pMediaControl->Stop();
}

//Здесь происходит сброс всех параметров и освобождение интерфейсов
void CDirectShow::Close()
{
	if (m_pMediaEventEx)
		m_pMediaEventEx->SetNotifyWindow(NULL, DS_MEDIAEVENTEX, 0);
	if (m_pVideoWindow)
		m_pVideoWindow->put_Visible(OAFALSE);
	for (m_lCounter = 0; m_lCounter < E_MAX_ARR_SIZE; m_lCounter++)
		SR(m_pFGBaseFilter[m_lCounter]);
	if (m_dwROTRegister)
		RemoveFGFromROT();
    SR(m_pMediaControl);
	SR(m_pMediaSeeking);
    SR(m_pMediaEventEx);
	SR(m_pBasicAudio);
	SR(m_pBasicVideo2);
	SR(m_pVideoWindow);
	SR(m_pVideoFrameStep);
    SR(m_pGraphBuilder);
}

double CDirectShow::GetRate()
{
	if (m_pMediaSeeking)
	{
		double dblTmp;
		m_pMediaSeeking->GetRate(&dblTmp);
		return dblTmp;
	} else return 0;
}

void CDirectShow::SetRate(double dblRate)
{
	if (m_pMediaSeeking)
	{
		if (dblRate > 0 && dblRate <= 2)
			m_pMediaSeeking->SetRate(dblRate);
	}
}

int CDirectShow::IsSeekable()
{
	if (m_pMediaSeeking)
	{
		__int64 intTmp;
		m_pMediaSeeking->GetDuration(&intTmp);
		if (intTmp > 0)
		{
			DWORD dwCaps = AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanGetDuration;
			if (SUCCEEDED(m_pMediaSeeking->CheckCapabilities(&dwCaps))) return 1; else return 0;
		}
		else return 0;
	}
	else return 0;
}

__int64 CDirectShow::GetPosition(BOOL bInMS)
{
	if (m_pMediaSeeking)
	{
		__int64 intRes;
		m_pMediaSeeking->GetCurrentPosition(&intRes);
		return (bInMS)?(intRes / 10000):intRes;
	} else return 0;
}

void CDirectShow::SetPosition(__int64 intNewPos, BOOL bInMS, BOOL bSeekToKeyFrame)
{
	if (m_pMediaSeeking)
	{
		__int64 intPos = intNewPos;
		if (bInMS) intPos *= 10000;
		DWORD dwCurrentFlags = AM_SEEKING_AbsolutePositioning;
		if (bSeekToKeyFrame)
		{
			dwCurrentFlags |= AM_SEEKING_SeekToKeyFrame;
		}
		m_pMediaSeeking->SetPositions(&intPos, dwCurrentFlags, NULL,
			AM_SEEKING_NoPositioning);
	}
}

__int64 CDirectShow::GetLength(BOOL bInMS)
{
	if (m_pMediaSeeking)
	{
		__int64 intRes;
		m_pMediaSeeking->GetDuration(&intRes);
		return (bInMS)?(intRes / 10000):intRes;
	} else return 0;
}

void CDirectShow::SetLength(__int64 intNewLen, BOOL bInMS)
{
	if (m_pMediaSeeking)
	{
		__int64 intLen = intNewLen;
		if (bInMS) intLen *= 10000;
		m_pMediaSeeking->SetPositions(NULL, AM_SEEKING_NoPositioning, &intLen,
			AM_SEEKING_AbsolutePositioning);
	}
}

int CDirectShow::GetMute()
{
	if (m_pBasicAudio)
	{
		return (m_intPrevVol == 1)?0:1;
	} else return 0;
}

void CDirectShow::SetMute(int intValue)
{
	if (m_pBasicAudio)
	{
		if (intValue)
		{
			if (m_intPrevVol == 1)
			{
				m_pBasicAudio->get_Volume((long *)&m_intPrevVol);
				m_pBasicAudio->put_Volume(-10000);
			}
		}
		else
		{
			if (m_intPrevVol < 1)
			{
				m_pBasicAudio->put_Volume((long)m_intPrevVol);
				m_intPrevVol = 1;
			}
		}
	}
}

int CDirectShow::GetVolume()
{
	if (m_pBasicAudio)
	{
		if (m_intPrevVol == 1)
		{
			long lTmp;
			m_pBasicAudio->get_Volume(&lTmp);
			return (int)lTmp;
		}
		else
		{
			return m_intPrevVol;
		}
	} else return 0;
}

//От -10000 (-100 dB) до 0 (0 dB)
void CDirectShow::SetVolume(int intValue)
{
	if (m_pBasicAudio)
	{
		if (m_intPrevVol == 1)
		{
			m_pBasicAudio->put_Volume((long)intValue);
		} else m_intPrevVol = intValue;
	}
}

int CDirectShow::GetBalance()
{
	if (m_pBasicAudio)
	{
		long lTmp;
		m_pBasicAudio->get_Balance(&lTmp);
		return (int)lTmp;
	} else return 0;
}

//От -10000 (L) до 10000 (R)
void CDirectShow::SetBalance(int intValue)
{
	if (m_pBasicAudio)
		m_pBasicAudio->put_Balance((long)intValue);
}

//Возвращает состояние воспроизведения
ENGINESTATE CDirectShow::GetState()
{
	if (m_pMediaControl)
	{
		OAFilterState fState;
		m_pMediaControl->GetState(10, &fState);
		switch (fState)
		{
			case 2:
				return E_STATE_PLAYING;
			case 1:
				return E_STATE_PAUSED;
			case 0:
			default:
				return E_STATE_STOPPED;
		}
	} else return E_STATE_STOPPED;
}

int CDirectShow::IsVideo()
{
	if (m_pVideoWindow)
	{
		long lTmp = 0;
		if (SUCCEEDED(m_pVideoWindow->get_Visible(&lTmp))) return 1; else return 0;
	}
	else return 0;
}

int CDirectShow::CanStep(DWORD dwFrames)
{
	if (m_pVideoFrameStep)
	{
		if (SUCCEEDED(m_pVideoFrameStep->CanStep(dwFrames, NULL)))
			return 1; else return 0;
	} else return 0;
}

void CDirectShow::FrameStep(DWORD dwFrames)
{
	if (m_pVideoFrameStep)
	{
		if (m_pMediaControl)
		{
			OAFilterState fTmp;
			m_pMediaControl->GetState(10, &fTmp);
			if (fTmp != 1)
				m_pMediaControl->Pause();
		}
		m_pVideoFrameStep->Step(dwFrames, NULL);
	}
}

int CDirectShow::GetOriginalVideoSize(LPSIZE pSZ)
{
	if (m_pBasicVideo2)
	{
		m_pBasicVideo2->GetVideoSize(&pSZ->cx, &pSZ->cy);
		return 1;
	} else return 0;
}

int CDirectShow::GetVideoSize(LPRECT pRC)
{
	if (m_pVideoWindow)
	{
		m_pVideoWindow->GetWindowPosition(&pRC->left, &pRC->top, &pRC->right, &pRC->bottom);
		return 1;
	} else return 0;
}

void CDirectShow::SetVideoSize(RECT RC)
{
	if (m_pVideoWindow)
		m_pVideoWindow->SetWindowPosition(RC.left, RC.top, RC.right, RC.bottom);
}

HWND CDirectShow::GetVideoOwner()
{
	if (m_pVideoWindow)
	{
		OAHWND hOwner;
		m_pVideoWindow->get_Owner(&hOwner);
		return (HWND)hOwner;
	} else return 0;
}

void CDirectShow::SetVideoOwner(HWND hOwner)
{
	if (m_pVideoWindow)
		m_pVideoWindow->put_Owner((OAHWND)hOwner);
}

int CDirectShow::GetVideoStyles(LPLONG pStyle, LPLONG pExStyle)
{
	if (m_pVideoWindow)
	{
		if (pStyle) m_pVideoWindow->get_WindowStyle(pStyle);
		if (pExStyle) m_pVideoWindow->get_WindowStyleEx(pExStyle);
		return 1;
	} else return 0;
}

void CDirectShow::SetVideoStyles(LONG lStyle, LONG lExStyle)
{
	if (m_pVideoWindow)
	{
		if (lStyle) m_pVideoWindow->put_WindowStyle(lStyle);
		if (lExStyle) m_pVideoWindow->put_WindowStyleEx(lExStyle);
	}
}

int CDirectShow::GetVideoVisible()
{
	if (m_pVideoWindow)
	{
		long lVisible;
		m_pVideoWindow->get_Visible(&lVisible);
		return (lVisible == OATRUE)?1:0;
	} else return 0;
}

void CDirectShow::SetVideoVisible(BOOL bVisible)
{
	if (m_pVideoWindow)
		m_pVideoWindow->put_Visible((bVisible)?OATRUE:OAFALSE);
}

int CDirectShow::GetFullscreen()
{
	if (m_pVideoWindow)
	{
		long bFS;
		m_pVideoWindow->get_FullScreenMode(&bFS);
		return (bFS == OATRUE)?1:0;
	} else return 0;
}

void CDirectShow::SetFullscreen(int intMode)
{
	if (m_pVideoWindow)
		m_pVideoWindow->put_FullScreenMode((intMode)?OATRUE:OAFALSE);
}

//Копирует текущий кадр в буфер обмена
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::CopyCurrentFrame()
{
	if (m_pBasicVideo2)
	{
		LPBYTE pImage;
		long lArrSize;
		OAFilterState fTmp;
		if (m_pMediaControl)
		{
			m_pMediaControl->GetState(10, &fTmp);
			if (fTmp != 1)
				m_pMediaControl->Pause();
		}
		if (FAILED(m_pBasicVideo2->GetCurrentImage(&lArrSize, NULL))) return -1;
		pImage = new BYTE[lArrSize];
		m_pBasicVideo2->GetCurrentImage(&lArrSize, (long *)pImage);
		OpenClipboard(NULL);
		SetClipboardData(CF_DIB, pImage);
		CloseClipboard();
		delete[] pImage;
		if (m_pMediaControl)
			if (fTmp == 2) m_pMediaControl->Run();
		return 0;
	} else return -1;
}

//Сохраняет текущий кадр в файл (Windows Bitmap)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::SaveCurrentFrame(LPCWSTR lpwFileName)
{
	if (m_pBasicVideo2)
	{
		HANDLE hFile;
		BITMAPFILEHEADER BFH = { 0 };
		LPBYTE pImage, pFileData;
		DWORD dwWritten;
		long lArrSize;
		OAFilterState fTmp;
		if (m_pMediaControl)
		{
			m_pMediaControl->GetState(10, &fTmp);
			if (fTmp != 1)
				m_pMediaControl->Pause();
		}
		if (FAILED(m_pBasicVideo2->GetCurrentImage(&lArrSize, NULL))) return -1;
		pImage = new BYTE[lArrSize];
		m_pBasicVideo2->GetCurrentImage(&lArrSize, (long *)pImage);
		hFile = CreateFile(lpwFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			BFH.bfType = 0x4D42; //0x42 -- 'B', 0x4D -- 'M'
			BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
			BFH.bfSize = sizeof(BITMAPFILEHEADER) + lArrSize;
			pFileData = new BYTE[BFH.bfSize];
			CopyMemory(pFileData, &BFH, sizeof(BITMAPFILEHEADER));
			CopyMemory(pFileData + sizeof(BITMAPFILEHEADER), pImage, lArrSize);
			WriteFile(hFile, pFileData, BFH.bfSize, &dwWritten, NULL);
			CloseHandle(hFile);
			delete[] pFileData;
		}
		delete[] pImage;
		if (m_pMediaControl)
			if (fTmp == 2) m_pMediaControl->Run();
		return 0;
	} else return -1;
}

//Добавляет текущий Filter Graph в ROT (Running Object Table)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::AddFGToROT()
{
	if (m_pGraphBuilder)
	{
		IMoniker *pMoniker = NULL;
		IRunningObjectTable *pRunningObjectTable = NULL;
		if (FAILED(GetRunningObjectTable(0, &pRunningObjectTable)))
		{
			return -1;
		}
		WCHAR lpwItem[256] = { 0 };
		swprintf(lpwItem, L"FilterGraph %08p pid %08x", (LPDWORD)m_pGraphBuilder, GetCurrentProcessId());
		if (FAILED(CreateItemMoniker(L"!", lpwItem, &pMoniker))) return -1;
		pRunningObjectTable->Register(0, m_pGraphBuilder, pMoniker, &m_dwROTRegister);
		pMoniker->Release();
		pRunningObjectTable->Release();
		return 0;
	}
	else return -1;
}

//Возвращает массив имен зарегистрированных в системе фильтров DirectShow (определенной категории)
//Что бы узнать требуемую размерность массива, достаточно вызвать функцию
//с lpwDSFilArr равным NULL - значение вернется в pArrSize.
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetDSFiltersNames(LPWSTR *lpwDSFilArr, LPDWORD pArrSize, DWORD dwBuffSize)
{
	if (!m_lDSFilCount) return -1;
	if (lpwDSFilArr)
	{
		IPropertyBag *pPropBag = NULL;
		VARIANT varDSFName = { 0 };
		varDSFName.vt = VT_BSTR;
		for (m_lCounter = 0; m_lCounter < m_lDSFilCount; m_lCounter++)
		{
			if (m_pDSFMoniker[m_lCounter] == NULL) return -1;
			m_pDSFMoniker[m_lCounter]->BindToStorage(NULL, NULL, IID_IPropertyBag, (LPVOID *)&pPropBag);
			if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &varDSFName, NULL)))
			{
				wcscpy(lpwDSFilArr[m_lCounter], varDSFName.bstrVal);
				SysFreeString(varDSFName.bstrVal);
			}
			else
			{
				wcscpy(lpwDSFilArr[m_lCounter], L"<untitled>");
			}
			pPropBag->Release();
		}
		return 0;
	}
	else
	{
		*pArrSize = m_lDSFilCount;
		return 0;
	}
}

//Возвращает информацию о фильтре DirectShow
//Перед вызовом этой функции необходимо вызвать UpdateDSFiltersArray(...) (если она не вызывалась)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetDSFilterInfo(LPCWSTR lpwDSFilName, DSFILTERINFO *pDSFI)
{
	if (!m_pGraphBuilder) return -1;
	if (!m_lDSFilCount) return -1;
	IPropertyBag *pPropBag = NULL;
	VARIANT varDSFName = { 0 }, varDSFCLSID = { 0 }, varDSFData = { 0 };
	varDSFName.vt = VT_BSTR;
	varDSFCLSID.vt = VT_BSTR;
	varDSFData.vt = (VT_UI1 | VT_ARRAY);
	for (m_lCounter = 0; m_lCounter < m_lDSFilCount; m_lCounter++)
	{
		m_pDSFMoniker[m_lCounter]->BindToStorage(NULL, NULL, IID_IPropertyBag, (LPVOID *)&pPropBag);
		if (FAILED(pPropBag->Read(L"FriendlyName", &varDSFName, NULL)))
		{
			if (m_lCounter >= (m_lDSFilCount - 1))
			{
				pPropBag->Release();
				return -1;
			} else continue;
		}
		if (_wcsicmp(lpwDSFilName, varDSFName.bstrVal) == 0)
		{
			if(SUCCEEDED(pPropBag->Read(L"CLSID", &varDSFCLSID, NULL)))
			{
				CLSIDFromString(varDSFCLSID.bstrVal, &pDSFI->cCLSID);
				SysFreeString(varDSFCLSID.bstrVal);
			}
			else
			{
				ZeroMemory(&pDSFI->cCLSID, sizeof(pDSFI->cCLSID));
			}
			if(SUCCEEDED(pPropBag->Read(L"FilterData", &varDSFData, NULL)))
			{
				BSTR *bstrData = NULL;
				LPBYTE pData = NULL;
				IAMFilterData *pFilterData = NULL;
				SafeArrayAccessData(varDSFData.parray, (LPVOID *)&bstrData);
				CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC, __uuidof(IAMFilterData),
					(LPVOID *)&pFilterData);
				if (SUCCEEDED(pFilterData->ParseFilterData((LPBYTE)bstrData,
					varDSFData.parray->cbElements * varDSFData.parray->rgsabound[0].cElements, &pData)))
				{
					REGFILTER2 *pRF2 = (REGFILTER2 *)*((DWORD *)pData);
					pDSFI->dwMerit = pRF2->dwMerit;
					CoTaskMemFree(pRF2);
				}
				SR(pFilterData);
			}
			else
			{
				pDSFI->dwMerit = 0;
				//Обнуление остальных членов структуры
			}
			SysFreeString(varDSFName.bstrVal);
			pPropBag->Release();
			break;
		}
		else
		{
			if (m_lCounter >= (m_lDSFilCount - 1))
			{
				SysFreeString(varDSFName.bstrVal);
				pPropBag->Release();
				return -1;
			}
		}
		SysFreeString(varDSFName.bstrVal);
		pPropBag->Release();
	}
	return 0;
}

//Добавляет фильтр DirectShow в Filter Graph
//Перед вызовом этой функции необходимо вызвать UpdateDSFiltersArray(...) (если она не вызывалась)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::AddDSFilterToFilterGraph(LPCWSTR lpwDSFilName)
{
	if (!m_pGraphBuilder) return -1;
	if (!m_lDSFilCount) return -1;
	IPropertyBag *pPropBag = NULL;
	VARIANT varDSFName = { 0 };
	varDSFName.vt = VT_BSTR;
	IBaseFilter *pDSFilter = NULL;
	for (m_lCounter = 0; m_lCounter < m_lDSFilCount; m_lCounter++)
	{
		m_pDSFMoniker[m_lCounter]->BindToStorage(NULL, NULL, IID_IPropertyBag, (LPVOID *)&pPropBag);
		if (FAILED(pPropBag->Read(L"FriendlyName", &varDSFName, NULL)))
		{
			if (m_lCounter >= (m_lDSFilCount - 1))
			{
				pPropBag->Release();
				return -1;
			} else continue;
		}
		if (_wcsicmp(lpwDSFilName, varDSFName.bstrVal) == 0)
		{
			if (FAILED(m_pDSFMoniker[m_lCounter]->BindToObject(NULL, NULL, IID_IBaseFilter,
				(LPVOID*)&pDSFilter)))
			{
				SysFreeString(varDSFName.bstrVal);
				pPropBag->Release();
				return -1;
			}
			m_pGraphBuilder->AddFilter(pDSFilter, varDSFName.bstrVal);
			SysFreeString(varDSFName.bstrVal);
			pPropBag->Release();
			break;
		}
		else
		{
			if (m_lCounter >= (m_lDSFilCount - 1))
			{
				SysFreeString(varDSFName.bstrVal);
				pPropBag->Release();
				return -1;
			}
		}
		SysFreeString(varDSFName.bstrVal);
		pPropBag->Release();
	}
	return 0;
}

//Обновляет массив m_pDSFMoniker[], хранящий список зарегистрированных в системе
//фильтров DirectShow заданной категории
//Функцию достаточно вызвать единожды (если не потребуются фильтры другой категории)
void CDirectShow::UpdateDSFiltersArray(DSFCATEGORY dCategory)
{
	ICreateDevEnum *pCreateDevEnum = NULL;
	IEnumMoniker *pEnumMoniker = NULL;
	GUID gSelCategory = { 0 };
	for (m_lCounter = 0; m_lCounter < E_MAX_ARR_SIZE; m_lCounter++)
	{
		SR(m_pDSFMoniker[m_lCounter]);
		m_pDSFMoniker[m_lCounter] = NULL;
	}
	switch (dCategory)
	{
		case DSF_CATEGORY_AUDIO_INPUT_DEVICE:
			gSelCategory = CLSID_AudioInputDeviceCategory;
			break;
		case DSF_CATEGORY_AUDIO_COMPRESSOR:
			gSelCategory = CLSID_AudioCompressorCategory;
			break;
		case DSF_CATEGORY_AUDIO_RENDERER:
			gSelCategory = CLSID_AudioRendererCategory;
			break;
		case DSF_CATEGORY_LEGACY_AM_FILTER:
			gSelCategory = CLSID_LegacyAmFilterCategory;
			break;
		case DSF_CATEGORY_MIDI_RENDERER:
			gSelCategory = CLSID_MidiRendererCategory;
			break;
		case DSF_CATEGORY_VIDEO_INPUT_DEVICE:
			gSelCategory = CLSID_VideoInputDeviceCategory;
			break;
		case DSF_CATEGORY_VIDEO_COMPRESSOR:
			gSelCategory = CLSID_VideoCompressorCategory;
			break;
		default:
			return;
	}
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, 
		(LPVOID *)&pCreateDevEnum);
	pCreateDevEnum->CreateClassEnumerator(gSelCategory, &pEnumMoniker, 0);
	pEnumMoniker->Next(E_MAX_ARR_SIZE, &m_pDSFMoniker[0], &m_lDSFilCount);
	pEnumMoniker->Release();
	pCreateDevEnum->Release();
}

//Возвращает массив имен фильтров DirectShow, находящихся в текущем Filter Graph
//Что бы узнать требуемую размерность массива, достаточно вызвать функцию
//с lpwFGFilArr равным NULL - значение вернется в pArrSize.
//Перед вызовом этой функции желательно вызвать UpdateFGFiltersArray()
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetFGFiltersNames(LPWSTR *lpwFGFilArr, LPDWORD pArrSize, DWORD dwBuffSize)
{
	if (lpwFGFilArr)
	{
		FILTER_INFO FI = { 0 };
		for (m_lCounter = 0; m_lCounter < *pArrSize; m_lCounter++)
		{
			if (m_pFGBaseFilter[m_lCounter] == NULL) return -1;
			if (FAILED(m_pFGBaseFilter[m_lCounter]->QueryFilterInfo(&FI)))
			{
				if (m_lCounter >= (*pArrSize - 1))
					return -1; else continue;
			}
			else
			{
				wcsncpy(lpwFGFilArr[m_lCounter], FI.achName, dwBuffSize);
				SR(FI.pGraph);
			}
		}
		return 0;
	}
	else
	{
		*pArrSize = m_lFGFilCount;
		return 0;
	}
}

//Вызывает окно свойств фильтра lpwFGFilName
//Если параметр bCheck равен TRUE, функция лишь проверит наличие у фильтра окна свойств,
//и вернет значение больше нуля, если таковое окно имеется
//Перед вызовом этой функции желательно вызвать UpdateFGFiltersArray()
//В случае ошибки/отсутствия у фильтра окна свойств функция вернет значение меньше нуля
int CDirectShow::FGFiltersPropertyPages(LPCWSTR lpwFGFilName, BOOL bCheck)
{
	if (!m_lFGFilCount) return -1;
	ISpecifyPropertyPages *pSpecifyPP = NULL;
	FILTER_INFO FI = { 0 };
	for (m_lCounter = 0; m_lCounter < m_lFGFilCount; m_lCounter++)
	{
		if (m_pFGBaseFilter[m_lCounter] == NULL) return -1;
		if (FAILED(m_pFGBaseFilter[m_lCounter]->QueryFilterInfo(&FI)))
		{
			if (m_lCounter >= (m_lFGFilCount - 1))
				return -1; else continue;
		}
		else
		{
			SR(FI.pGraph);
			if (_wcsicmp(FI.achName, lpwFGFilName) == 0)
			{
				if (FAILED(m_pFGBaseFilter[m_lCounter]->QueryInterface(IID_ISpecifyPropertyPages,
					(LPVOID *)&pSpecifyPP)))
				{
					return -1;
				}
				else
				{
					if (bCheck)
					{
						pSpecifyPP->Release();
						return 1;
					}
					else
					{
						CAUUID caUUID = { 0 };
						pSpecifyPP->GetPages(&caUUID);
						pSpecifyPP->Release();
						HRESULT hRCPF = OleCreatePropertyFrame(m_hAppWnd, 0, 0, FI.achName, 1,
							(IUnknown **)&m_pFGBaseFilter[m_lCounter], caUUID.cElems, caUUID.pElems, 0, 0, NULL);
						CoTaskMemFree(caUUID.pElems);
						if (FAILED(hRCPF))
						{
							WCHAR lpwMsg[512] = { 0 };
							swprintf(lpwMsg, L"Unable to display '%s' properties.", FI.achName);
							DSErrorMsg(hRCPF, lpwMsg);
							return -1;
						}
						break;
					}
				}
			}
		}
	}
	return 0;
}

//Обновляет массив m_pFGBaseFilter[], хранящий список фильтров DirectShow в текущем Filter Graph
//Функция Open() вызывает эту функцию автоматически
//Однако, крайне желательно вызывать её повторно перед вызовом функций,
//работающих с m_pFGBaseFilter[] массивом
void CDirectShow::UpdateFGFiltersArray()
{
	IEnumFilters *pEnumFilters = NULL;
	if (FAILED(m_pGraphBuilder->EnumFilters(&pEnumFilters))) return;
	for (m_lCounter = 0; m_lCounter < E_MAX_ARR_SIZE; m_lCounter++)
	{
		SR(m_pFGBaseFilter[m_lCounter]);
		m_pFGBaseFilter[m_lCounter] = NULL;
	}
	pEnumFilters->Reset();
	pEnumFilters->Next(E_MAX_ARR_SIZE, &m_pFGBaseFilter[0], &m_lFGFilCount);
	pEnumFilters->Release();
}

//Возвращает массив имен DMO (DirectX Media Objects)
//Способ получения необходимой размерности массива аналогичен способу у функции GetFGFiltersNames(...)
//Перед вызовом этой функции необходимо вызвать UpdateDMOArray(...) (если она не вызывалась)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetDMONames(LPWSTR *lpwDMOArr, LPDWORD pArrSize, DWORD dwBuffSize)
{
	if (lpwDMOArr)
	{
		for (m_lCounter = 0; m_lCounter < *pArrSize; m_lCounter++)
		{
			if (m_lpwDMONames[m_lCounter] == NULL) return -1;
			wcsncpy(lpwDMOArr[m_lCounter], m_lpwDMONames[m_lCounter], dwBuffSize);
		}
		return 0;
	}
	else
	{
		*pArrSize = m_lDMOCount;
		return 0;
	}
}

//Добавляет к текущему Filter Graph оболочку 'DMOWrapper' и вызывает инициализацию
//DMO объекта с именем lpwDMOName
//Перед вызовом этой функции необходимо вызвать UpdateDMOArray(...) (если она не вызывалась)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::AddDMOToFilterGraph(LPCWSTR lpwDMOName)
{
	if (!m_pGraphBuilder) return -1;
	if (!m_lDMOCount) return -1;
	IBaseFilter *pDMOWBaseFilter = NULL;
	IDMOWrapperFilter *pDMOWrapper;
	for (m_lCounter = 0; m_lCounter < m_lDMOCount; m_lCounter++)
	{
		if (_wcsicmp(m_lpwDMONames[m_lCounter], lpwDMOName) == 0)
		{
			if (FAILED(CoCreateInstance(CLSID_DMOWrapperFilter, NULL, CLSCTX_INPROC,
				IID_IBaseFilter, (LPVOID *)&pDMOWBaseFilter))) return -1;
			if (FAILED(pDMOWBaseFilter->QueryInterface(IID_IDMOWrapperFilter,
				(LPVOID *)&pDMOWrapper))) return -1;
			pDMOWrapper->Init(m_cDMOCLSIDs[m_lCounter], m_gidRecentDMOCat);
			pDMOWrapper->Release();
			m_pGraphBuilder->AddFilter(pDMOWBaseFilter, m_lpwDMONames[m_lCounter]);
			pDMOWBaseFilter->Release();
		}
		else
		{
			if (m_lCounter >= (m_lDMOCount - 1))
				return -1; else continue;
		}
	}
	return 0;
}

//Инициализирует массивы *DMO*
//Функцию достаточно вызвать единожды (если не потребуется DMO другой категории)
void CDirectShow::UpdateDMOArray(DMOCATEGORY dCategory)
{
	IEnumDMO *pEnumDMO = NULL;
	LPWSTR lpwDMOTmp[E_MAX_ARR_SIZE];
	for (m_lCounter = 0; m_lCounter < E_MAX_ARR_SIZE; m_lCounter++)
	{
		lpwDMOTmp[m_lCounter] = NULL;
		if (m_lpwDMONames[m_lCounter])
		{
			delete[] m_lpwDMONames[m_lCounter];
			m_lpwDMONames[m_lCounter] = NULL;
		}
		ZeroMemory(&m_cDMOCLSIDs[m_lCounter], sizeof(m_cDMOCLSIDs[m_lCounter]));
	}
	ZeroMemory(&m_gidRecentDMOCat, sizeof(m_gidRecentDMOCat));
	switch (dCategory)
	{
		case DMO_CATEGORY_AUDIO_DECODER:
			m_gidRecentDMOCat = DMOCATEGORY_AUDIO_DECODER;
			break;
		case DMO_CATEGORY_AUDIO_EFFECT:
			m_gidRecentDMOCat = DMOCATEGORY_AUDIO_EFFECT;
			break;
		case DMO_CATEGORY_AUDIO_ENCODER:
			m_gidRecentDMOCat = DMOCATEGORY_AUDIO_ENCODER;
			break;
		case DMO_CATEGORY_VIDEO_DECODER:
			m_gidRecentDMOCat = DMOCATEGORY_VIDEO_DECODER;
			break;
		case DMO_CATEGORY_VIDEO_EFFECT:
			m_gidRecentDMOCat = DMOCATEGORY_VIDEO_EFFECT;
			break;
		case DMO_CATEGORY_VIDEO_ENCODER:
			m_gidRecentDMOCat = DMOCATEGORY_VIDEO_ENCODER;
			break;
		case DMO_CATEGORY_AUDIO_CAPTURE_EFFECT:
			m_gidRecentDMOCat = DMOCATEGORY_AUDIO_CAPTURE_EFFECT;
			break;
		default:
			return;
	}
    if (FAILED(DMOEnum(m_gidRecentDMOCat, DMO_ENUMF_INCLUDE_KEYED, 0,
		NULL, 0, NULL, &pEnumDMO))) return;
	pEnumDMO->Next(E_MAX_ARR_SIZE, &m_cDMOCLSIDs[0], &lpwDMOTmp[0], &m_lDMOCount);
	if (!m_lDMOCount) return;
	for (m_lCounter = 0; m_lCounter < m_lDMOCount; m_lCounter++)
	{
		m_lpwDMONames[m_lCounter] = new WCHAR[MAX_PATH];
		wcscpy(m_lpwDMONames[m_lCounter], lpwDMOTmp[m_lCounter]);
		CoTaskMemFree(lpwDMOTmp[m_lCounter]);
	}
	pEnumDMO->Release();
}

//Возвращает информацию о медиа файле (если таковая доступна)
//Как правило, подобную информацию предоставляют т.н. 'parser filters'
//Перед вызовом этой функции желательно вызвать UpdateFGFiltersArray()
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetMediaContent(LPMEDIACONTENT pMC)
{
	if (!m_lFGFilCount) return -1;
	IAMMediaContent *pAMMediaContent = NULL;
	BSTR bstrTmp;
	for (m_lCounter = 0; m_lCounter < m_lFGFilCount; m_lCounter++)
	{
		if (!m_pFGBaseFilter[m_lCounter]) return -1;
		m_pFGBaseFilter[m_lCounter]->QueryInterface(IID_IAMMediaContent, (LPVOID *)&pAMMediaContent);
		if (!pAMMediaContent)
		{
			if (m_lCounter >= (m_lFGFilCount - 1)) return -1;
		}
		else break;
	}
	//Копируем на один символ меньше, чем позволяют соответствующие члены структуры
	//Это на случай, если строка окажется больше и null-char просто не скопируется
	if (SUCCEEDED(pAMMediaContent->get_AuthorName(&bstrTmp)))
	{
		wcsncpy(pMC->Author, bstrTmp, 63);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_Title(&bstrTmp)))
	{
		wcsncpy(pMC->Title, bstrTmp, 63);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_Rating(&bstrTmp)))
	{
		wcsncpy(pMC->Rating, bstrTmp, 63);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_Copyright(&bstrTmp)))
	{
		wcsncpy(pMC->Copyright, bstrTmp, 127);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_Description(&bstrTmp)))
	{
		wcsncpy(pMC->Description, bstrTmp, 254);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_MoreInfoText(&bstrTmp)))
	{
		wcsncpy(pMC->MoreInfo, bstrTmp, 254);
		SysFreeString(bstrTmp);
	}
	pAMMediaContent->Release();
	return 0;
}

//Приватная функция. Удаляет текущий граф из ROT (Running Object Table)
void CDirectShow::RemoveFGFromROT()
{
	IRunningObjectTable *pRunningObjectTable = NULL;
    if (SUCCEEDED(GetRunningObjectTable(0, &pRunningObjectTable)))
	{
        pRunningObjectTable->Revoke(m_dwROTRegister);
        pRunningObjectTable->Release();
		m_dwROTRegister = 0;
    }
}

//Приватная функция. Выводит сообщение об ошибке DirectShow (hex-код и описание)
void CDirectShow::DSErrorMsg(HRESULT hr, LPCWSTR lpwEM)
{
	WCHAR lpwMsg[512] = { 0 };
	WCHAR lpwErrDesc[MAX_ERROR_TEXT_LEN] = { 0 };
	AMGetErrorText(hr, lpwErrDesc, MAX_ERROR_TEXT_LEN);
	if (!lpwEM) lpwEM = m_lpwFileName;
	swprintf(lpwMsg, L"%s\n\nError code: 0x%08x\nError description: %s",
		lpwEM, hr, wcslen(lpwErrDesc)?lpwErrDesc:L"None");
	MessageBox(m_hAppWnd, lpwMsg, (m_lpwAppName)?m_lpwAppName:L"Error",
		MB_ICONEXCLAMATION);
}
