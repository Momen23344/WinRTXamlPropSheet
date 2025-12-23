#pragma once
//Windows
#define NOMINMAX
#include <Unknwn.h>
#include <Prsht.h>
#include <objidl.h>
#include <ShObjIdl_core.h>
#include <WinUser.h>
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#pragma comment(lib, "Comctl32.lib")

//WinRT
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

//Standard Library
#include <mutex>