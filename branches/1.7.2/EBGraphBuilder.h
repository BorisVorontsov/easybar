#ifndef EBGRAPHBUILDER_H
#define EBGRAPHBUILDER_H

class CEBGraphBuilder : public /*IUnknown, */IStdMarshalInfo
{
public:
	CEBGraphBuilder();
	~CEBGraphBuilder();
	void AttachGraph(IGraphBuilder *pGraph);
	void DetachGraph();
	HRESULT CreateMarshal();
	void DeleteMarshal();

	//IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

	//IStdMarshalInfo
	HRESULT STDMETHODCALLTYPE GetClassForHandler(DWORD dwDestContext, void *pvDestContext, CLSID *pClsid);
protected:
	//
private:
	IGraphBuilder *m_pAttachedGB;
	IMarshal *m_pStdMarshal;
	UINT m_nRefCntr;
};

#endif