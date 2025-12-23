#include "pch.h"
#include "XamlWindow.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::UI::Xaml;
    using namespace Windows::UI::Xaml::Controls;
    using namespace Windows::UI::Xaml::Hosting;
}

extern HMODULE g_hModule;
winrt::WindowsXamlManager g_XamlMgr = nullptr;

constexpr wchar_t s_WindowClassName[18] = L"DesktopXamlWindow";
WNDCLASSEXW s_WindowClass{0};
bool s_IsClassRegistered = false;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT XamlWindow::CreateXamlWindow(XamlWindow** ppOut)
{
    if (ppOut == nullptr) { return E_INVALIDARG; }
    HWND hwnd = CreateWindowExW(0, s_WindowClassName, L"XAML Host Window", WS_OVERLAPPEDWINDOW,
        0, 0, 0, 0,
        nullptr, nullptr, g_hModule, nullptr);
    if (hwnd == nullptr)
    { return HRESULT_FROM_WIN32(GetLastError()); }
    MessageBoxW(nullptr, L"Try to init XamlMgr", L"", MB_OK);
    if (!s_XamlMgr)
    { s_XamlMgr = winrt::WindowsXamlManager::InitializeForCurrentThread(); }
    MessageBoxW(nullptr, L"Try to init XamlSource", L"", MB_OK);
    auto xamlSource = winrt::DesktopWindowXamlSource();
    auto pSourceNative = xamlSource.as<IDesktopWindowXamlSourceNative>();
    HRESULT hr1 = pSourceNative->AttachToWindow(hwnd);
    HWND xamlframe = nullptr;
    HRESULT hr2 = pSourceNative->get_WindowHandle(&xamlframe);
    SetLastError(0);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(xamlframe));
    HRESULT hr3 = HRESULT_FROM_WIN32(GetLastError());
    auto hr1failed = FAILED(hr1);
    auto hr2failed = FAILED(hr2);
    if (hr1failed || hr2failed || FAILED(hr3))
    {
        xamlSource.Close();
        //xamlMgr.Close();
        DestroyWindow(hwnd);
        if (hr1failed) { return hr1; }
        return hr2failed ? hr2 : hr3;
    }
    *ppOut = new XamlWindow(hwnd, xamlSource);
    ShowWindow(xamlframe, SW_SHOW);
    return S_OK;
}

XamlWindow::XamlWindow(HWND hwnd, winrt::DesktopWindowXamlSource const& xamlSource) :
    m_IsClosed(false), m_InternalWindowHandle(hwnd),
    m_XamlSource(xamlSource), m_Presenter(winrt::ContentPresenter())
{
    m_XamlSource.Content(m_Presenter);
}

winrt::IInspectable XamlWindow::Content()
{
    std::lock_guard guard{m_mutex};
    if (m_IsClosed)
    { winrt::check_hresult(RO_E_CLOSED); }
    return m_Presenter.Content();
}

void XamlWindow::Content(winrt::IInspectable const& value)
{
    std::lock_guard guard{m_mutex};
    if (m_IsClosed)
    { winrt::check_hresult(RO_E_CLOSED); }
    m_Presenter.Content(value);
}

HRESULT XamlWindow::AttachToWindow(HWND hwnd)
{
    std::lock_guard guard{m_mutex};
    if (m_IsClosed)
    { return RO_E_CLOSED; }
    if (hwnd == nullptr)
    { return E_INVALIDARG; }
    if (m_ExternalWindowHandle != hwnd)
    {
        SetLastError(0);
        LONG_PTR oldStyle = GetWindowLongPtrW(m_InternalWindowHandle, GWL_STYLE);
        HWND oldParent = SetParent(m_InternalWindowHandle, hwnd);
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(hr)) { return hr; }
        SetLastError(0);
        SetWindowLongPtrW(m_InternalWindowHandle, GWL_STYLE, WS_CHILD);
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(hr))
        {
            SetParent(m_InternalWindowHandle, oldParent);
            SetWindowLongPtrW(m_InternalWindowHandle, GWL_STYLE, oldStyle);
            return hr;
        }
        m_ExternalWindowHandle = hwnd;
        ShowWindow(m_InternalWindowHandle, SW_SHOW);
    } return UpdateSizeInternal();
}

HRESULT XamlWindow::UpdateSize()
{
    std::lock_guard guard{m_mutex};
    if (m_IsClosed)
    { return RO_E_CLOSED; }
    if (m_ExternalWindowHandle == nullptr)
    { return S_FALSE; }
    return UpdateSizeInternal();
}

HRESULT XamlWindow::UpdateSizeInternal()
{
    RECT rect;
    GetClientRect(m_ExternalWindowHandle, &rect);
    SetWindowPos(m_InternalWindowHandle, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);
    UpdateWindow(m_InternalWindowHandle);
    return S_OK;
}

void XamlWindow::Close()
{
    std::lock_guard guard{m_mutex};
    if (!m_IsClosed)
    {
        m_IsClosed = true;
        m_XamlSource.Close();
        DestroyWindow(m_InternalWindowHandle);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_SIZE:
        {
            HWND sub = reinterpret_cast<HWND>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            RECT rect;
            GetClientRect(hwnd, &rect);
            SetWindowPos(sub, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
            return 0;
        }

        case WM_DESTROY:
            //PostQuitMessage(0);
            return 0;

        case WM_NCDESTROY:
        {
            MessageBoxW(nullptr, L"WM_NCDESTROY", L"", MB_OK);
            return 0;
        }

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

HRESULT XamlWindow::RegisterWindowClass()
{
    if (!s_IsClassRegistered)
    {
        s_WindowClass.cbSize = sizeof(WNDCLASSEXW);
        s_WindowClass.lpfnWndProc = WndProc;
        s_WindowClass.hInstance = g_hModule;
        s_WindowClass.lpszClassName = s_WindowClassName;
        SetLastError(0);
        if (RegisterClassExW(&s_WindowClass) == 0)
        { return HRESULT_FROM_WIN32(GetLastError()); }
        else { s_IsClassRegistered = true; return S_OK; }
    } else { return S_FALSE; }

}

HRESULT XamlWindow::UnregisterWindowClass()
{
    if (s_IsClassRegistered)
    {
        SetLastError(0);
        if (UnregisterClassW(s_WindowClassName, g_hModule) == 0)
        { return HRESULT_FROM_WIN32(GetLastError()); }
        else { s_IsClassRegistered = false; return S_OK; }
    } else { return S_FALSE; }
}

winrt::WindowsXamlManager XamlWindow::s_XamlMgr = nullptr;
