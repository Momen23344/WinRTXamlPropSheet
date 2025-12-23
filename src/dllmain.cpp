#include "pch.h"
#include "WinRTXamlPropSheet.h"
#include "ClassFactory.hpp"

HMODULE g_hModule = NULL;

namespace winrt
{
    using namespace Windows::UI::Xaml::Hosting;
}

extern winrt::WindowsXamlManager g_XamlMgr;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //winrt::init_apartment();
        g_hModule = hModule;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        if (g_XamlMgr)
        { g_XamlMgr.Close(); }
        //winrt::uninit_apartment();
        break;
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    try
    {
        *ppv = nullptr;
        return winrt::make<ClassFactory<WinRTXamlPropSheet>>().as(riid, ppv);
    }
    catch (...)
    {
        return winrt::to_hresult();
    }
}

STDAPI DllCanUnloadNow()
{
    return winrt::get_module_lock() ? S_FALSE : S_OK;
}
