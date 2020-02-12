#include "pch.h"
#include "D2DWinrt.h"


using namespace winrt;
using namespace Windows;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::System;
using namespace Windows::Graphics::Display;

static D2DWinrt* pd2dwinrt = nullptr;

#define d2dwinrt (*pd2dwinrt)


namespace
{
    winrt::com_ptr<ID2D1Factory1> CreateFactory()
    {
        D2D1_FACTORY_OPTIONS options{};

#ifdef _DEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

        winrt::com_ptr<ID2D1Factory1> factory;

        winrt::check_hresult(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            options,
            factory.put()));

        return factory;
    }

    HRESULT CreateDevice(D3D_DRIVER_TYPE const type, winrt::com_ptr<ID3D11Device>& device)
    {
        WINRT_ASSERT(!device);

        return D3D11CreateDevice(
            nullptr,
            type,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            nullptr, 0,
            D3D11_SDK_VERSION,
            device.put(),
            nullptr,
            nullptr);
    }

    winrt::com_ptr<ID3D11Device> CreateDevice()
    {
        winrt::com_ptr<ID3D11Device> device;
        HRESULT hr{ CreateDevice(D3D_DRIVER_TYPE_HARDWARE, device) };

        if (DXGI_ERROR_UNSUPPORTED == hr)
        {
            hr = CreateDevice(D3D_DRIVER_TYPE_WARP, device);
        }

        winrt::check_hresult(hr);
        return device;
    }

    winrt::com_ptr<ID2D1DeviceContext> CreateRenderTarget(
        winrt::com_ptr<ID2D1Factory1> const& factory,
        winrt::com_ptr<ID3D11Device> const& device)
    {
        WINRT_ASSERT(factory);
        WINRT_ASSERT(device);

        winrt::com_ptr<IDXGIDevice> const dxdevice{ device.as<IDXGIDevice>() };

        winrt::com_ptr<ID2D1Device> d2device;
        winrt::check_hresult(factory->CreateDevice(dxdevice.get(), d2device.put()));

        winrt::com_ptr<ID2D1DeviceContext> target;
        winrt::check_hresult(d2device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, target.put()));
        return target;
    }

    winrt::com_ptr<IDXGIFactory2> GetDxgiFactory(winrt::com_ptr<ID3D11Device> const& device)
    {
        WINRT_ASSERT(device);

        winrt::com_ptr<IDXGIDevice> const dxdevice{ device.as<IDXGIDevice>() };

        winrt::com_ptr<IDXGIAdapter> adapter;
        winrt::check_hresult(dxdevice->GetAdapter(adapter.put()));

        winrt::com_ptr<IDXGIFactory2> factory;
        factory.capture(adapter, &IDXGIAdapter::GetParent);
        return factory;
    }

    void CreateDeviceSwapChainBitmap(
        winrt::com_ptr<IDXGISwapChain1> const& swapchain,
        winrt::com_ptr<ID2D1DeviceContext> const& target)
    {
        WINRT_ASSERT(swapchain);
        WINRT_ASSERT(target);

        winrt::com_ptr<IDXGISurface> surface;
        surface.capture(swapchain, &IDXGISwapChain1::GetBuffer, 0);

        D2D1_BITMAP_PROPERTIES1 const props{ D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)) };

        winrt::com_ptr<ID2D1Bitmap1> bitmap;

        winrt::check_hresult(target->CreateBitmapFromDxgiSurface(surface.get(),
            props,
            bitmap.put()));

        target->SetTarget(bitmap.get());
    }

    winrt::com_ptr<IDXGISwapChain1> CreateSwapChainForCoreWindow(winrt::com_ptr<ID3D11Device> const& device)
    {
        WINRT_ASSERT(device);

        winrt::com_ptr<IDXGIFactory2> const factory{ GetDxgiFactory(device) };

        DXGI_SWAP_CHAIN_DESC1 props{};
        props.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        props.SampleDesc.Count = 1;
        props.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        props.BufferCount = 2;
        props.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

        winrt::com_ptr<IDXGISwapChain1> swapChain;

        winrt::check_hresult(factory->CreateSwapChainForCoreWindow(
            device.get(),
            winrt::get_unknown(CoreWindow::GetForCurrentThread()),
            &props,
            nullptr, // all or nothing
            swapChain.put()));

        return swapChain;
    }
}

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    winrt::com_ptr<ID2D1Factory1> m_factory;
    winrt::com_ptr<ID2D1DeviceContext> m_target;
    winrt::com_ptr<IDXGISwapChain1> m_swapChain;

    float m_dpi{};

    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const&)
    {
        auto& w = D2DWinrt::CreeateInstance();

        pd2dwinrt = &w;



    }

    void Load(hstring const&)
    {
        CoreWindow const window{ CoreWindow::GetForCurrentThread() };

        window.SizeChanged([&](auto&&...)
            {
                if (m_target)
                {
                    ResizeSwapChainBitmap();
                    Render();
                }
            });

        DisplayInformation const display{ DisplayInformation::GetForCurrentView() };
        m_dpi = display.LogicalDpi();

        display.DpiChanged([&](DisplayInformation const& display, IInspectable const&)
            {
                if (m_target)
                {
                    m_dpi = display.LogicalDpi();
                    m_target->SetDpi(m_dpi, m_dpi);

                    Render();
                }
            });

        m_factory = CreateFactory();
        auto wfac = CreateDeviceIndependentResources();

        d2dwinrt.SetIndependentResources(m_factory, wfac);

        InnerCreateRenderTarget();


        d2dwinrt.OnEntry();

    }

    void InnerCreateRenderTarget()
    {
        winrt::com_ptr<ID3D11Device> const device{ CreateDevice() };
        m_target = CreateRenderTarget(m_factory, device);
        m_swapChain = CreateSwapChainForCoreWindow(device);

        CreateDeviceSwapChainBitmap(m_swapChain, m_target);

        m_target->SetDpi(m_dpi, m_dpi);

        d2dwinrt.CreateDeviceResources(m_target);
    }


    void Uninitialize()
    {
    }

    void Run()
    {
        CoreWindow const window{ CoreWindow::GetForCurrentThread() };
        window.Activate();


        while (!d2dwinrt.IsClose())
        {
            window.Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

            if (d2dwinrt.IsRedraw())
                Render();

        }

        d2dwinrt.Close();

        window.Close();
    }

    void SetWindow(CoreWindow const& window)
    {
        d2dwinrt.SetWindow(window);

    }


    void Render()
    {
        if (!m_target)
        {
            InnerCreateRenderTarget();
        }

        m_target->BeginDraw();

        d2dwinrt.OnDraw();

        m_target->EndDraw();

        HRESULT const hr{ m_swapChain->Present(1, 0) };

        if (S_OK != hr && DXGI_STATUS_OCCLUDED != hr)
        {
            ReleaseDevice();
        }
    }

    void ReleaseDevice()
    {
        m_target = nullptr;
        m_swapChain = nullptr;

        ReleaseDeviceResources();
    }

    void ResizeSwapChainBitmap()
    {
        WINRT_ASSERT(m_target);
        WINRT_ASSERT(m_swapChain);

        m_target->SetTarget(nullptr);

        if (S_OK == m_swapChain->ResizeBuffers(0, // all buffers
            0, 0, // client area
            DXGI_FORMAT_UNKNOWN, // preserve format
            0)) // flags
        {
            CreateDeviceSwapChainBitmap(m_swapChain, m_target);

        }
        else
        {
            ReleaseDevice();
        }
    }

    winrt::com_ptr<::IDWriteFactory3> CreateDeviceIndependentResources()
    {
        winrt::com_ptr<::IUnknown> pun;
        auto hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), pun.put());
        WINRT_ASSERT(hr == 0);

        return pun.as<IDWriteFactory3>();
    }

    void ReleaseDeviceResources()
    {
        d2dwinrt.ReleaseDeviceResources();
    }

};

