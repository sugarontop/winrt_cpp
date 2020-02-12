#pragma once

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Core;



class D2DWinrt
{
	public :
		D2DWinrt():bRedraw_(true){}

		bool IsRedraw(){ return bRedraw_; }


	virtual void SetWindow(Windows::UI::Core::CoreWindow const& window)=0;
	virtual bool IsClose()=0;
	virtual void OnDraw()=0;
	virtual void SetIndependentResources(winrt::com_ptr<ID2D1Factory> fac, winrt::com_ptr<IDWriteFactory3> wfac)=0;
	virtual void CreateDeviceResources(winrt::com_ptr<ID2D1DeviceContext> cxt)=0;
	virtual void ReleaseDeviceResources()=0;
	virtual void OnEntry()=0;
	virtual void Close()=0;

	static D2DWinrt& CreeateInstance();
	protected :
		bool bRedraw_;
};