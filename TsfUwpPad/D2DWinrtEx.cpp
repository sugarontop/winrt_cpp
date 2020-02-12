#include "pch.h"
#include "D2DWinrtEx.h"
#include "libsrc/D2DLParam.h"
#include "libsrc/caret.h"

using namespace winrt;
using namespace Windows;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::System;


using namespace V5;

#define BLINK_DURATION 500 //ms
std::function<void()> g_caret_blick_func;

D2DWinrt& D2DWinrt::CreeateInstance()
{
    //
    // caret event timer
    //
    auto controller{ Windows::System::DispatcherQueueController::CreateOnDedicatedThread()};
    auto q = controller.DispatcherQueue();
    static Windows::System::DispatcherQueueTimer caret_blink_timer = q.CreateTimer();
    caret_blink_timer.Interval(std::chrono::milliseconds{ BLINK_DURATION });
    caret_blink_timer.Tick([](auto const& , auto const& )
    {
        if (g_caret_blick_func)
	        g_caret_blick_func();
    });
    caret_blink_timer.Start();

    
    
    //
    // main window
    //
    static D2DWinrtEx ex;
	return ex;
}

void D2DWinrtEx::SetWindow(CoreWindow const& window) 
{
    window.PointerPressed({ this, &D2DWinrtEx::OnPointerPressed });
    window.PointerMoved({ this, &D2DWinrtEx::OnPointerMoved });
    window.PointerReleased({ this, &D2DWinrtEx::OnPointerReleased });
    window.PointerWheelChanged({ this, &D2DWinrtEx::OnPointerWheelChanged });

    window.KeyDown( {this, &D2DWinrtEx::OnKeyDown });
    window.KeyUp( {this, &D2DWinrtEx::OnKeyUp });

    window.SizeChanged( {this, &D2DWinrtEx::OnSizeChanged });
    
}



void D2DWinrtEx::OnSizeChanged(IInspectable const &, WindowSizeChangedEventArgs const & args)
{
    V5::SizeChangeEvent ev;
        
    ev.sz = args.Size();
    
    DefWndProc( this, WM_SIZE,(INT_PTR)&ev, (INT_PTR)&ev );

    bRedraw_ = true;
}


void D2DWinrtEx::OnPointerPressed(IInspectable const &, PointerEventArgs const & args)
{
    winrt::Windows::UI::Input::PointerPoint p = args.CurrentPoint();

    V5::MouseEvent ev;
    ev.pt = FPointF(p.RawPosition().X,p.RawPosition().Y);
    ev.bLeft = (int)p.Properties().PointerUpdateKind(); // 1: lbuttondown
    
    if (PointerUpdateKind::LeftButtonPressed == p.Properties().PointerUpdateKind())
        DefWndProc(this, WM_LBUTTONDOWN, (INT_PTR)&ev, (INT_PTR)&ev);
    else if (PointerUpdateKind::RightButtonPressed == p.Properties().PointerUpdateKind())
        DefWndProc(this, WM_RBUTTONDOWN, (INT_PTR)&ev, (INT_PTR)&ev);

    bRedraw_ = true;
}
void D2DWinrtEx::OnPointerMoved(IInspectable const &, PointerEventArgs const & args)
{
     winrt::Windows::UI::Input::PointerPoint p = args.CurrentPoint();

    V5::MouseEvent ev;
    ev.pt = FPointF(p.RawPosition().X,p.RawPosition().Y);
    ev.bLeft = 0;
    ev.kind = (int)p.Properties().PointerUpdateKind(); // 1: lbuttondown
    

    DefWndProc( this, WM_MOUSEMOVE,(INT_PTR)&ev, (INT_PTR)&ev );
}
void D2DWinrtEx::OnPointerReleased(IInspectable const &, PointerEventArgs const & args)
{
    winrt::Windows::UI::Input::PointerPoint p = args.CurrentPoint();

    V5::MouseEvent ev;
    ev.pt = FPointF(p.RawPosition().X,p.RawPosition().Y);
    ev.bLeft = (int)p.Properties().PointerUpdateKind(); // 1: lbuttondown

    if (PointerUpdateKind::LeftButtonReleased == p.Properties().PointerUpdateKind())
        DefWndProc(this, WM_LBUTTONUP, (INT_PTR)&ev, (INT_PTR)&ev);
    else if (PointerUpdateKind::RightButtonReleased == p.Properties().PointerUpdateKind())
        DefWndProc(this, WM_RBUTTONUP, (INT_PTR)&ev, (INT_PTR)&ev);

    bRedraw_ = true;
}
void D2DWinrtEx::OnPointerWheelChanged(IInspectable const&, PointerEventArgs const& args)
{
    winrt::Windows::UI::Input::PointerPoint p = args.CurrentPoint(); 
    
    V5::MouseEvent ev;
    ev.pt = FPointF(p.RawPosition().X, p.RawPosition().Y);    
    ev.wheel_delta = p.Properties().MouseWheelDelta();

    DefWndProc(this, WM_MOUSEHWHEEL, (INT_PTR)&ev, (INT_PTR)&ev);

    bRedraw_ = true;
}

