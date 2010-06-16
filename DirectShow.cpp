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
	m_lpFileName = NULL;
	m_lpAppName = NULL;
	m_hAppWnd = NULL;
	m_hDSCBWnd = NULL;
	m_bNoFGError = FALSE;
	m_lCounter = 0;
	m_dwROTRegister = 0;
	m_lBACount = 0;
	m_intCurrentBA = -1;
	m_lDSFilCount = 0;
	m_lFGFilCount = 0;
	m_lDMOCount = 0;
	m_pAMStreamSelect = NULL;
	m_pGraphBuilder = NULL;
	m_pMediaControl = NULL;
	m_pMediaSeeking = NULL;
	m_pMediaEventEx = NULL;
	m_pBasicVideo2 = NULL;
	m_pVideoWindow = NULL;
	m_pVideoFrameStep = NULL;
	//Инициализация массивов
	//------------------------------------------------
	for (m_lCounter = 0; m_lCounter < E_MAX_BA; m_lCounter++)
		m_pBasicAudio[m_lCounter] = NULL;
	for (m_lCounter = 0; m_lCounter < E_MAX_BF; m_lCounter++)
		m_pFGBaseFilter[m_lCounter] = NULL;
	for (m_lCounter = 0; m_lCounter < E_MAX_ARR_SIZE; m_lCounter++)
	{
		m_pDSFMoniker[m_lCounter] = NULL;
		m_lpDMONames[m_lCounter] = NULL;
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
		WNDCLASSEX WCEX = {};
		WCEX.cbSize = sizeof(WNDCLASSEX); 
		WCEX.lpfnWndProc = (WNDPROC)DSCBWndProc;
		WCEX.hInstance = hInstance;
		WCEX.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		WCEX.lpszClassName = DS_CB_WND_CLASS;
		RegisterClassEx(&WCEX);
		m_hDSCBWnd = CreateWindow(WCEX.lpszClassName, NULL, WS_POPUP,
			16, 16, 32, 32, HWND_MESSAGE, NULL, hInstance, (LPVOID)this);
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
			long lEvent;
			LONG_PTR lParam1, lParam2;
			if (SUCCEEDED(pThis->m_pMediaEventEx->GetEvent(&lEvent, &lParam1, &lParam2, 0)))
			{
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
			}
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
			(m_lpAppName)?m_lpAppName:L"Error", MB_ICONSTOP);
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
	HRESULT hRRF = m_pGraphBuilder->RenderFile(m_lpFileName, NULL);
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
	m_intPrevVol = 1;
	m_pGraphBuilder->QueryInterface(IID_IBasicVideo, (LPVOID *)&m_pBasicVideo2);
	m_pGraphBuilder->QueryInterface(IID_IVideoWindow, (LPVOID *)&m_pVideoWindow);
	m_pGraphBuilder->QueryInterface(IID_IVideoFrameStep, (LPVOID *)&m_pVideoFrameStep);
	UpdateFGFiltersArray();
	if (m_lBACount)
		SelectAudioStream_E(0, 0, 0, SASF_APPLYINITPARAMS);
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
	{
		if (!HasVideo())
			m_pMediaControl->Stop();
		if (m_pMediaSeeking)
		{
			OAFilterState fState;
			__int64 intPos = 0;
			m_pMediaControl->GetState(INFINITE, &fState);
			m_pMediaSeeking->SetPositions(&intPos, AM_SEEKING_AbsolutePositioning, NULL,
				AM_SEEKING_NoPositioning);
		}
		if (HasVideo())
			m_pMediaControl->StopWhenReady();
	}
}

//Здесь происходит сброс всех параметров и освобождение интерфейсов
void CDirectShow::Close()
{
	if (m_pMediaEventEx)
		m_pMediaEventEx->SetNotifyWindow(NULL, 0, 0);
	if (m_pVideoWindow)
	{
		//m_pVideoWindow->put_Visible(OAFALSE);
		m_pVideoWindow->put_MessageDrain(NULL);
	}
	SR(m_pAMStreamSelect);
	for (m_lCounter = 0; m_lCounter < m_lBACount; m_lCounter++)
		SR(m_pBasicAudio[m_lCounter]);
	m_lBACount = 0;
	m_intCurrentBA = -1;
	for (m_lCounter = 0; m_lCounter < m_lFGFilCount; m_lCounter++)
		SR(m_pFGBaseFilter[m_lCounter]);
	m_lFGFilCount = 0;
	if (m_dwROTRegister)
		RemoveFGFromROT();
	SR(m_pMediaControl);
	SR(m_pMediaSeeking);
	SR(m_pMediaEventEx);
	SR(m_pBasicVideo2);
	SR(m_pVideoWindow);
	SR(m_pVideoFrameStep);
	SR(m_pGraphBuilder);
}

