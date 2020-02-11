#pragma once

#include "D2DWinrt.h"
#include "libsrc/D2DContext.h"
//#include "../libsrc/D2DUniversalControlBase.h"


class D2DWinrtEx;
class TestWindow
{
	public :
		HRESULT DefWndProc(D2DWinrtEx* sender, UINT msg, INT_PTR wp, INT_PTR lp);
		HRESULT DefPaintWndProc(D2DWinrtEx* sender, UINT msg, INT_PTR wp, INT_PTR lp);

		void OnMouseMove(INT_PTR wParam, INT_PTR lParam);
		void OnLButtonDown(INT_PTR wParam, INT_PTR lParam);
		void OnLButtonUp(INT_PTR wParam, INT_PTR lParam);
		void AddReturn();

		UINT _uSelDragStart;
		bool IsCapture_;

};


class D2DWinrtEx : public D2DWinrt, public TestWindow //, public V5::D2DMainWindow
{
	public :
		D2DWinrtEx():bClose_(false){}

		void SetWindow(Windows::UI::Core::CoreWindow const& window) override;

		virtual V5::D2DContext* cxt();
		virtual void Close(){}

		virtual void OnDraw() override;
		virtual bool IsClose() { return bClose_; }

		virtual void SetIndependentResources(winrt::com_ptr<ID2D1Factory> fac, winrt::com_ptr<IDWriteFactory3> wfac) override;
		virtual void CreateDeviceResources(winrt::com_ptr<ID2D1DeviceContext> cxt) override;
		virtual void ReleaseDeviceResources() override;

		virtual void OnEntry() override;
		virtual void Redraw()  { D2DWinrt::bRedraw_ = true; }

	protected :
		void OnPointerPressed(IInspectable const &, PointerEventArgs const & args);
		void OnPointerMoved(IInspectable const &, PointerEventArgs const & args);
		void OnPointerReleased(IInspectable const &, PointerEventArgs const & args);
		void OnPointerWheelChanged(IInspectable const&, PointerEventArgs const& args);

		void OnKeyDown(IInspectable const &, KeyEventArgs const & args);
		void OnKeyUp(IInspectable const &, KeyEventArgs const & args);
	
		void OnSizeChanged(IInspectable const &, WindowSizeChangedEventArgs const & args);

	protected :
		
		bool bClose_;
		winrt::com_ptr<ID2D1DeviceContext> dcxt_;
		winrt::com_ptr<ID2D1Factory> fac_;
		winrt::com_ptr<IDWriteFactory3> wfac_;

		V5::D2DContext cxt_;
};