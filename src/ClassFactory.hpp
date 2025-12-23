#include "pch.h"

template<typename T>
class ClassFactory : public winrt::implements<ClassFactory<T>, IClassFactory>
{
public:
    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
    {
        try
        {
            *ppvObject = nullptr;
            if (pUnkOuter)
            {
                return CLASS_E_NOAGGREGATION;
            }
            return winrt::make<T>().as(riid, ppvObject);
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    HRESULT __stdcall LockServer(BOOL)
    {
        return S_OK;
    }
};