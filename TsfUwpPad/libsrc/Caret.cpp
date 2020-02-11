#include "pch.h"
#include "libsrc/DTimer.h"
#include "D2DContext.h"
#include "caret.h"

namespace V5
{
static void Blink(DTimer* timer)
{
	int a = Caret::GetCaret().bShow_;
	
	if ( a == 1 )
		a = -1;
	else if ( a == -1 )
		a = 1;
	else if ( a == 0 )
		timer->Cancel();

	Caret::GetCaret().bShow_ = a;
}

Caret::Caret()
{
	bShow_ = 0;
	is_start_change_ = 0;
	blink_pos_ = 0;
	
	timer_ = nullptr;
}

Caret& Caret::GetCaret()
{
	static Caret ca;
	return ca;
}

void Caret::Activate()
{
	bShow_ = 1;
	is_start_change_ = 0;

	if ( timer_ == nullptr )
	{
		timer_ = new DTimer();
		std::function<void(DTimer*)> func = Blink;

		timer_->CreatePeriodicTimer( func );
	}
}

void Caret::UnActivate()
{
	bShow_ = 0;

	if ( timer_ )
		timer_->Cancel();

	is_start_change_ = 0;
	delete timer_;
	timer_ = nullptr;
}



void Caret::ShowCaret()
{
	bShow_ = 1;
}
void Caret::HideCaret()
{
	bShow_ = 0;
}
		
bool Caret::Draw(D2DContext& cxt )
{
	if ( bShow_ == 0  ) return false;
	else if ( bShow_ == -1 ) return true;
		
	auto rc = GetCaretRect();

	D2DMatrix mat(cxt);

	mat.PushTransform();
			
	//mat.NewTransform(ti_->mat);
	
	cxt.cxt->FillRectangle( rc, cxt.black );
	
	
	mat.PopTransform();
	return true;
}

FRectF Caret::GetCaretRect()
{
	int pos=-1;


	FRectF rc; 
	

	if ( rcs.size()==0)
	{
		rc.SetRect(0, 0, 2, 30); //ti_->line_height);
		return rc;
	}
	
	// caretの位置で算出する場合 pos:-1
	if ( pos < 0 )
	{
		//	pos = ( is_start_change_ == 1 ? ti_->sel_start_pos : ti_->sel_end_pos );
		pos = blink_pos_;
	}

	
	if (pos > 0 && (pos - 1) < rcs.size())
	{
		rc = rcs[pos - 1];
		

		if (rc.left != rc.right)
		{
			rc.Offset(rc.Width(), 0);
			rc.right = rc.left + 2;
		}
		else
		{
			// 行頭
			rc.Offset(0, rc.Height());
			rc.left = 0;
			rc.right = rc.left + 2;
		}
	}
	else
	{
		// 先頭
		rc = rcs[0];
		rc.right = rc.left + 2;
	}
	
	return rc;
}
};