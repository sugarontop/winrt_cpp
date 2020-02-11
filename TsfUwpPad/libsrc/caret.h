#pragma once

class DTimer;
#include "D2DMisc.h"

namespace V5 {


struct TextInfo;
class D2DContext;

class Caret
{
	private :
		Caret();

	public :
		static Caret& GetCaret();

		virtual void Activate(  );
		virtual void UnActivate();

		virtual void ShowCaret();
		virtual void HideCaret();
		
		virtual bool Draw(D2DContext& cxt );
		virtual FRectF GetCaretRect();

		int bShow_;

		int is_start_change_;
		int temp_pos_;

		int blink_pos_;

		std::vector<RECT> rcs;
	
	private :
		DTimer* timer_;

};
};