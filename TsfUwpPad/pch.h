#pragma once
#include <Unknwn.h>
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.System.h>


#include <d2d1_3.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>


#include <vector>
#include <map>
#include <string>
#include <msctf.h>
#include <strsafe.h>
#include <functional>


void Trace(LPCWSTR fm, ...);

#define TRACE Trace