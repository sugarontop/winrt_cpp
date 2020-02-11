﻿#pragma once
#include "D2DMisc.h"
#include "comptr.h"

//#define  ComPTR winrt::com_ptr



#ifdef ENGLISH
#define DEFAULTFONT_NAME	L"Segoe UI"
#define DEFAULTFONT_HEIGHT	16
#define DEFAULTLOCALE	L"en-US"
#define DEFAULT_WEIGHT	DWRITE_FONT_WEIGHT_NORMAL
#else
#define DEFAULTFONT_NAME	L"Yu Gothic UI"
#define DEFAULTFONT_HEIGHT	16		// 16dpi->0.17inch->4.32mm
#define DEFAULTLOCALE	L"ja-JP"
#define DEFAULT_WEIGHT	DWRITE_FONT_WEIGHT_NORMAL
#endif



namespace V5
{
	struct D2DContextBase
	{
		ComPTR<ID2D1RenderTarget>  cxt;
		ComPTR<IDWriteTextFormat> textformat;
		ComPTR<IDWriteFactory> wfactory;
	};
	struct SingletonD2DInstance
	{
		ComPTR<IDWriteFactory> wrfactory;
		ComPTR<ID2D1Factory>  factory;
		ComPTR<IDWriteTextFormat> textformat; // IDWriteTextFormat1 is from Win8.1.

		static SingletonD2DInstance& Init();

	};

#define STOCKSIZE 16
	struct D2DContext;



	struct D2DContextText
	{
		bool Init(D2DContext& inshw, float height, LPCWSTR fontname);

		UINT GetLineMetrics(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics);
		UINT GetLineMetric(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, DWRITE_LINE_METRICS& lineMetric);
		UINT GetLineMetric(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics);
		UINT GetLineMetric(const D2D1_SIZE_F& sz, IDWriteTextFormat* fmt, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics);

		ComPTR<IDWriteTextFormat> textformat;
		ComPTR<IDWriteFactory> wfactory;

		float xoff;			// １行表示の左端の余幅
		float line_height;	// １行表示の高さ

	};
	struct D2DContext : public D2DContextBase
	{
		/*
		struct D2DContextBase
		{
		ComPTR<ID2D1RenderTarget>  cxt;
		IDWriteTextFormat* text;
		IDWriteFactory* wfactory;
		};
		*/
		D2DContext();
		D2DContext(const D2DContext& cxt);

		DWORD tickcount_;
		SingletonD2DInstance* insins;
		LPVOID free_space;

		operator ID2D1RenderTarget*() const { return cxt.get(); }
#ifdef USE_ID2D1DEVICECONTEXT
		ComPTR<IDXGISwapChain1> dxgiSwapChain;
		D2D1_SIZE_U RenderSize_;
#endif

		ComPTR<ID2D1SolidColorBrush> ltgray;
		ComPTR<ID2D1SolidColorBrush> ltgray2;
		ComPTR<ID2D1SolidColorBrush> black;
		ComPTR<ID2D1SolidColorBrush> white;
		ComPTR<ID2D1SolidColorBrush> red;
		ComPTR<ID2D1SolidColorBrush> gray;
		ComPTR<ID2D1SolidColorBrush> bluegray;
		ComPTR<ID2D1SolidColorBrush> transparent;
		ComPTR<ID2D1SolidColorBrush> halftone;
		ComPTR<ID2D1SolidColorBrush> halftoneRed;
		ComPTR<ID2D1SolidColorBrush> tooltip;

		ComPTR<ID2D1SolidColorBrush> frame[3];

		ComPTR<ID2D1SolidColorBrush> yellow;
		ComPTR<ID2D1SolidColorBrush> blue;
		ComPTR<ID2D1SolidColorBrush> green;
		ComPTR<ID2D1LinearGradientBrush> silver_grd;

		ComPTR<ID2D1StrokeStyle> dot4_;
		ComPTR<ID2D1StrokeStyle> dot2_;
		ComPTR<ID2D1DrawingStateBlock>  m_stateBlock;

		ComPTR<ID2D1Factory> factory() { return insins->factory; }
		D2DContextText cxtt;

		

		//void Init(SingletonD2DInstance& ins, HWND hWnd);
		void Destroy();

		//void Init(SingletonD2DInstance& ins, const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void Init(SingletonD2DInstance& ins, ComPTR<ID2D1Factory> fac, ComPTR<IDWriteFactory> wfac, ComPTR<ID2D1DeviceContext> cxt );

		void CreateResourceOpt();

		void CreateDeviceContextRenderTargetTest(HWND hWnd, UINT width);


		void DestroyRenderTargetResource();

		void DestroyAll();

		void SetAntiAlias(bool bl) { cxt->SetAntialiasMode(bl ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED); }


		HRESULT CreateFont(LPCWSTR fontnm, float height, IDWriteTextFormat** ret);


		void CreateRenderResource(HWND hWnd);


		void CreateDeviceResources(ID2D1RenderTarget* t);


		ComPTR<IDWriteTextFormat> textformats[3];

	protected:
		void CreateHwndRenderTarget(HWND hWnd);
		void CreateDeviceContextRenderTarget(HWND hWnd);
		
		
	};

	struct D2DRectFilter
	{
		D2DRectFilter(D2DContext& cxt1, const FRectF& rc) :cxt(cxt1)
		{
			cxt.cxt->PushAxisAlignedClip(rc, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			cnt = 1;
		}

		D2DRectFilter(D2DContext& cxt1, FRectF&& rc) :cxt(cxt1)
		{
			cxt.cxt->PushAxisAlignedClip(std::move(rc), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			cnt = 1;
		}

		~D2DRectFilter()
		{
			if (cnt == 1)
				cxt.cxt->PopAxisAlignedClip();
		}
		void Off()
		{
			if (cnt == 1)
			{
				cxt.cxt->PopAxisAlignedClip();
				cnt = 0;
			}
		}

	private:
		D2DContext& cxt;
		int cnt;
	};

	

	inline void ThrowIfFailed(HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm)
	{
		if (FAILED(hr))
		{
			//throw D2DError(hr, msg, line, fnm);
			//General access denied error 0x80070005 
		}
	};
#define THROWIFFAILED(hr,msg) ThrowIfFailed(hr,msg, __LINE__, __FILE__)





	

	class D2DColor
	{
		public :
			D2DColor():clr_(D2RGB(0,0,0)){}
			D2DColor( int r, int g, int b, int a ):clr_(D2RGBA(r,g,b,a)){}
			D2DColor( DWORD dw ):clr_( ColorF(((dw&0xFF0000)>>16)/255.0f, ((dw&0x00FF00)>>8)/255.0f, (dw&0xFF)/255.0f, 1.0f )){}
		
			operator ColorF() const { return clr_; }
				
		private :
			ColorF clr_;
	};





};



