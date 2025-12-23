#pragma once

struct FilePathList
{
    wchar_t** list;
    unsigned int count;
};

class WinRTXamlPropSheet : public winrt::implements< WinRTXamlPropSheet, IShellExtInit, IShellPropSheetExt>
{
public:
    WinRTXamlPropSheet();

    //IShellExtInit
    HRESULT __stdcall Initialize(PCIDLIST_ABSOLUTE, IDataObject*, HKEY);

    //IShellPropSheetExt
    HRESULT __stdcall AddPages(LPFNSVADDPROPSHEETPAGE, LPARAM);
    HRESULT __stdcall ReplacePage(EXPPS, LPFNSVADDPROPSHEETPAGE, LPARAM);

    ~WinRTXamlPropSheet();

private:
    FilePathList files;
    UINT m_Effectives = 0;
    UINT m_PageCount = 0;

    static std::mutex s_mutex;
    static uint64_t s_InstCount;

    static INT_PTR CALLBACK MyPropPageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static UINT CALLBACK MyPropPageCallback(HWND, UINT uMsg, LPPROPSHEETPAGE ppsp);
};