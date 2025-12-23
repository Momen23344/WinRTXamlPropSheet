#pragma once

class XamlWindow final
{
public:
    static HRESULT CreateXamlWindow(XamlWindow**);

    winrt::Windows::Foundation::IInspectable Content();
    void Content(winrt::Windows::Foundation::IInspectable const&);

    HRESULT AttachToWindow(HWND);
    HRESULT UpdateSize();

    void Close();

    static HRESULT RegisterWindowClass();
    static HRESULT UnregisterWindowClass();

protected:
    XamlWindow(HWND, winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource const&);

private:
    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource m_XamlSource = nullptr;
    winrt::Windows::UI::Xaml::Controls::ContentPresenter m_Presenter = nullptr;

    HWND m_InternalWindowHandle = nullptr;
    HWND m_ExternalWindowHandle = nullptr;

    bool m_IsClosed = true;
    std::mutex m_mutex;

    HRESULT UpdateSizeInternal();

    static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager s_XamlMgr;
};