double CDirectShow::GetRate()
{
	if (!IsSeekable()) return 0;
	if (m_pMediaSeeking)
	{
		double dblTmp;
		m_pMediaSeeking->GetRate(&dblTmp);
		return dblTmp;
	} else return 0;
}

void CDirectShow::SetRate(double dblRate)
{
	if (!IsSeekable()) return;
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
	if (!IsSeekable()) return 0;
	if (m_pMediaSeeking)
	{
		__int64 intRes;
		m_pMediaSeeking->GetCurrentPosition(&intRes);
		return (bInMS)?(intRes / 10000):intRes;
	} else return 0;
}

void CDirectShow::SetPosition(__int64 intNewPos, BOOL bInMS, BOOL bSeekToKeyFrame)
{
	if (!IsSeekable()) return;
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
	if (!IsSeekable()) return 0;
	if (m_pMediaSeeking)
	{
		__int64 intRes;
		m_pMediaSeeking->GetDuration(&intRes);
		return (bInMS)?(intRes / 10000):intRes;
	} else return 0;
}

void CDirectShow::SetLength(__int64 intNewLen, BOOL bInMS)
{
	if (!IsSeekable()) return;
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
	if (!HasAudio()) return 0;
	if (m_pBasicAudio[m_intCurrentBA])
	{
		return (m_intPrevVol == 1)?0:1;
	} else return 0;
}

void CDirectShow::SetMute(int intValue)
{
	if (!HasAudio()) return;
	if (m_pBasicAudio[m_intCurrentBA])
	{
		if (intValue)
		{
			if (m_intPrevVol == 1)
			{
				m_pBasicAudio[m_intCurrentBA]->get_Volume((long *)&m_intPrevVol);
				m_pBasicAudio[m_intCurrentBA]->put_Volume(-10000);
			}
		}
		else
		{
			if (m_intPrevVol < 1)
			{
				m_pBasicAudio[m_intCurrentBA]->put_Volume((long)m_intPrevVol);
				m_intPrevVol = 1;
			}
		}
	}
}

