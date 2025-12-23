#include "pch.h"
#include "WinRTXamlPropSheet.h"
#include "XamlWindow.h"
#include "Resource.h"

namespace winrt
{
    using namespace Windows::UI::Xaml;
    using namespace Windows::UI::Xaml::Controls;
    using namespace Windows::UI::Xaml::Hosting;
}

extern HMODULE g_hModule;

WinRTXamlPropSheet::WinRTXamlPropSheet()
{
    MessageBoxW(nullptr, L"COM Class inited", L"", MB_OK);
}

//IShellExtInit
HRESULT __stdcall WinRTXamlPropSheet::Initialize(PCIDLIST_ABSOLUTE, IDataObject* pDataObj, HKEY hkeyProgID)
{
    MessageBoxW(nullptr, L"Initialize called!", L"", MB_OK);
    if(pDataObj == nullptr)
    { return E_INVALIDARG; }

    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stm{0};

    HRESULT hr = pDataObj->GetData(&fe, &stm);
    if (SUCCEEDED(hr))
    {
        HDROP hDrop = (HDROP)GlobalLock(stm.hGlobal);
        if (hDrop)
        {
            files.count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
            files.list = new wchar_t*[files.count]{};
            for (UINT i = 0; i < files.count; ++i)
            {
                files.list[i] = new wchar_t[MAX_PATH]();
                if (DragQueryFile(hDrop, i, files.list[i], MAX_PATH))
                { ++m_Effectives; }
            } GlobalUnlock(stm.hGlobal);
        }
        ReleaseStgMedium(&stm);
        return m_Effectives != 0 ? S_OK : E_FAIL;
    } return hr;
}

//IShellPropSheetExt
HRESULT __stdcall WinRTXamlPropSheet::AddPages(LPFNSVADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    MessageBoxW(nullptr, L"AddPages called", L"", MB_OK);

    FilePathList* newList = new FilePathList{};
    newList->list = new wchar_t*[m_Effectives]{};
    newList->count = m_Effectives;
    UINT completed = 0;
    for (UINT i = 0; i < files.count; ++i)
    {
        if (files.list[i] != nullptr && files.list[i][0] != L'\0')
        {
            newList->list[completed] = new wchar_t[MAX_PATH]();
            wcscpy_s(newList->list[completed], MAX_PATH, files.list[i]);
            ++completed;
        }
    }

    PROPSHEETPAGE  psp;
    HPROPSHEETPAGE hPage;

    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_USETITLE | PSP_USECALLBACK;
    psp.pfnCallback = WinRTXamlPropSheet::MyPropPageCallback;
    psp.pfnDlgProc = WinRTXamlPropSheet::MyPropPageProc;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_EMPTYPROPPAGE);

    psp.hInstance = g_hModule;
    psp.pszTitle = L"Sample Sheet";
    psp.lParam = (LPARAM)newList;

    hPage = CreatePropertySheetPage(&psp);
    HRESULT hr = E_FAIL;

    if (hPage)
    {
        if (pfnAddPage(hPage, lParam))
        { return S_OK; }
        else { DestroyPropertySheetPage(hPage); }
    } else { hr = E_OUTOFMEMORY; }
    
    for (UINT i = 0; i < newList->count; ++i)
    { delete[] newList->list[i]; }
    delete[] newList->list;
    delete newList;
    return hr;
}

HRESULT __stdcall WinRTXamlPropSheet::ReplacePage(EXPPS, LPFNSVADDPROPSHEETPAGE, LPARAM)
{ return E_NOTIMPL; }

//Static Methods
INT_PTR CALLBACK WinRTXamlPropSheet::MyPropPageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static std::map<HWND, XamlWindow*> mappingList{};
    static std::map<HWND, std::mutex> mutexList{};

    switch (uMsg)
    {
        case WM_INITDIALOG:
        try
        {
            MessageBoxW(nullptr, L"WM_INITDIALOG", L"", MB_OK);

            if (s_InstCount == 0)
            {
                HRESULT hr = XamlWindow::RegisterWindowClass();
                MessageBoxW(nullptr, winrt::to_hstring(static_cast<int>(hr)).c_str(), L"", MB_OK);
            }

            ++s_InstCount;
            PROPSHEETPAGE* ppsp = reinterpret_cast<PROPSHEETPAGE*>(lParam);
            FilePathList* data = reinterpret_cast<FilePathList*>(ppsp->lParam);
            XamlWindow* window = nullptr;
            XamlWindow::CreateXamlWindow(&window);
            auto iterator = mappingList.try_emplace(hDlg, window).first;
            mutexList.try_emplace(hDlg);
            for (UINT i = 0; i < data->count; ++i)
            {
                MessageBoxW(nullptr, data->list[i], L"", MB_OK);
            }

            auto testControl = winrt::TextBox();
            testControl.AcceptsReturn(true);
            testControl.TextWrapping(winrt::TextWrapping::Wrap);
            iterator->second->Content(testControl);

            MessageBoxW(nullptr, L"return TRUE", L"", MB_OK);
            return TRUE;
        }
        catch (winrt::hresult_error const& ex)
        { MessageBoxW(nullptr, ex.message().c_str(), winrt::to_hstring(ex.code()).c_str(), MB_OK); break; }

        case WM_SIZE:
        {
            MessageBoxW(nullptr, L"WM_SIZE", L"", MB_OK);
            mappingList.find(hDlg)->second->AttachToWindow(hDlg);
            return 0;
        }

        case WM_DESTROY:
        {
            MessageBoxW(nullptr, L"WM_DESTROY", L"", MB_OK);
            //std::lock_guard guard(mutexList.find(hDlg)->second);
            auto iterator = mappingList.find(hDlg);
            if (iterator != mappingList.end())
            {
                iterator->second->Close();
                MessageBoxW(nullptr, L"Window closed", L"", MB_OK);
                delete iterator->second;
                mappingList.erase(iterator);
            }

            --s_InstCount;
            if (s_InstCount == 0)
            {
                HRESULT hr = XamlWindow::UnregisterWindowClass();
                MessageBoxW(nullptr, winrt::to_hstring(static_cast<int>(hr)).c_str(), L"", MB_OK);
            }

            break;
        }

        default: break;
    } return FALSE;
}

UINT CALLBACK WinRTXamlPropSheet::MyPropPageCallback(HWND, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    FilePathList* data = reinterpret_cast<FilePathList*>(ppsp->lParam);

    switch (uMsg)
    {
        case PSPCB_CREATE:
        {
            return 1;
        }

        case PSPCB_RELEASE:
        {
            for (UINT i = 0; i < data->count; ++i)
            { delete[] data->list[i]; }
            delete[] data->list;
            delete data;
        }
    }
    return 1;
}

WinRTXamlPropSheet::~WinRTXamlPropSheet()
{
    for (UINT i = 0; i < files.count; ++i)
    { delete[] files.list[i]; }
    delete[] files.list;
}

std::mutex WinRTXamlPropSheet::s_mutex{};
uint64_t WinRTXamlPropSheet::s_InstCount = 0;
