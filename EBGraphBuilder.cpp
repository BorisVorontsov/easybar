#include <windows.h>
#include <dshow.h>
#include <objidl.h>

#include "ebgraphbuilder.h"

// {FACCB70B-A915-4116-BB37-78610D33367C}
static const CLSID CLSID_EBGraphBuilder = 
{ 0xfaccb70b, 0xa915, 0x4116, { 0xbb, 0x37, 0x78, 0x61, 0xd, 0x33, 0x36, 0x7c } };


CEBGraphBuilder::CEBGraphBuilder()
{
	//CoInitialize(NULL);

	m_pAttachedGB = NULL;
	m_pStdMarshal = NULL;
	m_nRefCntr = 0;
}

CEBGraphBuilder::~CEBGraphBuilder()
{
	if (m_pStdMarshal)
		m_pStdMarshal->Release();

	//CoUninitialize();
}

void CEBGraphBuilder::AttachGraph(IGraphBuilder *pGraph)
{
	if (m_pAttachedGB)
		DetachGraph();
	m_pAttachedGB = pGraph;
}

void CEBGraphBuilder::DetachGraph()
{
	m_pAttachedGB = NULL;
}

HRESULT CEBGraphBuilder::CreateMarshal()
{
	if (!m_pAttachedGB) return E_FAIL;
	if (m_pStdMarshal)
		DeleteMarshal();
	return CoGetStandardMarshal(IID_IGraphBuilder, m_pAttachedGB, MSHCTX_NOSHAREDMEM, NULL, MSHLFLAGS_TABLESTRONG, &m_pStdMarshal);
}

void CEBGraphBuilder::DeleteMarshal()
{
	if (m_pStdMarshal)
		m_pStdMarshal->Release();
}

//---------------------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE CEBGraphBuilder::QueryInterface(REFIID riid, void **ppvObject)
{
	if (riid == IID_IUnknown)
	{
		/*this::*/AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == IID_IGraphBuilder)
	{
		if (!m_pAttachedGB)
		{
			*ppvObject = NULL;
			return E_FAIL;
		}
		else
		{
			m_pAttachedGB->AddRef();
			*ppvObject = m_pAttachedGB;
			return S_OK;
		}
	}
	else if (riid == IID_IMarshal)
	{
		if (!m_pStdMarshal)
		{
			*ppvObject = NULL;
			return E_FAIL;
		}
		else
		{
			m_pStdMarshal->AddRef();
			*ppvObject = m_pStdMarshal;
			return S_OK;
		}
	}
	else if (riid == IID_IStdMarshalInfo)
	{
		/*this::*/AddRef();
		*ppvObject = this;
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CEBGraphBuilder::AddRef(void)
{
	return ++m_nRefCntr;
}

ULONG STDMETHODCALLTYPE CEBGraphBuilder::Release(void)
{
	if (m_nRefCntr)
	{
		if ((--m_nRefCntr) == 0) {
			//
		}
	}

	return m_nRefCntr;
}

//---------------------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE CEBGraphBuilder::GetClassForHandler(DWORD dwDestContext, void *pvDestContext, CLSID *pClsid)
{
	memcpy(pClsid, &CLSID_EBGraphBuilder, sizeof(CLSID_EBGraphBuilder));
	return S_OK;
}