int CDirectShow::GetVolume()
{
	if (!HasAudio()) return 0;
	if (m_pBasicAudio[m_intCurrentBA])
	{
		if (m_intPrevVol == 1)
		{
			long lTmp;
			m_pBasicAudio[m_intCurrentBA]->get_Volume(&lTmp);
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
	if (!HasAudio()) return;
	if (m_pBasicAudio[m_intCurrentBA])
	{
		if (m_intPrevVol == 1)
		{
			m_pBasicAudio[m_intCurrentBA]->put_Volume((long)intValue);
		} else m_intPrevVol = intValue;
	}
}

int CDirectShow::GetBalance()
{
	if (!HasAudio()) return 0;
	if (m_pBasicAudio[m_intCurrentBA])
	{
		long lTmp;
		m_pBasicAudio[m_intCurrentBA]->get_Balance(&lTmp);
		return (int)lTmp;
	} else return 0;
}

//От -10000 (L) до 10000 (R)
void CDirectShow::SetBalance(int intValue)
{
	if (!HasAudio()) return;
	if (m_pBasicAudio[m_intCurrentBA])
		m_pBasicAudio[m_intCurrentBA]->put_Balance((long)intValue);
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

int CDirectShow::HasAudio()
{
	if (m_lBACount && m_pBasicAudio[m_intCurrentBA])
	{
		long lTmp = 0;
		if (SUCCEEDED(m_pBasicAudio[m_intCurrentBA]->get_Volume(&lTmp))) return 1; else return 0;
	}
	else return 0;
}

int CDirectShow::HasVideo()
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
	if (!CanStep(dwFrames)) return;
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
	if (!HasVideo()) return 0;
	if (m_pBasicVideo2)
	{
		m_pBasicVideo2->GetVideoSize(&pSZ->cx, &pSZ->cy);
		return 1;
	} else return 0;
}

int CDirectShow::GetVideoSize(LPRECT pRC)
{
	if (!HasVideo()) return 0;
	if (m_pVideoWindow)
	{
		m_pVideoWindow->GetWindowPosition(&pRC->left, &pRC->top, &pRC->right, &pRC->bottom);
		return 1;
	} else return 0;
}

void CDirectShow::SetVideoSize(RECT RC)
{
	if (!HasVideo()) return;
	if (m_pVideoWindow)
		m_pVideoWindow->SetWindowPosition(RC.left, RC.top, RC.right, RC.bottom);
}

HWND CDirectShow::GetVideoOwner()
{
	if (!HasVideo()) return NULL;
	if (m_pVideoWindow)
	{
		OAHWND hOwner;
		m_pVideoWindow->get_Owner(&hOwner);
		return (HWND)hOwner;
	} else return NULL;
}

void CDirectShow::SetVideoOwner(HWND hOwner)
{
	if (!HasVideo()) return;
	if (m_pVideoWindow)
		m_pVideoWindow->put_Owner((OAHWND)hOwner);
}

int CDirectShow::GetVideoStyles(LPLONG pStyle, LPLONG pExStyle)
{
	if (!HasVideo()) return 0;
	if (m_pVideoWindow)
	{
		if (pStyle) m_pVideoWindow->get_WindowStyle(pStyle);
		if (pExStyle) m_pVideoWindow->get_WindowStyleEx(pExStyle);
		return 1;
	} else return 0;
}

void CDirectShow::SetVideoStyles(LONG lStyle, LONG lExStyle)
{
	if (!HasVideo()) return;
	if (m_pVideoWindow)
	{
		if (lStyle) m_pVideoWindow->put_WindowStyle(lStyle);
		if (lExStyle) m_pVideoWindow->put_WindowStyleEx(lExStyle);
	}
}

int CDirectShow::GetVideoVisible()
{
	if (!HasVideo()) return 0;
	if (m_pVideoWindow)
	{
		long lVisible;
		m_pVideoWindow->get_Visible(&lVisible);
		return (lVisible == OATRUE)?1:0;
	} else return 0;
}

void CDirectShow::SetVideoVisible(BOOL bVisible)
{
	if (!HasVideo()) return;
	if (m_pVideoWindow)
		m_pVideoWindow->put_Visible((bVisible)?OATRUE:OAFALSE);
}

int CDirectShow::GetFullscreen()
{
	if (!HasVideo()) return 0;
	if (m_pVideoWindow)
	{
		long bFS;
		m_pVideoWindow->get_FullScreenMode(&bFS);
		return (bFS == OATRUE)?1:0;
	} else return 0;
}

void CDirectShow::SetFullscreen(int intMode)
{
	if (!HasVideo()) return;
	if (m_pVideoWindow)
		m_pVideoWindow->put_FullScreenMode((intMode)?OATRUE:OAFALSE);
}

//Копирует текущий кадр в буфер обмена
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::CopyCurrentFrame() //*доработанная версия функции от Jenya
{
	if (!HasVideo()) return 0;
	if (m_pBasicVideo2)
	{
		HGLOBAL hGbCopy;
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

		if (!OpenClipboard(NULL)) return -1;
		EmptyClipboard();

		hGbCopy = GlobalAlloc(GMEM_MOVEABLE, lArrSize);
		pImage = (LPBYTE)GlobalLock(hGbCopy);
		m_pBasicVideo2->GetCurrentImage(&lArrSize, (long *)pImage);
		GlobalUnlock(hGbCopy);

		SetClipboardData(CF_DIB, hGbCopy);
		CloseClipboard();

		if (m_pMediaControl)
			if (fTmp == 2) m_pMediaControl->Run();
		return 0;
	} else return -1;
}

//Сохраняет текущий кадр в файл (Windows Bitmap)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::SaveCurrentFrame(LPCWSTR lpFileName) //*доработанная версия функции от Jenya
{
	if (!HasVideo()) return 0;
	if (m_pBasicVideo2)
	{
		HANDLE hFile;
		BITMAPFILEHEADER BFH = {};
		LPBITMAPINFOHEADER pBI;
		LPBITMAPCOREHEADER pBC;
		LPBYTE pImage;
		DWORD dwWritten;
		long lArrSize;
		DWORD dwColors, dwPaletteSize;
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
		hFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (hFile != INVALID_HANDLE_VALUE)
		{
			pBI = (LPBITMAPINFOHEADER)pImage;
			pBC = (LPBITMAPCOREHEADER)pImage;

			if (pBI->biSize == sizeof(BITMAPCOREHEADER))
			{
				if (pBC->bcBitCount > 8)
					dwColors = 0;
				else
					dwColors =  (DWORD)pow((double)2, pBC->bcBitCount);
				dwPaletteSize = (dwColors * sizeof(RGBTRIPLE));
			}
			else
			{
				 if (pBI->biClrUsed != 0)
					 dwColors = pBI->biClrUsed;
				 else if (pBI->biBitCount > 8)
					 dwColors = 0;
				 else
					 dwColors =  (DWORD)pow((double)2, (int)pBI->biBitCount);
				 dwPaletteSize = (dwColors * sizeof(RGBQUAD));
			}

			BFH.bfType = 0x4D42; //0x42 -- 'B', 0x4D -- 'M'
			BFH.bfOffBits = (sizeof(BITMAPFILEHEADER) + pBI->biSize + dwPaletteSize);
			BFH.bfSize = (sizeof(BITMAPFILEHEADER) + lArrSize);
			if (WriteFile(hFile, &BFH, sizeof(BITMAPFILEHEADER), &dwWritten, NULL))
				WriteFile(hFile, pImage, lArrSize, &dwWritten, NULL);
			SetEndOfFile(hFile);
			CloseHandle(hFile);
		}

		delete[] pImage;
		if (m_pMediaControl)
			if (fTmp == 2) m_pMediaControl->Run();
		return 0;
	} else return -1;
}

//Возвращает массив имен (имена опциональны) доступных потоков заданного типа
//Что бы узнать требуемую размерность массива , достаточно вызвать функцию
//с lpASArr равным NULL - значение вернется в pArrSize.
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetAvailableStreams(DSSTREAMTYPE dStreamType, LPWSTR *lpStmArr, LPDWORD pArrSize, DWORD dwBuffSize)
{
	if (!m_pAMStreamSelect) return -1;
	DWORD dwStmCnt = 0, dwStmCnt2 = 0;
	AM_MEDIA_TYPE *pMT = NULL;
	LPWSTR lpName = NULL;
	m_pAMStreamSelect->Count(&dwStmCnt);
	for (m_lCounter = 0; m_lCounter < dwStmCnt; m_lCounter++)
	{
		if (SUCCEEDED(m_pAMStreamSelect->Info(m_lCounter, &pMT, NULL, NULL, NULL, &lpName, NULL, NULL)))
		{
			if ((pMT->majortype == MEDIATYPE_Audio) && (dStreamType == DSST_AUDIO))
			{
				if (lpStmArr)
					wcsncpy(lpStmArr[dwStmCnt2++], lpName, dwBuffSize);
				else (*pArrSize)++;
			}
			else if ((pMT->majortype == MEDIATYPE_Video) && (dStreamType == DSST_VIDEO))
			{
				if (lpStmArr)
					wcsncpy(lpStmArr[dwStmCnt2++], lpName, dwBuffSize);
				else (*pArrSize)++;
			}
			FreeMediaType(pMT);
			CoTaskMemFree(lpName);
		}
	}
	return 0;
}

//Выбирает поток
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::SelectStream(DSSTREAMTYPE dStreamType, LPCWSTR lpStmName)
{
	if (!m_pAMStreamSelect) return -1;
	DWORD dwStmCnt = 0;
	AM_MEDIA_TYPE *pMT = NULL;
	LPWSTR lpName = NULL;
	m_pAMStreamSelect->Count(&dwStmCnt);
	for (m_lCounter = 0; m_lCounter < dwStmCnt; m_lCounter++)
	{
		if (SUCCEEDED(m_pAMStreamSelect->Info(m_lCounter, &pMT, NULL, NULL, NULL, &lpName, NULL, NULL)))
		{
			if ((pMT->majortype == MEDIATYPE_Audio) && (dStreamType == DSST_AUDIO))
			{
				if (_wcsicmp(lpName, lpStmName) == 0)
				{
					m_pAMStreamSelect->Enable(m_lCounter, AMSTREAMSELECTENABLE_ENABLE);
					break;
				}
			}
			else if ((pMT->majortype == MEDIATYPE_Video) && (dStreamType == DSST_VIDEO))
			{
				if (_wcsicmp(lpName, lpStmName) == 0)
				{
					m_pAMStreamSelect->Enable(m_lCounter, AMSTREAMSELECTENABLE_ENABLE);
					break;
				}
			}
			FreeMediaType(pMT);
			CoTaskMemFree(lpName);
		}
	}
	return 0;
}

//Позволяет узнать, выбран ли поток
BOOL CDirectShow::IsStreamSelected(DSSTREAMTYPE dStreamType, LPCWSTR lpStmName)
{
	if (!m_pAMStreamSelect) return FALSE;
	DWORD dwStmCnt = 0;
	AM_MEDIA_TYPE *pMT = NULL;
	LPWSTR lpName = NULL;
	DWORD dwFlags;
	BOOL bResult = FALSE;
	m_pAMStreamSelect->Count(&dwStmCnt);
	for (m_lCounter = 0; m_lCounter < dwStmCnt; m_lCounter++)
	{
		if (SUCCEEDED(m_pAMStreamSelect->Info(m_lCounter, &pMT, &dwFlags, NULL, NULL, &lpName, NULL, NULL)))
		{
			if ((pMT->majortype == MEDIATYPE_Audio) && (dStreamType == DSST_AUDIO))
			{
				if (_wcsicmp(lpName, lpStmName) == 0)
				{
					bResult = (dwFlags != 0);
					break;
				}
			}
			else if ((pMT->majortype == MEDIATYPE_Video) && (dStreamType == DSST_VIDEO))
			{
				if (_wcsicmp(lpName, lpStmName) == 0)
				{
					bResult = (dwFlags != 0);
					break;
				}
			}
			FreeMediaType(pMT);
			CoTaskMemFree(lpName);
		}
	}
	return bResult;
}

//Возвращает количество аудио потоков (внутренний механизм)
int CDirectShow::GetAudioStreamsCount_E()
{
	return m_lBACount;
}

//Вибирает аудио поток (внутренний механизм)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::SelectAudioStream_E(int intStmIndex, int intStmInitVol, int intStmInitBal, DWORD dwFlags)
{
	if (!m_lBACount) return -1;
	if (intStmIndex == m_intCurrentBA) return -1;
	if ((intStmIndex < 0) || (intStmIndex > (int)m_lBACount)) return -1;
	long lCurVol, lCurBal;
	if ((dwFlags & SASF_APPLYINITPARAMS) != SASF_APPLYINITPARAMS)
	{
		if (m_intPrevVol == 1)
			m_pBasicAudio[m_intCurrentBA]->get_Volume(&lCurVol);
		m_pBasicAudio[m_intCurrentBA]->get_Balance(&lCurBal);
	}
	for (m_lCounter = 0; m_lCounter < m_lBACount; m_lCounter++)
	{
		if (m_lCounter != intStmIndex)
			m_pBasicAudio[m_lCounter]->put_Volume(-10000);
	}
	if ((dwFlags & SASF_APPLYINITPARAMS) == SASF_APPLYINITPARAMS)
	{
		if (m_intPrevVol == 1)
			m_pBasicAudio[intStmIndex]->put_Volume(intStmInitVol);
		else
			m_intPrevVol = intStmInitVol;
		m_pBasicAudio[intStmIndex]->put_Balance(intStmInitBal);
	}
	else
	{
		if (m_intPrevVol == 1)
			m_pBasicAudio[intStmIndex]->put_Volume(lCurVol);
		m_pBasicAudio[intStmIndex]->put_Balance(lCurBal);
	}
	m_intCurrentBA = intStmIndex;
	return 0;
}

//Позволяет узнать, выбран ли аудио поток (внутренний механизм)
BOOL CDirectShow::IsAudioStreamSelected_E(int intStmIndex)
{
	if (!m_lBACount) return FALSE;
	if ((intStmIndex < 0) || (intStmIndex > (int)m_lBACount)) return -1;
	return (m_intCurrentBA == intStmIndex);
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

		/*if (m_pEBGraphBuilder)
			delete m_pEBGraphBuilder;
		m_pEBGraphBuilder = new CEBGraphBuilder();
		m_pEBGraphBuilder->AttachGraph(m_pGraphBuilder);
		m_pEBGraphBuilder->CreateMarshal();*/
		
		WCHAR lpItem[256] = {};
		swprintf(lpItem, L"FilterGraph %08x pid %08x", (LPDWORD)/*m_pEBGraphBuilder*/m_pGraphBuilder, GetCurrentProcessId());
		if (FAILED(CreateItemMoniker(L"!", lpItem, &pMoniker)))
		{
			pRunningObjectTable->Release();
			return -1;
		}

		pRunningObjectTable->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, /*m_pEBGraphBuilder*/m_pGraphBuilder, pMoniker, &m_dwROTRegister);
		pMoniker->Release();
		pRunningObjectTable->Release();
		return 0;
	}
	else return -1;
}