void D2DWinrtEx::OnKeyDown(IInspectable const &, KeyEventArgs const & args)
{
    V5::KeyEvent ev;
    auto win = CoreWindow::GetForCurrentThread();
    ev.IsPressShift = ((int)win.GetKeyState(VirtualKey::Shift) != 0);       //winrt::Windows::System::VirtualKey
    ev.IsPressControl = ((int)win.GetKeyState(VirtualKey::Control) != 0);

    winrt::Windows::System::VirtualKey k = args.VirtualKey();
    ev.key = (int)k;
    DefWndProc( this, WM_KEYDOWN,(INT_PTR)0,(INT_PTR)&ev);
    bRedraw_ = true;
}
void D2DWinrtEx::OnKeyUp(IInspectable const &, KeyEventArgs const & args)
{
    V5::KeyEvent ev;
    auto win = CoreWindow::GetForCurrentThread();
    ev.IsPressShift = ((int)win.GetKeyState(VirtualKey::Shift) != 0);       //winrt::Windows::System::VirtualKey
    ev.IsPressControl = ((int)win.GetKeyState(VirtualKey::Control) != 0);

    winrt::Windows::System::VirtualKey k = args.VirtualKey();
    ev.key = (int)k;
    DefWndProc( this, WM_KEYUP,(INT_PTR)0,(INT_PTR)&ev);
    bRedraw_ = true;
}


V5::D2DContext* D2DWinrtEx::cxt()
{ 
	return &(D2DWinrtEx::cxt_); 
}
void D2DWinrtEx::SetIndependentResources(winrt::com_ptr<ID2D1Factory> fac, winrt::com_ptr<IDWriteFactory3> wfac)
{
    fac_ = fac;
    wfac_ = wfac;


    

}

void D2DWinrtEx::CreateDeviceResources(winrt::com_ptr<ID2D1DeviceContext> cxt)
{
    SingletonD2DInstance s;

    ComPTR<ID2D1Factory> fac; fac.Attach(fac_.get());
    ComPTR<IDWriteFactory> wfac; wfac.Attach(wfac_.get());
    ComPTR<ID2D1DeviceContext> ccxt; ccxt.Attach(cxt.get());
    
    cxt_.Init(s,fac, wfac, ccxt);


    cxt_.CreateDeviceResources(cxt.get());
}

void D2DWinrtEx::ReleaseDeviceResources()
{
    cxt_.DestroyRenderTargetResource();
}

void D2DWinrtEx::Close()
{
    DefWndProc(this, WM_DESTROY, (INT_PTR)0, (INT_PTR)0);

}



void D2DWinrtEx::OnDraw()
{
    constexpr D2D1_COLOR_F color_white{ 1.0f,  1.0f,  1.0f,  1.0f };
   
    cxt_.cxt->Clear(color_white);

    DefPaintWndProc( this, WM_PAINT,0,0 );

    auto bCaret = Caret::GetCaret().Draw( cxt_ );
 
    bRedraw_ = (bRedraw_ || bCaret);
}

void OnEntry(D2DWinrtEx* win);


void D2DWinrtEx::OnEntry()
{
    ::OnEntry(this);
}