RECT uwpGetClientRect()
{
    auto bnd = CoreWindow::GetForCurrentThread().Bounds(); // DIP
    auto view = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
    float dpi = view.LogicalDpi();
    auto f = [dpi](LONG a)->LONG
    {
        return (LONG)(a * dpi / 96.0f);
    };

    RECT r={0};
    r.right = f(bnd.Width);
    r.bottom = f(bnd.Height);
    return r;
}
RECT uwpClientToScreen(RECT rc)
{
    //DIP value = (physical pixel x 96) / DPI
    //physical pixel value = (DIP x DPI) / 96

    auto bnd = CoreWindow::GetForCurrentThread().Bounds(); // DIP

    auto view = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();

    float dpi = view.LogicalDpi(); //<-- "—v‘f‚ªŒ©‚Â‚©‚è‚Ü‚¹‚ñB"‚Ìwarning

    rc.left += (LONG)bnd.X;
    rc.top += (LONG)bnd.Y;
    rc.right += (LONG)bnd.X;
    rc.bottom += (LONG)bnd.Y;

    auto f = [dpi](LONG a)->LONG
    {
        return (LONG)(a * dpi / 96.0f);
    };

    rc.left = f(rc.left);
    rc.top = f(rc.top);
    rc.right = f(rc.right);
    rc.bottom = f(rc.bottom);

    return rc; // physical pixel
}

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(winrt::make<App>());
}