//Возвращает массив имен зарегистрированных в системе фильтров DirectShow (определенной категории)
//Что бы узнать требуемую размерность массива, достаточно вызвать функцию
//с lpDSFilArr равным NULL - значение вернется в pArrSize.
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetDSFiltersNames(LPWSTR *lpDSFilArr, LPDWORD pArrSize, DWORD dwBuffSize)
{
	if (!m_lDSFilCount) return -1;
	if (lpDSFilArr)
	{
		IPropertyBag *pPropBag = NULL;
		VARIANT varDSFName = {};
		varDSFName.vt = VT_BSTR;
		for (m_lCounter = 0; m_lCounter < m_lDSFilCount; m_lCounter++)
		{
			if (m_pDSFMoniker[m_lCounter] == NULL) return -1;
			m_pDSFMoniker[m_lCounter]->BindToStorage(NULL, NULL, IID_IPropertyBag, (LPVOID *)&pPropBag);
			if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &varDSFName, NULL)))
			{
				wcsncpy(lpDSFilArr[m_lCounter], varDSFName.bstrVal, dwBuffSize);
				SysFreeString(varDSFName.bstrVal);
			}
			else
			{
				wcscpy(lpDSFilArr[m_lCounter], L"<untitled>");
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
int CDirectShow::GetDSFilterInfo(LPCWSTR lpDSFilName, DSFILTERINFO *pDSFI)
{
	if (!m_pGraphBuilder) return -1;
	if (!m_lDSFilCount) return -1;
	IPropertyBag *pPropBag = NULL;
	VARIANT varDSFName = {}, varDSFCLSID = {}, varDSFData = {};
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
		if (_wcsicmp(lpDSFilName, varDSFName.bstrVal) == 0)
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
int CDirectShow::AddDSFilterToFilterGraph(LPCWSTR lpDSFilName)
{
	if (!m_pGraphBuilder) return -1;
	if (!m_lDSFilCount) return -1;
	IPropertyBag *pPropBag = NULL;
	VARIANT varDSFName = {};
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
		if (_wcsicmp(lpDSFilName, varDSFName.bstrVal) == 0)
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
	GUID gSelCategory = {};
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
//с lpFGFilArr равным NULL - значение вернется в pArrSize.
//Перед вызовом этой функции желательно вызвать UpdateFGFiltersArray()
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetFGFiltersNames(LPWSTR *lpFGFilArr, LPDWORD pArrSize, DWORD dwBuffSize)
{
	if (lpFGFilArr)
	{
		FILTER_INFO FI = {};
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
				wcsncpy(lpFGFilArr[m_lCounter], FI.achName, dwBuffSize);
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

//Вызывает окно свойств фильтра lpFGFilName
//Если параметр bCheck равен TRUE, функция лишь проверит наличие у фильтра окна свойств,
//и вернет значение больше нуля, если таковое окно имеется
//Перед вызовом этой функции желательно вызвать UpdateFGFiltersArray()
//В случае ошибки/отсутствия у фильтра окна свойств функция вернет значение меньше нуля
int CDirectShow::FGFiltersPropertyPages(LPCWSTR lpFGFilName, BOOL bCheck)
{
	if (!m_lFGFilCount) return -1;
	ISpecifyPropertyPages *pSpecifyPP = NULL;
	FILTER_INFO FI = {};
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
			if (_wcsicmp(FI.achName, lpFGFilName) == 0)
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
						CAUUID caUUID = {};
						pSpecifyPP->GetPages(&caUUID);
						pSpecifyPP->Release();
						HRESULT hRCPF = OleCreatePropertyFrame(m_hAppWnd, 0, 0, FI.achName, 1,
							(IUnknown **)&m_pFGBaseFilter[m_lCounter], caUUID.cElems, caUUID.pElems, 0, 0, NULL);
						CoTaskMemFree(caUUID.pElems);
						if (FAILED(hRCPF))
						{
							WCHAR lpMsg[512] = {};
							swprintf(lpMsg, L"Unable to display '%s' properties.", FI.achName);
							DSErrorMsg(hRCPF, lpMsg);
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
//А так же инициализирует массив m_pBasicAudio и интерфейс m_pAMStreamSelect
//Функция Open() вызывает эту функцию автоматически
//Однако, крайне желательно вызывать её повторно перед вызовом функций,
//работающих с m_pFGBaseFilter[] массивом
void CDirectShow::UpdateFGFiltersArray()
{
	IEnumFilters *pEnumFilters = NULL;
	IBasicAudio *pBasicAudio = NULL;
	//CLSID cBF;
	IAMStreamSelect *pStreamSelect = NULL;
	int intOldCurBA = -1;
	long lOldVol, lOldBal;
	if (FAILED(m_pGraphBuilder->EnumFilters(&pEnumFilters))) return;
	for (m_lCounter = 0; m_lCounter < E_MAX_BF; m_lCounter++)
	{
		if (m_pFGBaseFilter[m_lCounter])
		{
			SR(m_pFGBaseFilter[m_lCounter]);
			m_pFGBaseFilter[m_lCounter] = NULL;
		}
	}
	pEnumFilters->Reset();
	pEnumFilters->Next(E_MAX_BF, &m_pFGBaseFilter[0], &m_lFGFilCount);
	pEnumFilters->Release();
	if (m_lBACount)
	{
		intOldCurBA = m_intCurrentBA;
		if (m_intPrevVol == 1)
			m_pBasicAudio[m_intCurrentBA]->get_Volume(&lOldVol);
		m_pBasicAudio[m_intCurrentBA]->get_Balance(&lOldBal);
	}
	m_lBACount = 0;
	m_intCurrentBA = -1;
	for (m_lCounter = 0; m_lCounter < m_lFGFilCount; m_lCounter++)
	{
		//m_pFGBaseFilter[m_lCounter]->GetClassID(&cBF);
		//if (IsEqualCLSID(cBF, CLSID_AudioRender) || IsEqualCLSID(cBF, CLSID_DSoundRender))
		//{
			if (SUCCEEDED(m_pFGBaseFilter[m_lCounter]->QueryInterface(IID_IBasicAudio, (LPVOID *)&pBasicAudio)))
			{
				m_pBasicAudio[++m_intCurrentBA] = pBasicAudio;
				m_lBACount++;
			}
		//}
		if (!m_pAMStreamSelect)
		{
			if (SUCCEEDED(m_pFGBaseFilter[m_lCounter]->QueryInterface(IID_IAMStreamSelect, (LPVOID *)&pStreamSelect)))
				m_pAMStreamSelect = pStreamSelect;
		}
	}
	//Дополнительная попытка получить IBasicAudio от GraphBuilder (от Jenya)
	/*if (m_lBACount == 0)
	{
		if (SUCCEEDED(m_pGraphBuilder->QueryInterface(IID_IBasicAudio, (LPVOID *)&pBasicAudio)))
		{
			m_pBasicAudio[++m_intCurrentBA] = pBasicAudio;
			m_lBACount++;
		}
	}*/
	m_intCurrentBA = -1;
	if ((intOldCurBA >= 0) && m_lBACount)
	{
		if (intOldCurBA <= (int)m_lBACount)
			SelectAudioStream_E(intOldCurBA, (m_intPrevVol == 1)?lOldVol:-10000, lOldBal, SASF_APPLYINITPARAMS);
		else
			SelectAudioStream_E(0, (m_intPrevVol == 1)?lOldVol:-10000, lOldBal, SASF_APPLYINITPARAMS);
	}
}

//Возвращает массив имен DMO (DirectX Media Objects)
//Способ получения необходимой размерности массива аналогичен способу у функции GetFGFiltersNames(...)
//Перед вызовом этой функции необходимо вызвать UpdateDMOArray(...) (если она не вызывалась)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::GetDMONames(LPWSTR *lpDMOArr, LPDWORD pArrSize, DWORD dwBuffSize)
{
	if (lpDMOArr)
	{
		for (m_lCounter = 0; m_lCounter < *pArrSize; m_lCounter++)
		{
			if (m_lpDMONames[m_lCounter] == NULL) return -1;
			wcsncpy(lpDMOArr[m_lCounter], m_lpDMONames[m_lCounter], dwBuffSize);
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
//DMO объекта с именем lpDMOName
//Перед вызовом этой функции необходимо вызвать UpdateDMOArray(...) (если она не вызывалась)
//В случае ошибки функция вернет значение меньше нуля
int CDirectShow::AddDMOToFilterGraph(LPCWSTR lpDMOName)
{
	if (!m_pGraphBuilder) return -1;
	if (!m_lDMOCount) return -1;
	IBaseFilter *pDMOWBaseFilter = NULL;
	IDMOWrapperFilter *pDMOWrapper;
	for (m_lCounter = 0; m_lCounter < m_lDMOCount; m_lCounter++)
	{
		if (_wcsicmp(m_lpDMONames[m_lCounter], lpDMOName) == 0)
		{
			if (FAILED(CoCreateInstance(CLSID_DMOWrapperFilter, NULL, CLSCTX_INPROC,
				IID_IBaseFilter, (LPVOID *)&pDMOWBaseFilter))) return -1;
			if (FAILED(pDMOWBaseFilter->QueryInterface(IID_IDMOWrapperFilter,
				(LPVOID *)&pDMOWrapper))) return -1;
			pDMOWrapper->Init(m_cDMOCLSIDs[m_lCounter], m_gidRecentDMOCat);
			pDMOWrapper->Release();
			m_pGraphBuilder->AddFilter(pDMOWBaseFilter, m_lpDMONames[m_lCounter]);
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
	LPWSTR lpDMOTmp[E_MAX_ARR_SIZE];
	for (m_lCounter = 0; m_lCounter < E_MAX_ARR_SIZE; m_lCounter++)
	{
		lpDMOTmp[m_lCounter] = NULL;
		if (m_lpDMONames[m_lCounter])
		{
			delete[] m_lpDMONames[m_lCounter];
			m_lpDMONames[m_lCounter] = NULL;
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
	pEnumDMO->Next(E_MAX_ARR_SIZE, &m_cDMOCLSIDs[0], &lpDMOTmp[0], &m_lDMOCount);
	if (!m_lDMOCount) return;
	for (m_lCounter = 0; m_lCounter < m_lDMOCount; m_lCounter++)
	{
		m_lpDMONames[m_lCounter] = new WCHAR[MAX_PATH];
		wcscpy(m_lpDMONames[m_lCounter], lpDMOTmp[m_lCounter]);
		CoTaskMemFree(lpDMOTmp[m_lCounter]);
	}
	pEnumDMO->Release();
}

//*bug workaround: в некоторых случаях информация приходит не в юникоде
//inline-функция для GetMediaContent
__inline void CheckAMMCString(BSTR bstrStr, LPWSTR lpOut, int intOutCharCnt) {
	int numbytes = (int)*((LPBYTE)bstrStr - 4);
	int flags = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK;
	if (!IsTextUnicode(bstrStr, numbytes, &flags))
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bstrStr, numbytes, lpOut, intOutCharCnt);
	else
		wcscpy(lpOut, bstrStr);
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
	WCHAR lpTmp[128];
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
		CheckAMMCString(bstrTmp, lpTmp, AS(lpTmp));
		wcsncpy(pMC->Author, lpTmp, AS(pMC->Author) - 1);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_Title(&bstrTmp)))
	{
		CheckAMMCString(bstrTmp, lpTmp, AS(lpTmp));
		wcsncpy(pMC->Title, lpTmp, AS(pMC->Title) - 1);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_Rating(&bstrTmp)))
	{
		CheckAMMCString(bstrTmp, lpTmp, AS(lpTmp));
		wcsncpy(pMC->Rating, lpTmp, AS(pMC->Rating) - 1);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_Copyright(&bstrTmp)))
	{
		CheckAMMCString(bstrTmp, lpTmp, AS(lpTmp));
		wcsncpy(pMC->Copyright, lpTmp, AS(pMC->Copyright) - 1);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_Description(&bstrTmp)))
	{
		CheckAMMCString(bstrTmp, lpTmp, AS(lpTmp));
		wcsncpy(pMC->Description, lpTmp, AS(pMC->Description) - 1);
		SysFreeString(bstrTmp);
	}
	if (SUCCEEDED(pAMMediaContent->get_MoreInfoText(&bstrTmp)))
	{
		CheckAMMCString(bstrTmp, lpTmp, AS(lpTmp));
		wcsncpy(pMC->MoreInfo, lpTmp, AS(pMC->MoreInfo) - 1);
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

	/*delete m_pEBGraphBuilder;
	m_pEBGraphBuilder = NULL;*/
}

//Приватная функция. Выводит сообщение об ошибке DirectShow (hex-код и описание)
void CDirectShow::DSErrorMsg(HRESULT hr, LPCWSTR lpEM)
{
	WCHAR lpMsg[512] = {};
	WCHAR lpErrDesc[MAX_ERROR_TEXT_LEN] = {};
	AMGetErrorText(hr, lpErrDesc, MAX_ERROR_TEXT_LEN);
	if (!lpEM) lpEM = m_lpFileName;
	swprintf(lpMsg, L"%s\n\nError code: 0x%08x\nError description: %s",
		lpEM, hr, wcslen(lpErrDesc)?lpErrDesc:L"None");
	MessageBox(m_hAppWnd, lpMsg, (m_lpAppName)?m_lpAppName:L"Error",
		MB_ICONEXCLAMATION);
}

//Приватная функция. Освобождает структуру AM_MEDIA_TYPE
void CDirectShow::FreeMediaType(AM_MEDIA_TYPE *pMT)
{
	if (pMT->cbFormat != 0)
	{
		CoTaskMemFree((LPVOID)pMT->pbFormat);
		pMT->cbFormat = 0;
		pMT->pbFormat = NULL;
	}
	SR(pMT->pUnk);
	CoTaskMemFree((LPVOID)pMT);
}
